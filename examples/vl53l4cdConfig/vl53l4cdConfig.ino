#include <LidarArray.h>

// Address of the expander that controls the XSHUT pins.
const uint8_t pcf8574Addresses[] = {0x20};

// The order of this array becomes the logical order of the sensors.
// The first 4 channels are used because this example configures 4 active sensors.
// If your wiring is not a simple dense layout, use the newer API
// with LidarSensorSlot in examples/sparseSensorMap.
const uint8_t xshutPins[] = {
    0, 1, 2, 3, 4, 5, 6, 7
};
const uint8_t sensorAddresses[] = {
    0x30, 0x32, 0x34, 0x36
};
const bool useCustomSensorAddresses = false;

// Main minimal-configuration example for VL53L4CD.
LidarArray lidar(LidarSensorModel::VL53L4CD);

void setup()
{
    Serial.begin(115200);
    Wire.begin();

    // The minimum required setup is:
    // - how many PCF8574 expanders are present
    // - how many sensors are active
    // - the PCF8574 addresses
    // - the logical XSHUT channel map
    lidar.setLayout(1, 4, pcf8574Addresses, xshutPins);

    // Optional tuning.
    lidar.setTimeout(100);
    lidar.setVL53L4CDTiming(50, 0);

    // Optional: manually define the final address of each sensor.
    // Leave false to use automatic mode (0x30 + index).
    if (useCustomSensorAddresses) {
        lidar.setSensorAddresses(sensorAddresses);
    }

    // Optional debug output so you can follow the boot process.
    lidar.setDebugOutput(&Serial);
    lidar.setDebugLevel(LidarDebugLevel::Info);
    lidar.setDebugScanBeforeInit(true);

    if (!lidar.begin()) {
        Serial.println("Partial initialization or at least one sensor failed.");
    }
}

void loop()
{
    for (uint8_t i = 0; i < lidar.getSensorCount(); ++i) {
        // readReading() exposes more information than readSensor().
        LidarReading reading = lidar.readReading(i);

        Serial.print("Sensor ");
        Serial.print(i);
        Serial.print(" -> ready=");
        Serial.print(lidar.isSensorReady(i) ? "yes" : "no");
        Serial.print(", status=");
        Serial.print(reading.status);
        Serial.print(", address=0x");
        if (reading.address < 0x10) {
            Serial.print('0');
        }
        Serial.print(reading.address, HEX);
        Serial.print(", dist=");
        Serial.print(reading.distanceMm);
        Serial.println(" mm");
    }

    Serial.println();
    delay(100);
}
