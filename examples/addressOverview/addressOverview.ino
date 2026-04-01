#include <LidarArray.h>

const uint8_t pcf8574Addresses[] = {0x20};
const uint8_t xshutPins[] = {
    0, 1, 2, 3, 4, 5, 6, 7
};
const uint8_t customSensorAddresses[] = {
    0x30, 0x32, 0x34, 0x36
};

// Troque para false se quiser voltar ao comportamento automatico:
// 0x30 + indice logico.
const bool useCustomAddresses = true;

LidarArray lidar(LidarSensorModel::VL53L4CD);

uint8_t expectedAddressForIndex(uint8_t index)
{
    if (useCustomAddresses)
    {
        return customSensorAddresses[index];
    }

    // Regra padrao da biblioteca quando nenhum array manual e informado.
    return 0x30 + index;
}

void setup()
{
    Serial.begin(115200);
    Wire.begin();

    lidar.setLayout(1, 4, pcf8574Addresses, xshutPins);
    lidar.setTimeout(100);
    lidar.setVL53L4CDTiming(50, 0);

    // Opcional: sobrescreve os enderecos finais atribuidos automaticamente.
    // Regras:
    // - todos precisam ser unicos
    // - nao podem colidir com o PCF8574
    // - nao use 0x29, porque ele e o endereco padrao do sensor no boot
    if (useCustomAddresses)
    {
        lidar.setSensorAddresses(customSensorAddresses);
    }

    if (!lidar.begin()) {
        Serial.println("Inicializacao parcial detectada.");
    }

    Serial.println("Resumo de enderecamento:");
    for (uint8_t i = 0; i < lidar.getSensorCount(); ++i)
    {
        LidarReading reading = lidar.readReading(i, false);

        Serial.print("Sensor ");
        Serial.print(i);
        Serial.print(" canal=");
        Serial.print(xshutPins[i]);
        Serial.print(" esperado=0x");
        if (expectedAddressForIndex(i) < 0x10) {
            Serial.print('0');
        }
        Serial.print(expectedAddressForIndex(i), HEX);

        Serial.print(" atual=0x");
        if (reading.address < 0x10) {
            Serial.print('0');
        }
        Serial.println(reading.address, HEX);
    }
}

void loop()
{
    // Este exemplo foca em mostrar o enderecamento final.
    delay(1000);
}
