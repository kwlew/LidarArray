#include <LidarArray.h>

// Each address here represents one PCF8574 found on the bus.
// This example uses a single expander at 0x20.
uint8_t pcf8574Addresses[] = {0x20};

// The order of this map defines the logical sensor order.
// If channel 0 comes first, it will be read as Sensor 0.
uint8_t xshutPins[1][8] = {
    {0, 1, 2, 3, 4, 5, 6, 7}
};

// Legacy API: simple when you only want distance values.
LidarArray lidar(1, 8, pcf8574Addresses, xshutPins);

void setup() 
{
    Serial.begin(115200);
    Wire.begin();

    // The library will enable one sensor at a time, start it at 0x29,
    // and then readdress it sequentially starting at 0x30.
    if (!lidar.initSensors()) {
        Serial.println("Failed to initialize all sensors.");
    }
}

void loop() 
{
    for (uint8_t i = 0; i < lidar.getSensorCount(); i++) 
    {
        // The i index follows the logical order defined by xshutPins.
        uint16_t distance = lidar.readSensor(i);

        Serial.print("Sensor ");
        Serial.print(i);
        Serial.print(": ");
        Serial.print(distance);
        Serial.print(" mm\t");
    }
    Serial.println();
    delay(100);
}
