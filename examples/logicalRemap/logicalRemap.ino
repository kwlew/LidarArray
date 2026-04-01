#include <LidarArray.h>

uint8_t pcf8574Addresses[] = {0x20};

// Este mapa define a ordem logica dos sensores.
// Trocar a ordem aqui muda quem sera Sensor 0, Sensor 1, Sensor 2...
//
// Exemplo:
// - se o sensor fisico que voce quer chamar de Sensor 0 estiver no canal 3,
//   coloque o canal 3 na primeira posicao.
// - se o sensor fisico que voce quer chamar de Sensor 3 estiver no canal 0,
//   coloque o canal 0 na quarta posicao.
//
// Ordem original:
// {0, 1, 2, 3, 4, 5, 6, 7}
//
// Ordem remapeada:
uint8_t xshutPins[1][8] = {
    {3, 1, 2, 0, 4, 5, 6, 7}
};

LidarArray lidar(1, 4, pcf8574Addresses, xshutPins);

void setup()
{
    Serial.begin(115200);
    Wire.begin();

    Serial.println("Exemplo de remapeio logico.");
    Serial.println("O canal fisico 3 agora sera o Sensor 0.");
    Serial.println("O canal fisico 0 agora sera o Sensor 3.");

    if (!lidar.initSensors()) {
        Serial.println("Inicializacao parcial detectada.");
    }
}

void loop()
{
    for (uint8_t i = 0; i < lidar.getSensorCount(); ++i)
    {
        Serial.print("Indice logico ");
        Serial.print(i);
        Serial.print(" -> distancia = ");
        Serial.print(lidar.readSensor(i));
        Serial.println(" mm");
    }

    Serial.println();
    delay(120);
}
