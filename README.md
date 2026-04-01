# LidarArray 1.1.0

Biblioteca para gerenciar arrays de sensores ToF `VL53L0X` ou `VL53L4CD` usando `PCF8574` para controlar os pinos `XSHUT`.

Esta versao 1.1.0 foca em:

- configuracao minima mais pratica
- debug nativo em `Print`
- inicializacao sequencial previsivel
- organizacao logica dos sensores por software
- enderecamento manual opcional por sensor

## Dependencias

- `VL53L0X` da Pololu
- `VL53L4CD` da Pololu

O sketch continua responsavel por chamar `Wire.begin()` ou `Wire.begin(SDA, SCL)` antes da inicializacao da biblioteca.

## Header recomendado

Use preferencialmente:

```cpp
#include <LidarArray.h>
```

O wrapper raiz `lidarArray.h` foi mantido apenas por compatibilidade.

## Configuracao minima

### VL53L0X com API legada

```cpp
#include <LidarArray.h>

uint8_t pcf8574Addresses[] = {0x20};
uint8_t xshutPins[1][8] = {
    {0, 1, 2, 3, 4, 5, 6, 7}
};

LidarArray lidar(1, 4, pcf8574Addresses, xshutPins);

void setup() {
    Serial.begin(115200);
    Wire.begin();
    lidar.initSensors(20000, 14, 10, 100);
}
```

Use esse caminho quando quiser manter o fluxo legado de `readSensor()` e `readSensorNB()`.

### VL53L4CD com configuracao curta

```cpp
#include <LidarArray.h>

const uint8_t pcf8574Addresses[] = {0x20};
const uint8_t xshutPins[] = {0, 1, 2, 3, 4, 5, 6, 7};

LidarArray tofarr(LidarSensorModel::VL53L4CD);

void setup() {
    Serial.begin(115200);
    Wire.begin();

    tofarr.setLayout(1, 4, pcf8574Addresses, xshutPins);
    tofarr.setTimeout(100);
    tofarr.setVL53L4CDTiming(50, 0);

    tofarr.begin();
}
```

Os campos realmente obrigatorios para começar sao:

- modelo do sensor
- quantidade de PCF8574
- quantidade de sensores ativos
- enderecos dos PCF8574
- mapa `xshutPins`

O restante pode ficar nos defaults e ser ajustado depois.

### Configurando pela struct

Se preferir guardar a configuracao em uma struct curta:

```cpp
auto config = LidarArrayConfig::forVL53L4CD(1, 4, pcf8574Addresses, xshutPins);
config.timeoutMs = 100;

LidarArray tofarr(config);
```

Tambem e possivel editar a configuracao acumulada antes do `begin()`:

```cpp
LidarArray tofarr(LidarSensorModel::VL53L4CD);

tofarr.config().numPCF = 1;
tofarr.config().numSensors = 4;
tofarr.config().pcf8574Addresses = pcf8574Addresses;
tofarr.config().xshutPins = xshutPins;
tofarr.config().timeoutMs = 100;
tofarr.config().vl53l4cdTimingBudgetMs = 50;
```

### Enderecos manuais opcionais

Por padrao, a biblioteca usa `0x30 + indice logico`.

Se quiser definir manualmente o endereco final de cada sensor, passe um array com `numSensors` posicoes:

```cpp
const uint8_t sensorAddresses[] = {0x30, 0x32, 0x34, 0x36};

tofarr.setSensorAddresses(sensorAddresses);
```

Faça isso antes de chamar `begin()`.

Tambem funciona pela struct:

```cpp
auto config = LidarArrayConfig::forVL53L4CD(1, 4, pcf8574Addresses, xshutPins);
config.sensorAddresses = sensorAddresses;
```

Regras para esse array:

- cada endereco precisa ser unico
- nao pode colidir com o endereco de nenhum `PCF8574`
- nao use `0x29`, porque esse e o endereco padrao usado durante a inicializacao sequencial
- se quiser voltar ao modo automatico, passe `nullptr` em `setSensorAddresses()`

## Como a biblioteca organiza os sensores

### Ordem logica

A ordem logica dos sensores segue a ordem do mapa `xshutPins`.

Exemplo:

```cpp
const uint8_t xshutPins[] = {
    0, 1, 2, 3, 4, 5, 6, 7
};
```

Nesse caso:

- `readReading(0)` corresponde ao canal `0`
- `readReading(1)` corresponde ao canal `1`
- `readReading(2)` corresponde ao canal `2`

Se voce trocar a ordem do mapa, troca tambem a ordem logica dos indices.

### Enderecos I2C finais

Durante a inicializacao, cada sensor e ligado sozinho, inicializado no endereco padrao e depois reenderecado.

Hoje o contrato da biblioteca e:

- sensor ToF padrao antes do reenderecamento: `0x29`
- modo automatico: enderecos finais `0x30 + indice logico`
- modo manual: enderecos finais vindos do array configurado em `setSensorAddresses()` ou `config.sensorAddresses`

Exemplo para 4 sensores:

- indice `0` -> `0x30`
- indice `1` -> `0x31`
- indice `2` -> `0x32`
- indice `3` -> `0x33`

Exemplo manual para 4 sensores:

- indice `0` -> `0x30`
- indice `1` -> `0x32`
- indice `2` -> `0x34`
- indice `3` -> `0x36`

Os indices logicos continuam vindo do `xshutPins`. O que muda e apenas o endereco final que cada sensor recebe no barramento.

## Trocar sensores de lugar via software

Nesta versao, “trocar sensor de lugar via software” significa trocar a ordem logica do mapa `xshutPins`, nao mover o hardware.

Exemplo de ordem original:

```cpp
const uint8_t xshutPins[] = {
    0, 1, 2, 3, 4, 5, 6, 7
};
```

Exemplo de ordem remapeada:

```cpp
const uint8_t xshutPins[] = {
    3, 1, 2, 0, 4, 5, 6, 7
};
```

Com isso:

- o sensor ligado no canal fisico `3` passa a ser o indice logico `0`
- o sensor ligado no canal fisico `0` passa a ser o indice logico `3`

Esse remapeio e util quando:

- a instalacao fisica ja esta pronta
- os sensores foram montados em ordem diferente da desejada
- voce quer alinhar o indice logico com a geometria do robô sem refazer cabos

## Debug de inicializacao

O debug foi pensado para mostrar o passo a passo do barramento durante o boot.

Exemplo:

```cpp
LidarArrayDebugConfig debugConfig =
    LidarArrayDebugConfig::verbose(&Serial, true, true, true, 700, 2500);

lidar.setDebugConfig(debugConfig);
lidar.begin();
```

Campos principais:

- `scanBeforeInit`: faz um scan antes de derrubar os ToF
- `scanEachStep`: faz scan apos cada sensor ser adicionado
- `bootDelayMs`: pausa antes de comecar a sequencia, util para abrir o monitor serial
- `animationDelayMs`: pausa entre as etapas de configuracao dos sensores

Comportamento esperado:

1. `scanBeforeInit` mostra o barramento como ele estava antes da biblioteca agir.
2. Depois do shutdown, o endereco `0x29` deve desaparecer se todos os ToF foram realmente desligados.
3. A cada sensor inicializado, deve aparecer `0x30`, depois `0x31`, depois `0x32` e assim por diante.

Observacao:

- o delay animado vale para as etapas de configuracao dos sensores
- o scan I2C em si nao pausa por endereco

## Leitura detalhada

```cpp
LidarReading reading = lidar.readReading(0, true);

Serial.print(reading.distanceMm);
Serial.print(" mm, status=");
Serial.println(reading.status);
```

Campos de `LidarReading`:

- `distanceMm`: ultima distancia lida
- `status`: status bruto do sensor ou status interno de indisponibilidade
- `valid`: leitura valida para uso
- `timeout`: timeout detectado pelo driver
- `dataReady`: indica se havia dado pronto na leitura
- `address`: endereco I2C atribuido ao sensor

## Troubleshooting rapido

### Vejo `0x29` no scan antes da inicializacao

Isso e normal quando algum ToF ainda esta acordado antes da biblioteca assumir o controle.

Esse endereco nao deve ser usado como endereco final manual em `setSensorAddresses()`.

### `0x29` nao some depois do shutdown

Revise:

- mapa `xshutPins`
- ligacao do `PCF8574`
- alimentacao dos sensores

### Alguns sensores ficam como `sensor unavailable`

Isso indica falha de inicializacao naquele indice especifico. O restante do array pode continuar funcionando.

Use:

- `getInitializedSensorCount()`
- `isSensorReady(i)`
- `scanEachStep`

para descobrir em que ponto a sequencia falhou.

### `status=254`

Esse status e interno da biblioteca e indica sensor nao pronto ou indisponivel para leitura naquele indice.

### PlatformIO continua mostrando codigo antigo

Se o projeto de testes continuar preso em cache:

- apague a pasta `.pio/` do projeto `testes`
- rode o build novamente

## Exemplos publicos

- `examples/basic`
  - fluxo legado simples
- `examples/usingTwoPCF`
  - dois expansores e numeracao continua
- `examples/vl53l4cdConfig`
  - configuracao minima usando a API atual
- `examples/logicalRemap`
  - mostra como trocar a ordem logica dos sensores
- `examples/guidedDebug`
  - mostra o debug de boot e o scan passo a passo
- `examples/addressOverview`
  - mostra como inspecionar os enderecos finais automaticos ou manuais

## Projeto de testes

A validacao manual fica no projeto PlatformIO:

`C:\Users\czjoa\OneDrive\Documentos\Projetos\Bibliotecas\testes`

O foco desta rodada continua sendo o `ESP32-P4` da Waveshare.

## Observacoes finais

- Um `LidarArray` suporta apenas um modelo de sensor por instancia.
- Sensores alem de `numSensors` permanecem em shutdown.
- `Vector.h` continua no repositorio, mas nao faz mais parte do fluxo interno.
- Falhas de inicializacao nao travam o restante do array.

## Licenca

MIT.
