#include <LidarArray.h>

// Two expanders provide 16 possible XSHUT channels.
// Logical indices remain sequential even when moving from one PCF to the next.
// This example uses the legacy dense layout.
// If you need sensors on arbitrary pins across multiple PCFs,
// prefer the sparseSensorMap example with LidarSensorSlot.
uint8_t pcf8574Addresses[] = {0x20, 0x21};

// The first 8 logical indices live on PCF 0x20.
// The next 8 logical indices live on PCF 0x21.
uint8_t xshutPins[2][8] = {
    {0, 1, 2, 3, 4, 5, 6, 7},
    {0, 1, 2, 3, 4, 5, 6, 7}
};

LidarArray lidar(2, 16, pcf8574Addresses, xshutPins);

void setup() 
{
    Serial.begin(115200);
    Wire.begin();

    // If one sensor fails, the others can still continue working.
    if (!lidar.initSensors()) {
        Serial.println("Partial initialization detected.");
    }
}

void loop() 
{
    for (uint8_t i = 0; i < lidar.getSensorCount(); i++) 
    {
        uint16_t distance = lidar.readSensor(i);

        // Example:
        // Sensor 0..7  -> PCF 0x20
        // Sensor 8..15 -> PCF 0x21
        Serial.print("Sensor ");
        Serial.print(i);
        Serial.print(": ");
        Serial.print(distance);
        Serial.print(" mm\t");
    }
    Serial.println();
    delay(100);
}
