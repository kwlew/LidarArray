# LidarArray 1.1.0

## English

Library for managing `VL53L0X` or `VL53L4CD` ToF sensor arrays using `PCF8574` expanders to drive the `XSHUT` lines.

Version `1.1.0` focuses on:

- a shorter configuration flow
- native debug output through `Print`
- predictable sequential sensor initialization
- logical sensor remapping in software
- optional manual I2C addresses per sensor

### Dependencies

- Pololu `VL53L0X`
- Pololu `VL53L4CD`

Your sketch must still call `Wire.begin()` or `Wire.begin(SDA, SCL)` before initializing the library.

### Recommended Header

```cpp
#include <LidarArray.h>
```

The root wrapper `lidarArray.h` is still available only for backward compatibility.

### Minimal Setup

#### Legacy VL53L0X flow

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

Use this path if you want to keep the legacy `readSensor()` / `readSensorNB()` behavior.

#### Short VL53L4CD setup

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

The only required pieces to get started are:

- sensor model
- number of `PCF8574` expanders
- number of active sensors
- `PCF8574` I2C addresses
- `xshutPins` mapping

Everything else can stay at the defaults.

### Logical Order and I2C Addresses

Logical sensor order follows the order of the `xshutPins` map.

By default, final sensor addresses are assigned as:

- sensor boot address: `0x29`
- final address: `0x30 + logical index`

You can also provide a manual address array before `begin()`:

```cpp
const uint8_t sensorAddresses[] = {0x30, 0x32, 0x34, 0x36};
tofarr.setSensorAddresses(sensorAddresses);
```

Rules for manual addresses:

- each address must be unique
- addresses must not collide with any `PCF8574`
- `0x29` must not be used as a final address
- `setSensorAddresses(nullptr)` restores automatic addressing

### Initialization Debug

The library can show the full initialization sequence through `Serial` or any other `Print`.

```cpp
LidarArrayDebugConfig debugConfig =
    LidarArrayDebugConfig::verbose(&Serial, true, true, true, 700, 2500);

lidar.setDebugConfig(debugConfig);
lidar.begin();
```

Main debug options:

- `scanBeforeInit`
- `scanEachStep`
- `bootDelayMs`
- `animationDelayMs`

Expected behavior:

1. `scanBeforeInit` shows the bus before the library changes anything.
2. After shutdown, `0x29` should disappear if all ToF sensors are really off.
3. As sensors are added back one by one, new final addresses should appear on the bus.

### Examples

- `examples/basic`: simple legacy flow
- `examples/usingTwoPCF`: two expanders with continuous sensor numbering
- `examples/vl53l4cdConfig`: minimal current API setup
- `examples/logicalRemap`: change logical order without moving hardware
- `examples/guidedDebug`: guided boot debug and per-step I2C scan
- `examples/addressOverview`: inspect automatic or manual final addresses

### Notes

- One `LidarArray` instance supports only one sensor model at a time.
- Sensors above `numSensors` remain in shutdown.
- `Vector.h` is still in the repository, but it is no longer used internally.
- A full public API reference is available at the end of this README.

---

## PT-BR

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

Os campos realmente obrigatorios para comecar sao:

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

Faca isso antes de chamar `begin()`.

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

Nesta versao, "trocar sensor de lugar via software" significa trocar a ordem logica do mapa `xshutPins`, nao mover o hardware.

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
- voce quer alinhar o indice logico com a geometria do robo sem refazer cabos

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

## Referencia da API publica

### Builders e helpers publicos

- `LidarArrayConfig::defaults(model)`: cria uma configuracao base com os defaults do modelo informado.
- `LidarArrayConfig::forModel(model, numPCF, numSensors, pcf8574Addresses, xshutPins, wire, sensorAddresses)`: cria uma configuracao completa para qualquer modelo suportado.
- `LidarArrayConfig::forVL53L0X(numPCF, numSensors, pcf8574Addresses, xshutPins, wire, sensorAddresses)`: cria uma configuracao pronta para `VL53L0X`.
- `LidarArrayConfig::forVL53L4CD(numPCF, numSensors, pcf8574Addresses, xshutPins, wire, sensorAddresses)`: cria uma configuracao pronta para `VL53L4CD`.
- `LidarArrayDebugConfig::verbose(out, scanBeforeInit, scanEachStep, animated, animationDelayMs, bootDelayMs)`: cria uma configuracao de debug em nivel verbose usando um `Print`.

### Construtores

- `LidarArray(LidarSensorModel model = LidarSensorModel::VL53L0X)`: cria uma instancia nova usando apenas o modelo e os defaults internos.
- `LidarArray(const LidarArrayConfig &config)`: cria a instancia usando uma configuracao pronta.
- `LidarArray(uint8_t numPCF, uint8_t numSensors, const uint8_t pcf8574Addresses[], const uint8_t xshutPins[][8])`: construtor legado para arrays `VL53L0X`.
- `~LidarArray()`: libera os buffers internos alocados pela biblioteca.

### Inicializacao

- `begin()`: inicializa o array usando a configuracao atual e retorna `true` somente se todos os sensores configurados ficarem prontos.
- `initSensors()`: wrapper legado para `begin()` usando os defaults do fluxo antigo.
- `initSensors(int measurementT)`: wrapper legado que ajusta o timing budget do `VL53L0X` antes do `begin()`.
- `initSensors(int measurementT, uint8_t preRange, uint8_t finalRange)`: wrapper legado que ajusta timing budget e periodos VCSEL do `VL53L0X`.
- `initSensors(int measurementT, uint8_t preRange, uint8_t finalRange, int timeout)`: wrapper legado que ajusta timing budget, VCSEL e timeout antes do `begin()`.

### Acesso a configuracao

- `config()`: retorna referencia mutavel para `LidarArrayConfig`, permitindo montar ou alterar a configuracao antes do `begin()`.
- `config() const`: retorna referencia somente leitura para a configuracao atual.
- `debug()`: retorna referencia mutavel para `LidarArrayDebugConfig`.
- `debug() const`: retorna referencia somente leitura para a configuracao de debug atual.

### Setters de configuracao

- `setLayout(numPCF, numSensors, pcf8574Addresses, xshutPins)`: define a topologia do array e o mapa logico dos canais `XSHUT`.
- `setSensorAddresses(sensorAddresses)`: define um array opcional com o endereco final de cada sensor; passando `nullptr`, volta ao modo automatico.
- `setWire(wire)`: define qual instancia de `TwoWire` sera usada pelo array.
- `setTimeout(timeoutMs)`: define o timeout usado pelos drivers Pololu.
- `setVL53L4CDTiming(timingBudgetMs, interMeasurementMs)`: define o timing do `VL53L4CD`.

### Leitura

- `readSensor(sensorIndex)`: faz leitura bloqueante do sensor informado e retorna apenas a distancia; no modo legado aplica clamp e fallback.
- `readSensorNB(sensorIndex)`: faz leitura nao bloqueante e retorna apenas a distancia.
- `readReading(sensorIndex, blocking)`: retorna uma `LidarReading` completa com distancia, status, validade, timeout, data ready e endereco.

### Acesso direto aos drivers

- `getSensor(sensorIndex)`: retorna referencia ao sensor `VL53L0X` para compatibilidade com a API antiga; para outros modelos, nao deve ser usado.
- `getVL53L0XSensor(sensorIndex)`: retorna ponteiro para o `VL53L0X` informado ou `nullptr` se a instancia nao for desse modelo.
- `getVL53L4CDSensor(sensorIndex)`: retorna ponteiro para o `VL53L4CD` informado ou `nullptr` se a instancia nao for desse modelo.

### Ajustes de leitura e debug

- `setMeasurementTimingBudget(timingBudget)`: atualiza o timing budget dos sensores `VL53L0X`.
- `setVcselPulsePeriod(type, period)`: atualiza o periodo VCSEL dos sensores `VL53L0X`; `type 0` significa pre-range e `type 1` significa final-range.
- `setDebugConfig(config)`: substitui toda a configuracao de debug atual.
- `setDebugOutput(out)`: define o destino do log de debug.
- `setDebugLevel(level)`: define o nivel minimo de log emitido.
- `setDebugScanBeforeInit(enabled)`: habilita ou desabilita o scan antes do shutdown dos ToF.
- `setDebugScanEachStep(enabled)`: habilita ou desabilita o scan apos cada sensor inicializado.
- `setDebugBootDelay(delayMs)`: define uma pausa antes da sequencia de boot para facilitar abrir o monitor serial.
- `setDebugStepDelay(delayMs)`: define a pausa entre as etapas de configuracao quando o debug animado estiver ativo.

### Estado e diagnostico

- `getSensorCount() const`: retorna a quantidade total de sensores configurados.
- `getInitializedSensorCount() const`: retorna quantos sensores ficaram prontos apos o `begin()`.
- `isSensorReady(sensorIndex) const`: informa se um indice especifico foi inicializado com sucesso.
- `scanI2C()`: faz um scan no barramento I2C configurado e retorna a quantidade de dispositivos encontrados.
