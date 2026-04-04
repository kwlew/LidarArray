#include <LidarArray.h>

const uint8_t pcf8574Addresses[] = {0x20};
const uint8_t xshutPins[] = {
    0, 1, 2, 3, 4, 5, 6, 7
};

LidarArray lidar(LidarSensorModel::VL53L0X);

void setup()
{
    Serial.begin(115200);
    Wire.begin();

    // Minimum array layout.
    lidar.setLayout(1, 4, pcf8574Addresses, xshutPins);
    lidar.setTimeout(100);
    lidar.setMeasurementTimingBudget(20000);
    lidar.setVcselPulsePeriod(0, 14);
    lidar.setVcselPulsePeriod(1, 10);

    // The goal of this example is to watch the boot sequence:
    // 1. scan the bus before the library changes anything
    // 2. pause so you can open the serial monitor
    // 3. scan again after all ToF sensors are shut down
    // 4. bring sensors back one by one and rescan the bus
    lidar.setDebugOutput(&Serial);
    lidar.setDebugLevel(LidarDebugLevel::Verbose);
    lidar.setDebugScanBeforeInit(true);
    lidar.setDebugScanEachStep(true);
    lidar.setDebugBootDelay(1500);
    lidar.setDebugStepDelay(500);

    bool ok = lidar.begin();
    Serial.print("begin() -> ");
    Serial.println(ok ? "OK" : "PARTIAL/FAILED");
}

void loop()
{
    for (uint8_t i = 0; i < lidar.getSensorCount(); ++i)
    {
        LidarReading reading = lidar.readReading(i, false);

        Serial.print("Sensor ");
        Serial.print(i);
        Serial.print(" ready=");
        Serial.print(lidar.isSensorReady(i) ? "yes" : "no");
        Serial.print(" status=");
        Serial.print(reading.status);
        Serial.print(" dist=");
        Serial.println(reading.distanceMm);
    }

    Serial.println();
    delay(150);
}
