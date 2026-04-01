#include <LidarArray.h>

// Cada endereco aqui representa um PCF8574 presente no barramento.
// Neste exemplo existe apenas um expansor em 0x20.
uint8_t pcf8574Addresses[] = {0x20};

// A ordem deste mapa define a ordem logica dos sensores.
// Se o canal 0 vier primeiro, ele sera lido como Sensor 0.
uint8_t xshutPins[1][8] = {
    {0, 1, 2, 3, 4, 5, 6, 7}
};

// API legada: simples quando voce quer apenas ler distancias.
LidarArray lidar(1, 8, pcf8574Addresses, xshutPins);

void setup() 
{
    Serial.begin(115200);
    Wire.begin();

    // A biblioteca vai ligar um sensor por vez, iniciar em 0x29
    // e reenderecar sequencialmente a partir de 0x30.
    if (!lidar.initSensors()) {
        Serial.println("Falha ao inicializar todos os sensores.");
    }
}

void loop() 
{
    for (uint8_t i = 0; i < lidar.getSensorCount(); i++) 
    {
        // O indice i representa a ordem logica definida no mapa xshutPins.
        uint16_t distancia = lidar.readSensor(i);

        Serial.print("Sensor ");
        Serial.print(i);
        Serial.print(": ");
        Serial.print(distancia);
        Serial.print(" mm\t");
    }
    Serial.println();
    delay(100);
}
