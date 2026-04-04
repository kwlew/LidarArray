#include <LidarArray.h>

const uint8_t pcf8574Addresses[] = {0x20};
const uint8_t xshutPins[] = {
    0, 1, 2, 3, 4, 5, 6, 7
};
const uint8_t customSensorAddresses[] = {
    0x30, 0x32, 0x34, 0x36
};

// Change this to false if you want to restore automatic addressing:
// 0x30 + logical index.
const bool useCustomAddresses = true;

LidarArray lidar(LidarSensorModel::VL53L4CD);

uint8_t expectedAddressForIndex(uint8_t index)
{
    if (useCustomAddresses)
    {
        return customSensorAddresses[index];
    }

    // Default library rule when no manual address array is provided.
    return 0x30 + index;
}

void setup()
{
    Serial.begin(115200);
    Wire.begin();

    lidar.setLayout(1, 4, pcf8574Addresses, xshutPins);
    lidar.setTimeout(100);
    lidar.setVL53L4CDTiming(50, 0);

    // Optional: override the final addresses assigned automatically.
    // Rules:
    // - every address must be unique
    // - addresses must not collide with the PCF8574
    // - do not use 0x29, because it is the sensor boot address
    if (useCustomAddresses)
    {
        lidar.setSensorAddresses(customSensorAddresses);
    }

    if (!lidar.begin()) {
        Serial.println("Partial initialization detected.");
    }

    Serial.println("Address summary:");
    for (uint8_t i = 0; i < lidar.getSensorCount(); ++i)
    {
        LidarReading reading = lidar.readReading(i, false);

        Serial.print("Sensor ");
        Serial.print(i);
        Serial.print(" channel=");
        Serial.print(xshutPins[i]);
        Serial.print(" expected=0x");
        if (expectedAddressForIndex(i) < 0x10) {
            Serial.print('0');
        }
        Serial.print(expectedAddressForIndex(i), HEX);

        Serial.print(" actual=0x");
        if (reading.address < 0x10) {
            Serial.print('0');
        }
        Serial.println(reading.address, HEX);
    }
}

void loop()
{
    // This example focuses on the final address assignment.
    delay(1000);
}
