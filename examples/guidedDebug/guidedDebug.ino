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

    // Layout minimo do array.
    lidar.setLayout(1, 4, pcf8574Addresses, xshutPins);
    lidar.setTimeout(100);
    lidar.setMeasurementTimingBudget(20000);
    lidar.setVcselPulsePeriod(0, 14);
    lidar.setVcselPulsePeriod(1, 10);

    // O objetivo deste exemplo e acompanhar o boot:
    // 1. scan do barramento antes da biblioteca agir
    // 2. pausa para abrir o monitor serial
    // 3. scan apos desligar todos os ToF
    // 4. adiciona um sensor por vez e faz novo scan
    lidar.setDebugOutput(&Serial);
    lidar.setDebugLevel(LidarDebugLevel::Verbose);
    lidar.setDebugScanBeforeInit(true);
    lidar.setDebugScanEachStep(true);
    lidar.setDebugBootDelay(1500);
    lidar.setDebugStepDelay(500);

    bool ok = lidar.begin();
    Serial.print("begin() -> ");
    Serial.println(ok ? "OK" : "PARCIAL/FALHA");
}

void loop()
{
    for (uint8_t i = 0; i < lidar.getSensorCount(); ++i)
    {
        LidarReading reading = lidar.readReading(i, false);

        Serial.print("Sensor ");
        Serial.print(i);
        Serial.print(" ready=");
        Serial.print(lidar.isSensorReady(i) ? "sim" : "nao");
        Serial.print(" status=");
        Serial.print(reading.status);
        Serial.print(" dist=");
        Serial.println(reading.distanceMm);
    }

    Serial.println();
    delay(150);
}
