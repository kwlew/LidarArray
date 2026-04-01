#include <LidarArray.h>

// Dois expansores significam 16 canais XSHUT possiveis.
// Os indices logicos continuam em sequencia mesmo mudando de PCF.
uint8_t pcf8574Addresses[] = {0x20, 0x21};

// Os 8 primeiros indices logicos ficam no PCF 0x20.
// Os 8 seguintes ficam no PCF 0x21.
uint8_t xshutPins[2][8] = {
    {0, 1, 2, 3, 4, 5, 6, 7},
    {0, 1, 2, 3, 4, 5, 6, 7}
};

LidarArray lidar(2, 16, pcf8574Addresses, xshutPins);

void setup() 
{
    Serial.begin(115200);
    Wire.begin();

    // Se algum sensor falhar, os demais ainda podem continuar funcionando.
    if (!lidar.initSensors()) {
        Serial.println("Inicializacao parcial detectada.");
    }
}

void loop() 
{
    for (uint8_t i = 0; i < lidar.getSensorCount(); i++) 
    {
        uint16_t distancia = lidar.readSensor(i);

        // Exemplo:
        // Sensor 0..7  -> PCF 0x20
        // Sensor 8..15 -> PCF 0x21
        Serial.print("Sensor ");
        Serial.print(i);
        Serial.print(": ");
        Serial.print(distancia);
        Serial.print(" mm\t");
    }
    Serial.println();
    delay(100);
}
