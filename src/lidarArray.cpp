#include "LidarArray.h"

#include <new>

LidarArrayConfig LidarArrayConfig::defaults(LidarSensorModel model)
{
    LidarArrayConfig config;
    config.model = model;

    if (model == LidarSensorModel::VL53L4CD)
    {
        config.timeoutMs = 100;
        config.vl53l4cdTimingBudgetMs = 50;
        config.vl53l4cdInterMeasurementMs = 0;
    }

    return config;
}

LidarArrayConfig LidarArrayConfig::forModel(
    LidarSensorModel model,
    uint8_t numPCF,
    uint8_t numSensors,
    const uint8_t *pcf8574Addresses,
    const uint8_t *xshutPins,
    TwoWire *wire,
    const uint8_t *sensorAddresses)
{
    LidarArrayConfig config = defaults(model);
    config.numPCF = numPCF;
    config.numSensors = numSensors;
    config.pcf8574Addresses = pcf8574Addresses;
    config.xshutPins = xshutPins;
    config.sensorAddresses = sensorAddresses;
    config.wire = wire != nullptr ? wire : &Wire;
    return config;
}

LidarArrayConfig LidarArrayConfig::forVL53L0X(
    uint8_t numPCF,
    uint8_t numSensors,
    const uint8_t *pcf8574Addresses,
    const uint8_t *xshutPins,
    TwoWire *wire,
    const uint8_t *sensorAddresses)
{
    return forModel(LidarSensorModel::VL53L0X, numPCF, numSensors, pcf8574Addresses, xshutPins, wire, sensorAddresses);
}

LidarArrayConfig LidarArrayConfig::forVL53L4CD(
    uint8_t numPCF,
    uint8_t numSensors,
    const uint8_t *pcf8574Addresses,
    const uint8_t *xshutPins,
    TwoWire *wire,
    const uint8_t *sensorAddresses)
{
    return forModel(LidarSensorModel::VL53L4CD, numPCF, numSensors, pcf8574Addresses, xshutPins, wire, sensorAddresses);
}

LidarArrayDebugConfig LidarArrayDebugConfig::verbose(
    Print *out,
    bool scanBeforeInit,
    bool scanEachStep,
    bool animated,
    uint16_t animationDelayMs,
    uint16_t bootDelayMs)
{
    LidarArrayDebugConfig config;
    config.out = out;
    config.level = LidarDebugLevel::Verbose;
    config.scanBeforeInit = scanBeforeInit;
    config.scanEachStep = scanEachStep;
    config.animated = animated;
    config.animationDelayMs = animationDelayMs;
    config.bootDelayMs = bootDelayMs;
    return config;
}

LidarArray::LidarArray(LidarSensorModel model)
{
    resetMembers();
    config_ = LidarArrayConfig::defaults(model);
}

LidarArray::LidarArray(const LidarArrayConfig &config)
{
    resetMembers();
    config_ = LidarArrayConfig::defaults(config.model);
    config_ = config;
}

LidarArray::LidarArray(
    uint8_t numPCF,
    uint8_t numSensors,
    const uint8_t pcf8574Addresses[],
    const uint8_t xshutPins[][8])
{
    resetMembers();
    config_ = LidarArrayConfig::forVL53L0X(
        numPCF,
        numSensors,
        pcf8574Addresses,
        reinterpret_cast<const uint8_t *>(xshutPins),
        &Wire);
    config_.legacyReadBehavior = true;
}

LidarArray::~LidarArray()
{
    releaseStorage();
}

void LidarArray::resetMembers()
{
    numPCF_ = 0;
    sensorCount_ = 0;
    totalSlots_ = 0;
    initializedSensorCount_ = 0;
    configCopied_ = false;
    configValid_ = false;

    model_ = LidarSensorModel::VL53L0X;
    wire_ = &Wire;

    timeoutMs_ = 0;
    shutdownDelayMs_ = 5;
    wakeDelayMs_ = 5;

    legacyReadBehavior_ = false;
    legacyMinDistanceMm_ = 10;
    legacyMaxDistanceMm_ = 700;
    legacyFallbackDistanceMm_ = 700;

    vl53l0xMeasurementTimingBudgetUs_ = 20000;
    applyVl53l0xVcselPeriods_ = false;
    vl53l0xPreRangeVcselPeriod_ = 14;
    vl53l0xFinalRangeVcselPeriod_ = 10;

    vl53l4cdTimingBudgetMs_ = 50;
    vl53l4cdInterMeasurementMs_ = 0;

    pcf8574Addresses_ = nullptr;
    xshutPins_ = nullptr;
    pcf8574States_ = nullptr;
    sensorAddresses_ = nullptr;
    sensorReady_ = nullptr;
    vl53l0xSensors_ = nullptr;
    vl53l4cdSensors_ = nullptr;

    config_ = LidarArrayConfig::defaults();
}

bool LidarArray::allocateStorage(uint8_t numPCF, uint8_t numSensors, LidarSensorModel model)
{
    if (numPCF == 0 || numSensors == 0)
    {
        return false;
    }

    numPCF_ = numPCF;
    sensorCount_ = numSensors;
    totalSlots_ = static_cast<uint8_t>(numPCF * kSlotsPerPcf);
    model_ = model;

    pcf8574Addresses_ = new (std::nothrow) uint8_t[numPCF_];
    xshutPins_ = new (std::nothrow) uint8_t[totalSlots_];
    pcf8574States_ = new (std::nothrow) uint8_t[numPCF_];
    sensorAddresses_ = new (std::nothrow) uint8_t[sensorCount_];
    sensorReady_ = new (std::nothrow) bool[sensorCount_];

    if (pcf8574Addresses_ == nullptr ||
        xshutPins_ == nullptr ||
        pcf8574States_ == nullptr ||
        sensorAddresses_ == nullptr ||
        sensorReady_ == nullptr)
    {
        releaseStorage();
        return false;
    }

    if (model_ == LidarSensorModel::VL53L0X)
    {
        vl53l0xSensors_ = new (std::nothrow) VL53L0X[sensorCount_];
        if (vl53l0xSensors_ == nullptr)
        {
            releaseStorage();
            return false;
        }
    }
    else
    {
        vl53l4cdSensors_ = new (std::nothrow) VL53L4CD[sensorCount_];
        if (vl53l4cdSensors_ == nullptr)
        {
            releaseStorage();
            return false;
        }
    }

    return true;
}

void LidarArray::releaseStorage()
{
    delete[] pcf8574Addresses_;
    delete[] xshutPins_;
    delete[] pcf8574States_;
    delete[] sensorAddresses_;
    delete[] sensorReady_;
    delete[] vl53l0xSensors_;
    delete[] vl53l4cdSensors_;

    pcf8574Addresses_ = nullptr;
    xshutPins_ = nullptr;
    pcf8574States_ = nullptr;
    sensorAddresses_ = nullptr;
    sensorReady_ = nullptr;
    vl53l0xSensors_ = nullptr;
    vl53l4cdSensors_ = nullptr;
}

void LidarArray::copyConfig(const LidarArrayConfig &config)
{
    if (config.pcf8574Addresses == nullptr || config.xshutPins == nullptr)
    {
        configCopied_ = false;
        return;
    }

    model_ = config.model;
    wire_ = config.wire != nullptr ? config.wire : &Wire;
    timeoutMs_ = config.timeoutMs;
    shutdownDelayMs_ = config.shutdownDelayMs;
    wakeDelayMs_ = config.wakeDelayMs;
    legacyReadBehavior_ = config.legacyReadBehavior;
    legacyMinDistanceMm_ = config.legacyMinDistanceMm;
    legacyMaxDistanceMm_ = config.legacyMaxDistanceMm;
    legacyFallbackDistanceMm_ = config.legacyFallbackDistanceMm;
    vl53l0xMeasurementTimingBudgetUs_ = config.vl53l0xMeasurementTimingBudgetUs;
    applyVl53l0xVcselPeriods_ = config.applyVl53l0xVcselPeriods;
    vl53l0xPreRangeVcselPeriod_ = config.vl53l0xPreRangeVcselPeriod;
    vl53l0xFinalRangeVcselPeriod_ = config.vl53l0xFinalRangeVcselPeriod;
    vl53l4cdTimingBudgetMs_ = config.vl53l4cdTimingBudgetMs;
    vl53l4cdInterMeasurementMs_ = config.vl53l4cdInterMeasurementMs;

    for (uint8_t i = 0; i < numPCF_; ++i)
    {
        pcf8574Addresses_[i] = config.pcf8574Addresses[i];
        pcf8574States_[i] = 0xFF;
    }

    for (uint8_t i = 0; i < totalSlots_; ++i)
    {
        xshutPins_[i] = config.xshutPins[i];
    }

    for (uint8_t i = 0; i < sensorCount_; ++i)
    {
        if (config.sensorAddresses != nullptr)
        {
            sensorAddresses_[i] = config.sensorAddresses[i];
        }
        else
        {
            sensorAddresses_[i] = kDefaultAddressBase + i;
        }
        sensorReady_[i] = false;
    }

    configCopied_ = true;
}

bool LidarArray::syncConfiguration()
{
    const bool needsReallocation =
        pcf8574Addresses_ == nullptr ||
        xshutPins_ == nullptr ||
        pcf8574States_ == nullptr ||
        sensorAddresses_ == nullptr ||
        sensorReady_ == nullptr ||
        numPCF_ != config_.numPCF ||
        sensorCount_ != config_.numSensors ||
        model_ != config_.model;

    if (needsReallocation)
    {
        releaseStorage();
        if (!allocateStorage(config_.numPCF, config_.numSensors, config_.model))
        {
            configValid_ = false;
            return false;
        }
    }

    copyConfig(config_);
    configValid_ = validateConfiguration();
    return configValid_;
}

bool LidarArray::validateConfiguration() const
{
    if (!configCopied_ ||
        numPCF_ == 0 ||
        sensorCount_ == 0 ||
        wire_ == nullptr ||
        pcf8574Addresses_ == nullptr ||
        xshutPins_ == nullptr ||
        pcf8574States_ == nullptr ||
        sensorAddresses_ == nullptr ||
        sensorReady_ == nullptr)
    {
        return false;
    }

    if (sensorCount_ > totalSlots_)
    {
        return false;
    }

    if (config_.sensorAddresses == nullptr)
    {
        const uint8_t maxSensorAddresses = (0x77 - kDefaultAddressBase) + 1;
        if (sensorCount_ > maxSensorAddresses)
        {
            return false;
        }
    }

    for (uint8_t i = 0; i < numPCF_; ++i)
    {
        if (pcf8574Addresses_[i] == 0 || pcf8574Addresses_[i] > 0x77)
        {
            return false;
        }

        for (uint8_t j = i + 1; j < numPCF_; ++j)
        {
            if (pcf8574Addresses_[i] == pcf8574Addresses_[j])
            {
                return false;
            }
        }

        uint8_t pinMask = 0;
        for (uint8_t slot = 0; slot < kSlotsPerPcf; ++slot)
        {
            const uint8_t index = (i * kSlotsPerPcf) + slot;
            const uint8_t pin = xshutPins_[index];
            if (pin >= kSlotsPerPcf)
            {
                return false;
            }
            if ((pinMask & (1 << pin)) != 0)
            {
                return false;
            }
            pinMask |= (1 << pin);
        }
    }

    for (uint8_t i = 0; i < sensorCount_; ++i)
    {
        const uint8_t targetAddress = sensorAddresses_[i];
        if (targetAddress < kMinimumManualAddress ||
            targetAddress > 0x77 ||
            targetAddress == kTofDefaultAddress)
        {
            return false;
        }

        for (uint8_t pcfIndex = 0; pcfIndex < numPCF_; ++pcfIndex)
        {
            if (targetAddress == pcf8574Addresses_[pcfIndex])
            {
                return false;
            }
        }

        for (uint8_t otherIndex = i + 1; otherIndex < sensorCount_; ++otherIndex)
        {
            if (targetAddress == sensorAddresses_[otherIndex])
            {
                return false;
            }
        }
    }

    return true;
}

bool LidarArray::begin()
{
    if (!syncConfiguration())
    {
        logMessage(LidarDebugLevel::Errors, F("Invalid configuration."));
        return false;
    }

    for (uint8_t i = 0; i < sensorCount_; ++i)
    {
        sensorReady_[i] = false;
    }
    initializedSensorCount_ = 0;

    if (debugConfig_.scanBeforeInit)
    {
        logMessage(LidarDebugLevel::Info, F("I2C scan before sensor shutdown."));
        scanI2C();
    }

    if (debugConfig_.bootDelayMs > 0)
    {
        if (shouldLog(LidarDebugLevel::Info))
        {
            debugConfig_.out->print(F("[LidarArray] Debug boot delay: "));
            debugConfig_.out->print(debugConfig_.bootDelayMs);
            debugConfig_.out->println(F(" ms"));
        }
        delay(debugConfig_.bootDelayMs);
    }

    if (!shutdownAllSensors())
    {
        logMessage(LidarDebugLevel::Errors, F("Failed to place sensors in shutdown."));
        return false;
    }

    if (debugConfig_.scanEachStep)
    {
        logMessage(LidarDebugLevel::Info, F("I2C scan with all TOF sensors in shutdown."));
        scanI2C();
    }

    bool allSensorsReady = true;

    for (uint8_t sensorIndex = 0; sensorIndex < sensorCount_; ++sensorIndex)
    {
        const bool ready = initializeSensor(sensorIndex);
        sensorReady_[sensorIndex] = ready;
        if (ready)
        {
            ++initializedSensorCount_;
        }
        else
        {
            allSensorsReady = false;
        }
    }

    if (allSensorsReady)
    {
        logMessage(LidarDebugLevel::Info, F("All sensors initialized."));
    }
    else
    {
        logMessage(LidarDebugLevel::Errors, F("Initialization completed with unavailable sensors."));
    }

    return allSensorsReady;
}

bool LidarArray::initSensors()
{
    return begin();
}

bool LidarArray::initSensors(int measurementT)
{
    config_.vl53l0xMeasurementTimingBudgetUs = static_cast<uint32_t>(measurementT);
    vl53l0xMeasurementTimingBudgetUs_ = static_cast<uint32_t>(measurementT);
    return begin();
}

bool LidarArray::initSensors(int measurementT, uint8_t preRange, uint8_t finalRange)
{
    config_.vl53l0xMeasurementTimingBudgetUs = static_cast<uint32_t>(measurementT);
    config_.applyVl53l0xVcselPeriods = true;
    config_.vl53l0xPreRangeVcselPeriod = preRange;
    config_.vl53l0xFinalRangeVcselPeriod = finalRange;
    vl53l0xMeasurementTimingBudgetUs_ = static_cast<uint32_t>(measurementT);
    applyVl53l0xVcselPeriods_ = true;
    vl53l0xPreRangeVcselPeriod_ = preRange;
    vl53l0xFinalRangeVcselPeriod_ = finalRange;
    return begin();
}

bool LidarArray::initSensors(int measurementT, uint8_t preRange, uint8_t finalRange, int timeout)
{
    config_.vl53l0xMeasurementTimingBudgetUs = static_cast<uint32_t>(measurementT);
    config_.applyVl53l0xVcselPeriods = true;
    config_.vl53l0xPreRangeVcselPeriod = preRange;
    config_.vl53l0xFinalRangeVcselPeriod = finalRange;
    config_.timeoutMs = timeout > 0 ? static_cast<uint16_t>(timeout) : 0;
    vl53l0xMeasurementTimingBudgetUs_ = static_cast<uint32_t>(measurementT);
    applyVl53l0xVcselPeriods_ = true;
    vl53l0xPreRangeVcselPeriod_ = preRange;
    vl53l0xFinalRangeVcselPeriod_ = finalRange;
    timeoutMs_ = timeout > 0 ? static_cast<uint16_t>(timeout) : 0;
    return begin();
}

LidarArrayConfig &LidarArray::config()
{
    return config_;
}

const LidarArrayConfig &LidarArray::config() const
{
    return config_;
}

LidarArrayDebugConfig &LidarArray::debug()
{
    return debugConfig_;
}

const LidarArrayDebugConfig &LidarArray::debug() const
{
    return debugConfig_;
}

void LidarArray::setLayout(uint8_t numPCF, uint8_t numSensors, const uint8_t *pcf8574Addresses, const uint8_t *xshutPins)
{
    config_.numPCF = numPCF;
    config_.numSensors = numSensors;
    config_.pcf8574Addresses = pcf8574Addresses;
    config_.xshutPins = xshutPins;
}

void LidarArray::setSensorAddresses(const uint8_t *sensorAddresses)
{
    config_.sensorAddresses = sensorAddresses;
}

void LidarArray::setWire(TwoWire *wire)
{
    config_.wire = wire != nullptr ? wire : &Wire;
    wire_ = config_.wire;
}

void LidarArray::setTimeout(uint16_t timeoutMs)
{
    config_.timeoutMs = timeoutMs;
    timeoutMs_ = timeoutMs;
}

void LidarArray::setVL53L4CDTiming(uint8_t timingBudgetMs, uint32_t interMeasurementMs)
{
    config_.vl53l4cdTimingBudgetMs = timingBudgetMs;
    config_.vl53l4cdInterMeasurementMs = interMeasurementMs;
    vl53l4cdTimingBudgetMs_ = timingBudgetMs;
    vl53l4cdInterMeasurementMs_ = interMeasurementMs;
}

uint16_t LidarArray::readSensor(uint8_t sensorIndex)
{
    const LidarReading reading = readReading(sensorIndex, true);
    if (legacyReadBehavior_)
    {
        return applyLegacyReadBehavior(reading);
    }
    return reading.distanceMm;
}

uint16_t LidarArray::readSensorNB(uint8_t sensorIndex)
{
    const LidarReading reading = readReading(sensorIndex, false);
    if (legacyReadBehavior_)
    {
        return applyLegacyReadBehavior(reading);
    }
    return reading.distanceMm;
}

LidarReading LidarArray::readReading(uint8_t sensorIndex, bool blocking)
{
    if (sensorIndex >= sensorCount_)
    {
        return buildInvalidReading(sensorIndex, kLibraryStatusInvalidIndex);
    }

    if (!sensorReady_[sensorIndex])
    {
        return buildInvalidReading(sensorIndex, kLibraryStatusNotReady);
    }

    if (model_ == LidarSensorModel::VL53L0X)
    {
        return readVL53L0X(sensorIndex, blocking);
    }

    return readVL53L4CD(sensorIndex, blocking);
}

VL53L0X &LidarArray::getSensor(uint8_t sensorIndex)
{
    VL53L0X *sensor = getVL53L0XSensor(sensorIndex);
    if (sensor != nullptr)
    {
        return *sensor;
    }

    static VL53L0X nullSensor;
    return nullSensor;
}

VL53L0X *LidarArray::getVL53L0XSensor(uint8_t sensorIndex)
{
    if (model_ != LidarSensorModel::VL53L0X || sensorIndex >= sensorCount_)
    {
        return nullptr;
    }
    return &vl53l0xSensors_[sensorIndex];
}

VL53L4CD *LidarArray::getVL53L4CDSensor(uint8_t sensorIndex)
{
    if (model_ != LidarSensorModel::VL53L4CD || sensorIndex >= sensorCount_)
    {
        return nullptr;
    }
    return &vl53l4cdSensors_[sensorIndex];
}

void LidarArray::setMeasurementTimingBudget(uint32_t timingBudget)
{
    config_.vl53l0xMeasurementTimingBudgetUs = timingBudget;
    vl53l0xMeasurementTimingBudgetUs_ = timingBudget;

    if (model_ != LidarSensorModel::VL53L0X)
    {
        logMessage(LidarDebugLevel::Errors, F("setMeasurementTimingBudget is only applied to VL53L0X instances."));
        return;
    }

    for (uint8_t i = 0; i < sensorCount_; ++i)
    {
        if (sensorReady_[i] && !vl53l0xSensors_[i].setMeasurementTimingBudget(vl53l0xMeasurementTimingBudgetUs_))
        {
            logSensorStep(i, sensorAddresses_[i], F("failed to update timing budget"), LidarDebugLevel::Errors);
        }
    }
}

void LidarArray::setVcselPulsePeriod(uint8_t type, uint8_t period)
{
    if (type == 0)
    {
        config_.vl53l0xPreRangeVcselPeriod = period;
        vl53l0xPreRangeVcselPeriod_ = period;
    }
    else
    {
        config_.vl53l0xFinalRangeVcselPeriod = period;
        vl53l0xFinalRangeVcselPeriod_ = period;
    }
    config_.applyVl53l0xVcselPeriods = true;
    applyVl53l0xVcselPeriods_ = true;

    if (model_ != LidarSensorModel::VL53L0X)
    {
        logMessage(LidarDebugLevel::Errors, F("setVcselPulsePeriod is only applied to VL53L0X instances."));
        return;
    }

    for (uint8_t i = 0; i < sensorCount_; ++i)
    {
        if (!sensorReady_[i])
        {
            continue;
        }

        bool applied = false;
        if (type == 0)
        {
            applied = vl53l0xSensors_[i].setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, period);
        }
        else if (type == 1)
        {
            applied = vl53l0xSensors_[i].setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, period);
        }

        if (!applied)
        {
            logSensorStep(i, sensorAddresses_[i], F("failed to update VCSEL period"), LidarDebugLevel::Errors);
        }
    }
}

void LidarArray::setDebugConfig(const LidarArrayDebugConfig &config)
{
    debugConfig_ = config;
}

void LidarArray::setDebugOutput(Print *out)
{
    debugConfig_.out = out;
}

void LidarArray::setDebugLevel(LidarDebugLevel level)
{
    debugConfig_.level = level;
}

void LidarArray::setDebugScanBeforeInit(bool enabled)
{
    debugConfig_.scanBeforeInit = enabled;
}

void LidarArray::setDebugScanEachStep(bool enabled)
{
    debugConfig_.scanEachStep = enabled;
}

void LidarArray::setDebugBootDelay(uint16_t delayMs)
{
    debugConfig_.bootDelayMs = delayMs;
}

void LidarArray::setDebugStepDelay(uint16_t delayMs)
{
    debugConfig_.animationDelayMs = delayMs;
    debugConfig_.animated = delayMs > 0;
}

uint8_t LidarArray::getSensorCount() const
{
    return sensorCount_ != 0 ? sensorCount_ : config_.numSensors;
}

uint8_t LidarArray::getInitializedSensorCount() const
{
    return initializedSensorCount_;
}

bool LidarArray::isSensorReady(uint8_t sensorIndex) const
{
    return sensorIndex < sensorCount_ && sensorReady_[sensorIndex];
}

uint8_t LidarArray::scanI2C()
{
    uint8_t foundCount = 0;

    if (shouldLog(LidarDebugLevel::Info))
    {
        debugConfig_.out->println(F("[LidarArray] I2C scan started."));
    }

    for (uint8_t address = 1; address < 127; ++address)
    {
        wire_->beginTransmission(address);
        const uint8_t errorCode = wire_->endTransmission();
        if (errorCode == 0)
        {
            ++foundCount;
            logScanResult(address, errorCode);
        }
        else if (errorCode == 4 && shouldLog(LidarDebugLevel::Verbose))
        {
            logScanResult(address, errorCode);
        }
    }

    if (shouldLog(LidarDebugLevel::Info))
    {
        debugConfig_.out->print(F("[LidarArray] I2C scan complete. Devices found: "));
        debugConfig_.out->println(foundCount);
    }

    return foundCount;
}

bool LidarArray::shutdownAllSensors()
{
    for (uint8_t pcfIndex = 0; pcfIndex < numPCF_; ++pcfIndex)
    {
        pcf8574States_[pcfIndex] = 0xFF;
    }

    for (uint8_t slotIndex = 0; slotIndex < totalSlots_; ++slotIndex)
    {
        const uint8_t pcfIndex = slotIndex / kSlotsPerPcf;
        const uint8_t pin = xshutPins_[slotIndex];
        pcf8574States_[pcfIndex] &= static_cast<uint8_t>(~(1 << pin));
    }

    bool success = true;
    for (uint8_t pcfIndex = 0; pcfIndex < numPCF_; ++pcfIndex)
    {
        if (!writePcf8574State(pcfIndex))
        {
            success = false;
        }
    }

    if (shutdownDelayMs_ > 0)
    {
        delay(shutdownDelayMs_);
    }

    return success;
}

bool LidarArray::bringSensorOutOfShutdown(uint8_t sensorIndex)
{
    const uint8_t pcfIndex = sensorIndex / kSlotsPerPcf;
    const uint8_t pin = xshutPins_[sensorIndex];
    const bool success = pcf8574Write(pcfIndex, pin, true);

    if (wakeDelayMs_ > 0)
    {
        delay(wakeDelayMs_);
    }

    return success;
}

bool LidarArray::initializeSensor(uint8_t sensorIndex)
{
    const uint8_t address = sensorAddresses_[sensorIndex];

    logSensorStep(sensorIndex, address, F("starting initialization"), LidarDebugLevel::Info);

    if (!bringSensorOutOfShutdown(sensorIndex))
    {
        logSensorStep(sensorIndex, address, F("failed to enable XSHUT"), LidarDebugLevel::Errors);
        return false;
    }

    bool ready = false;
    if (model_ == LidarSensorModel::VL53L0X)
    {
        ready = initializeVL53L0X(sensorIndex, address);
    }
    else
    {
        ready = initializeVL53L4CD(sensorIndex, address);
    }

    if (!ready)
    {
        const uint8_t pcfIndex = sensorIndex / kSlotsPerPcf;
        const uint8_t pin = xshutPins_[sensorIndex];
        pcf8574Write(pcfIndex, pin, false);
        if (shutdownDelayMs_ > 0)
        {
            delay(shutdownDelayMs_);
        }
        logSensorStep(sensorIndex, address, F("sensor unavailable"), LidarDebugLevel::Errors);
    }
    else
    {
        logSensorStep(sensorIndex, address, F("sensor ready"), LidarDebugLevel::Info);
    }

    if (debugConfig_.scanEachStep)
    {
        scanI2C();
    }

    return ready;
}

bool LidarArray::initializeVL53L0X(uint8_t sensorIndex, uint8_t address)
{
    VL53L0X &sensor = vl53l0xSensors_[sensorIndex];
    sensor.setBus(wire_);
    sensor.setTimeout(timeoutMs_);

    if (!sensor.init())
    {
        return false;
    }

    sensor.setAddress(address);
    sensor.setTimeout(timeoutMs_);

    if (!sensor.setMeasurementTimingBudget(vl53l0xMeasurementTimingBudgetUs_))
    {
        return false;
    }

    if (applyVl53l0xVcselPeriods_)
    {
        if (!sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, vl53l0xPreRangeVcselPeriod_))
        {
            return false;
        }
        if (!sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, vl53l0xFinalRangeVcselPeriod_))
        {
            return false;
        }
    }

    sensor.startContinuous();
    return true;
}

bool LidarArray::initializeVL53L4CD(uint8_t sensorIndex, uint8_t address)
{
    VL53L4CD &sensor = vl53l4cdSensors_[sensorIndex];
    sensor.setBus(wire_);
    sensor.setTimeout(timeoutMs_);

    if (!sensor.init())
    {
        return false;
    }

    sensor.setAddress(address);
    sensor.setTimeout(timeoutMs_);

    if (!sensor.setRangeTiming(vl53l4cdTimingBudgetMs_, vl53l4cdInterMeasurementMs_))
    {
        return false;
    }

    sensor.startContinuous();
    return true;
}

bool LidarArray::pcf8574Write(uint8_t pcf8574Index, uint8_t pin, bool state)
{
    if (pcf8574Index >= numPCF_ || pin >= kSlotsPerPcf)
    {
        return false;
    }

    if (state)
    {
        pcf8574States_[pcf8574Index] |= (1 << pin);
    }
    else
    {
        pcf8574States_[pcf8574Index] &= static_cast<uint8_t>(~(1 << pin));
    }

    return writePcf8574State(pcf8574Index);
}

bool LidarArray::writePcf8574State(uint8_t pcf8574Index)
{
    if (pcf8574Index >= numPCF_)
    {
        return false;
    }

    wire_->beginTransmission(pcf8574Addresses_[pcf8574Index]);
    wire_->write(pcf8574States_[pcf8574Index]);
    return wire_->endTransmission() == 0;
}

LidarReading LidarArray::buildInvalidReading(uint8_t sensorIndex, uint8_t status) const
{
    LidarReading reading;
    reading.status = status;
    if (sensorIndex < sensorCount_)
    {
        reading.address = sensorAddresses_[sensorIndex];
    }
    return reading;
}

LidarReading LidarArray::readVL53L0X(uint8_t sensorIndex, bool blocking)
{
    VL53L0X &sensor = vl53l0xSensors_[sensorIndex];
    LidarReading reading;
    reading.address = sensorAddresses_[sensorIndex];

    if (!blocking)
    {
        const uint8_t interruptStatus = sensor.readReg(VL53L0X::RESULT_INTERRUPT_STATUS);
        if ((interruptStatus & 0x07) == 0)
        {
            reading.status = kLibraryStatusNotReady;
            return reading;
        }

        const uint8_t rangeStatus = sensor.readReg(VL53L0X::RESULT_RANGE_STATUS);
        reading.distanceMm = sensor.readReg16Bit(VL53L0X::RESULT_RANGE_STATUS + 10);
        sensor.writeReg(VL53L0X::SYSTEM_INTERRUPT_CLEAR, 0x01);
        reading.status = (rangeStatus & 0x78) >> 3;
        reading.dataReady = true;
        reading.valid = reading.status == 0 && reading.distanceMm > 0;
        return reading;
    }

    reading.distanceMm = sensor.readRangeContinuousMillimeters();
    reading.timeout = sensor.timeoutOccurred();
    if (reading.timeout)
    {
        reading.distanceMm = 0;
        reading.status = kLibraryStatusTimeout;
        return reading;
    }

    reading.status = (sensor.readReg(VL53L0X::RESULT_RANGE_STATUS) & 0x78) >> 3;
    reading.dataReady = true;
    reading.valid = reading.status == 0 && reading.distanceMm > 0;
    return reading;
}

LidarReading LidarArray::readVL53L4CD(uint8_t sensorIndex, bool blocking)
{
    VL53L4CD &sensor = vl53l4cdSensors_[sensorIndex];
    LidarReading reading;
    reading.address = sensorAddresses_[sensorIndex];

    if (!blocking && !sensor.dataReady())
    {
        reading.status = kLibraryStatusNotReady;
        return reading;
    }

    reading.distanceMm = sensor.read(blocking);
    reading.timeout = sensor.timeoutOccurred();
    if (reading.timeout)
    {
        reading.distanceMm = 0;
        reading.status = kLibraryStatusTimeout;
        return reading;
    }

    reading.status = sensor.ranging_data.range_status;
    reading.dataReady = true;
    reading.valid = reading.status == 0;
    return reading;
}

uint16_t LidarArray::applyLegacyReadBehavior(const LidarReading &reading) const
{
    if (!reading.valid || reading.distanceMm == 0)
    {
        return legacyFallbackDistanceMm_;
    }

    return static_cast<uint16_t>(constrain(reading.distanceMm, legacyMinDistanceMm_, legacyMaxDistanceMm_));
}

void LidarArray::logMessage(LidarDebugLevel level, const __FlashStringHelper *message) const
{
    if (!shouldLog(level))
    {
        return;
    }

    debugConfig_.out->print(F("[LidarArray] "));
    debugConfig_.out->println(message);
}

void LidarArray::logScanResult(uint8_t address, uint8_t errorCode) const
{
    if (debugConfig_.out == nullptr)
    {
        return;
    }

    if (errorCode == 0 && !shouldLog(LidarDebugLevel::Info))
    {
        return;
    }

    if (errorCode != 0 && !shouldLog(LidarDebugLevel::Verbose))
    {
        return;
    }

    debugConfig_.out->print(F("[LidarArray] I2C "));
    if (errorCode == 0)
    {
        debugConfig_.out->print(F("device found at 0x"));
    }
    else
    {
        debugConfig_.out->print(F("bus error at 0x"));
    }

    if (address < 0x10)
    {
        debugConfig_.out->print('0');
    }
    debugConfig_.out->println(address, HEX);
}

void LidarArray::logSensorStep(
    uint8_t sensorIndex,
    uint8_t address,
    const __FlashStringHelper *message,
    LidarDebugLevel level) const
{
    if (!shouldLog(level))
    {
        return;
    }

    debugConfig_.out->print(F("[LidarArray] Sensor "));
    debugConfig_.out->print(sensorIndex);
    debugConfig_.out->print(F(" @ 0x"));
    if (address < 0x10)
    {
        debugConfig_.out->print('0');
    }
    debugConfig_.out->print(address, HEX);
    debugConfig_.out->print(F(": "));
    debugConfig_.out->println(message);
    animatePause();
}

bool LidarArray::shouldLog(LidarDebugLevel level) const
{
    if (debugConfig_.out == nullptr)
    {
        return false;
    }

    return static_cast<uint8_t>(debugConfig_.level) >= static_cast<uint8_t>(level);
}

void LidarArray::animatePause() const
{
    if (!debugConfig_.animated || debugConfig_.animationDelayMs == 0)
    {
        return;
    }

    delay(debugConfig_.animationDelayMs);
}
