#include <LidarArray.h>

// Endereco do expansor que controla os pinos XSHUT.
const uint8_t pcf8574Addresses[] = {0x20};

// A ordem deste array vira a ordem logica dos sensores.
// Os primeiros 4 canais serao usados porque o exemplo configura 4 sensores.
const uint8_t xshutPins[] = {
    0, 1, 2, 3, 4, 5, 6, 7
};
const uint8_t sensorAddresses[] = {
    0x30, 0x32, 0x34, 0x36
};
const bool useCustomSensorAddresses = false;

// Exemplo principal de configuracao minima para VL53L4CD.
LidarArray lidar(LidarSensorModel::VL53L4CD);

void setup()
{
    Serial.begin(115200);
    Wire.begin();

    // O minimo obrigatorio e definir:
    // - quantos PCF8574 existem
    // - quantos sensores estao ativos
    // - os enderecos dos PCF8574
    // - o mapa logico dos canais XSHUT
    lidar.setLayout(1, 4, pcf8574Addresses, xshutPins);

    // Ajustes finos opcionais.
    lidar.setTimeout(100);
    lidar.setVL53L4CDTiming(50, 0);

    // Opcional: define manualmente o endereco final de cada sensor.
    // Deixe false para usar o modo automatico (0x30 + indice).
    if (useCustomSensorAddresses) {
        lidar.setSensorAddresses(sensorAddresses);
    }

    // Debug opcional para acompanhar o boot.
    lidar.setDebugOutput(&Serial);
    lidar.setDebugLevel(LidarDebugLevel::Info);
    lidar.setDebugScanBeforeInit(true);

    if (!lidar.begin()) {
        Serial.println("Inicializacao parcial ou falha em algum sensor.");
    }
}

void loop()
{
    for (uint8_t i = 0; i < lidar.getSensorCount(); ++i) {
        // readReading() expõe mais informacoes que readSensor().
        LidarReading reading = lidar.readReading(i);

        Serial.print("Sensor ");
        Serial.print(i);
        Serial.print(" -> ready=");
        Serial.print(lidar.isSensorReady(i) ? "sim" : "nao");
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
