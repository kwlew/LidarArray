#ifndef LIDARARRAY_H
#define LIDARARRAY_H

#include <Arduino.h>
#include <Print.h>
#include <VL53L0X.h>
#include <VL53L4CD.h>
#include <Wire.h>

enum class LidarSensorModel : uint8_t
{
    VL53L0X,
    VL53L4CD
};

enum class LidarDebugLevel : uint8_t
{
    Off,
    Errors,
    Info,
    Verbose
};

struct LidarReading
{
    uint16_t distanceMm = 0;
    uint8_t status = 0xFF;
    bool valid = false;
    bool timeout = false;
    bool dataReady = false;
    uint8_t address = 0;
};

struct LidarArrayConfig
{
    LidarSensorModel model = LidarSensorModel::VL53L0X;
    uint8_t numPCF = 0;
    uint8_t numSensors = 0;
    const uint8_t *pcf8574Addresses = nullptr;
    const uint8_t *xshutPins = nullptr;
    TwoWire *wire = &Wire;
    uint16_t timeoutMs = 0;
    uint16_t shutdownDelayMs = 5;
    uint16_t wakeDelayMs = 5;
    bool legacyReadBehavior = false;
    uint16_t legacyMinDistanceMm = 10;
    uint16_t legacyMaxDistanceMm = 700;
    uint16_t legacyFallbackDistanceMm = 700;
    uint32_t vl53l0xMeasurementTimingBudgetUs = 20000;
    bool applyVl53l0xVcselPeriods = false;
    uint8_t vl53l0xPreRangeVcselPeriod = 14;
    uint8_t vl53l0xFinalRangeVcselPeriod = 10;
    uint8_t vl53l4cdTimingBudgetMs = 50;
    uint32_t vl53l4cdInterMeasurementMs = 0;
    const uint8_t *sensorAddresses = nullptr;

    static LidarArrayConfig defaults(LidarSensorModel model = LidarSensorModel::VL53L0X);
    static LidarArrayConfig forModel(
        LidarSensorModel model,
        uint8_t numPCF,
        uint8_t numSensors,
        const uint8_t *pcf8574Addresses,
        const uint8_t *xshutPins,
        TwoWire *wire = &Wire,
        const uint8_t *sensorAddresses = nullptr);
    static LidarArrayConfig forVL53L0X(
        uint8_t numPCF,
        uint8_t numSensors,
        const uint8_t *pcf8574Addresses,
        const uint8_t *xshutPins,
        TwoWire *wire = &Wire,
        const uint8_t *sensorAddresses = nullptr);
    static LidarArrayConfig forVL53L4CD(
        uint8_t numPCF,
        uint8_t numSensors,
        const uint8_t *pcf8574Addresses,
        const uint8_t *xshutPins,
        TwoWire *wire = &Wire,
        const uint8_t *sensorAddresses = nullptr);
};

struct LidarArrayDebugConfig
{
    Print *out = nullptr;
    LidarDebugLevel level = LidarDebugLevel::Off;
    bool scanBeforeInit = false;
    bool scanEachStep = false;
    bool animated = false;
    uint16_t animationDelayMs = 40;
    uint16_t bootDelayMs = 0;

    static LidarArrayDebugConfig verbose(
        Print *out,
        bool scanBeforeInit = false,
        bool scanEachStep = false,
        bool animated = false,
        uint16_t animationDelayMs = 40,
        uint16_t bootDelayMs = 0);
};

class LidarArray
{
public:
    explicit LidarArray(LidarSensorModel model = LidarSensorModel::VL53L0X);
    explicit LidarArray(const LidarArrayConfig &config);
    LidarArray(uint8_t numPCF, uint8_t numSensors, const uint8_t pcf8574Addresses[], const uint8_t xshutPins[][8]);
    ~LidarArray();

    LidarArray(const LidarArray &) = delete;
    LidarArray &operator=(const LidarArray &) = delete;

    bool begin();

    bool initSensors();
    bool initSensors(int measurementT);
    bool initSensors(int measurementT, uint8_t preRange, uint8_t finalRange);
    bool initSensors(int measurementT, uint8_t preRange, uint8_t finalRange, int timeout);

    LidarArrayConfig &config();
    const LidarArrayConfig &config() const;
    LidarArrayDebugConfig &debug();
    const LidarArrayDebugConfig &debug() const;

    void setLayout(uint8_t numPCF, uint8_t numSensors, const uint8_t *pcf8574Addresses, const uint8_t *xshutPins);
    void setSensorAddresses(const uint8_t *sensorAddresses);
    void setWire(TwoWire *wire);
    void setTimeout(uint16_t timeoutMs);
    void setVL53L4CDTiming(uint8_t timingBudgetMs, uint32_t interMeasurementMs = 0);

    uint16_t readSensor(uint8_t sensorIndex);
    uint16_t readSensorNB(uint8_t sensorIndex);
    LidarReading readReading(uint8_t sensorIndex, bool blocking = true);

    VL53L0X &getSensor(uint8_t sensorIndex);
    VL53L0X *getVL53L0XSensor(uint8_t sensorIndex);
    VL53L4CD *getVL53L4CDSensor(uint8_t sensorIndex);

    void setMeasurementTimingBudget(uint32_t timingBudget);
    void setVcselPulsePeriod(uint8_t type, uint8_t period);
    void setDebugConfig(const LidarArrayDebugConfig &config);
    void setDebugOutput(Print *out);
    void setDebugLevel(LidarDebugLevel level);
    void setDebugScanBeforeInit(bool enabled);
    void setDebugScanEachStep(bool enabled);
    void setDebugBootDelay(uint16_t delayMs);
    void setDebugStepDelay(uint16_t delayMs);

    uint8_t getSensorCount() const;
    uint8_t getInitializedSensorCount() const;
    bool isSensorReady(uint8_t sensorIndex) const;
    uint8_t scanI2C();

private:
    static constexpr uint8_t kSlotsPerPcf = 8;
    static constexpr uint8_t kTofDefaultAddress = 0x29;
    static constexpr uint8_t kDefaultAddressBase = 0x30;
    static constexpr uint8_t kMinimumManualAddress = 0x08;
    static constexpr uint8_t kLibraryStatusInvalidIndex = 0xFD;
    static constexpr uint8_t kLibraryStatusNotReady = 0xFE;
    static constexpr uint8_t kLibraryStatusTimeout = 0xFF;

    uint8_t numPCF_;
    uint8_t sensorCount_;
    uint8_t totalSlots_;
    uint8_t initializedSensorCount_;
    bool configCopied_;
    bool configValid_;

    LidarSensorModel model_;
    TwoWire *wire_;

    uint16_t timeoutMs_;
    uint16_t shutdownDelayMs_;
    uint16_t wakeDelayMs_;

    bool legacyReadBehavior_;
    uint16_t legacyMinDistanceMm_;
    uint16_t legacyMaxDistanceMm_;
    uint16_t legacyFallbackDistanceMm_;

    uint32_t vl53l0xMeasurementTimingBudgetUs_;
    bool applyVl53l0xVcselPeriods_;
    uint8_t vl53l0xPreRangeVcselPeriod_;
    uint8_t vl53l0xFinalRangeVcselPeriod_;

    uint8_t vl53l4cdTimingBudgetMs_;
    uint32_t vl53l4cdInterMeasurementMs_;

    LidarArrayDebugConfig debugConfig_;

    uint8_t *pcf8574Addresses_;
    uint8_t *xshutPins_;
    uint8_t *pcf8574States_;
    uint8_t *sensorAddresses_;
    bool *sensorReady_;

    VL53L0X *vl53l0xSensors_;
    VL53L4CD *vl53l4cdSensors_;

    LidarArrayConfig config_;

    void resetMembers();
    bool allocateStorage(uint8_t numPCF, uint8_t numSensors, LidarSensorModel model);
    void releaseStorage();
    void copyConfig(const LidarArrayConfig &config);
    bool syncConfiguration();
    bool validateConfiguration() const;

    bool shutdownAllSensors();
    bool bringSensorOutOfShutdown(uint8_t sensorIndex);
    bool initializeSensor(uint8_t sensorIndex);

    bool initializeVL53L0X(uint8_t sensorIndex, uint8_t address);
    bool initializeVL53L4CD(uint8_t sensorIndex, uint8_t address);

    bool pcf8574Write(uint8_t pcf8574Index, uint8_t pin, bool state);
    bool writePcf8574State(uint8_t pcf8574Index);

    LidarReading buildInvalidReading(uint8_t sensorIndex, uint8_t status) const;
    LidarReading readVL53L0X(uint8_t sensorIndex, bool blocking);
    LidarReading readVL53L4CD(uint8_t sensorIndex, bool blocking);
    uint16_t applyLegacyReadBehavior(const LidarReading &reading) const;

    void logMessage(LidarDebugLevel level, const __FlashStringHelper *message) const;
    void logScanResult(uint8_t address, uint8_t errorCode) const;
    void logSensorStep(uint8_t sensorIndex, uint8_t address, const __FlashStringHelper *message, LidarDebugLevel level) const;
    bool shouldLog(LidarDebugLevel level) const;
    void animatePause() const;
};

#endif
