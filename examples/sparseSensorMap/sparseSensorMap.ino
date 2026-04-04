#include <LidarArray.h>

const uint8_t pcf8574Addresses[] = {0x20, 0x21};

// In the newer API, each item describes:
// - which PCF8574 controls the sensor
// - which physical PCF pin is wired to XSHUT
// - which final address the sensor should receive (0 = automatic)
// - which public ID it should expose (-1 = reuse the internal index)
const LidarSensorSlot sensorMap[] = {
    {0, 3, 0, 0},
    {0, 4, 0, 1},
    {0, 7, 0, 2},
    {1, 0, 0, 10},
    {1, 2, 0, 11},
    {1, 4, 0, 12},
    {1, 5, 0, 13},
    {1, 7, 0, 14}
};

LidarArray lidar(LidarSensorModel::VL53L4CD);

void setup()
{
    Serial.begin(115200);
    Wire.begin();

    // The array order remains the internal logical order 0..N-1.
    // The sensorId field lets you expose different public names, such as 10..14.
    lidar.setLayout(2, 8, pcf8574Addresses, sensorMap);
    lidar.setTimeout(100);
    lidar.setVL53L4CDTiming(50, 0);

    if (!lidar.begin()) {
        Serial.println("Partial initialization or at least one sensor failed.");
    }
}

void loop()
{
    // Here we read by public sensor ID, not by internal index.
    for (int16_t sensorId = 0; sensorId <= 14; ++sensorId)
    {
        const int16_t index = lidar.indexOfSensorId(sensorId);
        if (index < 0) {
            continue;
        }

        const LidarSensorSlot *slot = lidar.getSensorSlot(static_cast<uint8_t>(index));
        LidarReading reading = lidar.readReadingById(sensorId, true);

        Serial.print("ID ");
        Serial.print(sensorId);
        Serial.print(" -> idx=");
        Serial.print(index);
        Serial.print(", pcf=");
        Serial.print(slot != nullptr ? slot->pcfIndex : 255);
        Serial.print(", pin=");
        Serial.print(slot != nullptr ? slot->pin : 255);
        Serial.print(", addr=0x");
        if (reading.address < 0x10) {
            Serial.print('0');
        }
        Serial.print(reading.address, HEX);
        Serial.print(", dist=");
        Serial.print(reading.distanceMm);
        Serial.println(" mm");
    }

    Serial.println();
    delay(150);
}
