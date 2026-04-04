#include <LidarArray.h>

uint8_t pcf8574Addresses[] = {0x20};

// This map defines the logical order of the sensors.
// Changing the order here changes which device becomes Sensor 0, Sensor 1, Sensor 2...
// For a sparse physical layout across multiple PCFs, use the newer
// LidarSensorSlot-based API shown in sparseSensorMap.
//
// Example:
// - if the physical sensor you want to call Sensor 0 is connected to channel 3,
//   place channel 3 in the first position.
// - if the physical sensor you want to call Sensor 3 is connected to channel 0,
//   place channel 0 in the fourth position.
//
// Original order:
// {0, 1, 2, 3, 4, 5, 6, 7}
//
// Remapped order:
uint8_t xshutPins[1][8] = {
    {3, 1, 2, 0, 4, 5, 6, 7}
};

LidarArray lidar(1, 4, pcf8574Addresses, xshutPins);

void setup()
{
    Serial.begin(115200);
    Wire.begin();

    Serial.println("Logical remap example.");
    Serial.println("Physical channel 3 now becomes Sensor 0.");
    Serial.println("Physical channel 0 now becomes Sensor 3.");

    if (!lidar.initSensors()) {
        Serial.println("Partial initialization detected.");
    }
}

void loop()
{
    for (uint8_t i = 0; i < lidar.getSensorCount(); ++i)
    {
        Serial.print("Logical index ");
        Serial.print(i);
        Serial.print(" -> distance = ");
        Serial.print(lidar.readSensor(i));
        Serial.println(" mm");
    }

    Serial.println();
    delay(120);
}
