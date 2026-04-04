# LidarArray 1.2.0

## English

`LidarArray` is an Arduino library for managing `VL53L0X` or `VL53L4CD` ToF sensor arrays through `PCF8574` expanders that control the `XSHUT` lines.

Version `1.2.0` includes:

- support for `VL53L0X` and `VL53L4CD`
- preserved legacy dense `VL53L0X` flow
- sparse physical mapping through `LidarSensorSlot`
- public logical sensor IDs and ID-based reads
- automatic or manual final I2C addressing
- native I2C scan and step-by-step initialization debug
- configurable `TwoWire` bus selection
- English hover documentation in `LidarArray.h`

### Dependencies

- Pololu `VL53L0X`
- Pololu `VL53L4CD`

Your sketch must still call `Wire.begin()` or `Wire.begin(SDA, SCL)` before using the library.

### Recommended Include

```cpp
#include <LidarArray.h>
```

The root wrapper `lidarArray.h` is still available for backward compatibility.

### Layout Modes

`LidarArray` supports two public layout styles:

- legacy dense layout with `xshutPins`
- sparse layout with `LidarSensorSlot`

Use the dense layout when sensors follow the classic sequential mapping by PCF block. Use the sparse layout when sensors are spread across arbitrary `PCF8574` pins and you want explicit physical mapping.

### LidarSensorSlot Cheat Sheet

The sparse mapping API uses the following slot order:

```cpp
{pcfIndex, pin, address, sensorId}
```

Example:

```cpp
const LidarSensorSlot sensorMap[] = {
    {0, 3, 0, 0},
    {0, 4, 0, 1},
    {0, 7, 0, 2},
    {1, 4, 0x36, 10},
    {1, 6, 0, 11}
};
```

Field meaning:

- `pcfIndex`: which `PCF8574` controls the sensor `XSHUT`
- `pin`: physical `PCF8574` pin, always `0..7`
- `address`: final sensor address, `0` for automatic assignment
- `sensorId`: public logical ID, `-1` to reuse the internal index

### Internal Index vs Public ID vs Physical Position

The library separates three concepts:

- internal index: always `0..N-1`
- public `sensorId`: optional user-facing label such as `10`, `11`, `12`
- physical position: the pair `pcfIndex + pin`

This means you can keep a compact internal array while exposing sensor names that match your robot geometry or application naming.

### Addressing Rules

Default behavior:

- sensor boot address: `0x29`
- final address: `0x30 + internal index`

Manual addressing is also supported:

- with dense legacy layout, use `setSensorAddresses(...)` or `config.sensorAddresses`
- with sparse layout, define the final address in each `LidarSensorSlot`

Rules:

- every final address must be unique
- final addresses must not collide with any `PCF8574`
- `0x29` must not be used as a final address
- do not combine `sensorMap` with `setSensorAddresses()`

### Quick Start

#### Legacy VL53L0X

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

void loop() {
    for (uint8_t i = 0; i < lidar.getSensorCount(); ++i) {
        Serial.println(lidar.readSensor(i));
    }
}
```

Use this path when you want the original dense `VL53L0X` flow and legacy `readSensor()` behavior.

#### Current VL53L4CD Setup

```cpp
#include <LidarArray.h>

const uint8_t pcf8574Addresses[] = {0x20};
const uint8_t xshutPins[] = {0, 1, 2, 3, 4, 5, 6, 7};

LidarArray lidar(LidarSensorModel::VL53L4CD);

void setup() {
    Serial.begin(115200);
    Wire.begin();

    lidar.setLayout(1, 4, pcf8574Addresses, xshutPins);
    lidar.setTimeout(100);
    lidar.setVL53L4CDTiming(50, 0);
    lidar.begin();
}
```

Minimum required information:

- sensor model
- number of `PCF8574` expanders
- number of active sensors
- `PCF8574` addresses
- `xshutPins` or `sensorMap`

Everything else can stay at the defaults.

#### Sparse Layout with Public IDs

```cpp
#include <LidarArray.h>

const uint8_t pcf8574Addresses[] = {0x20, 0x21};
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

void setup() {
    Serial.begin(115200);
    Wire.begin();

    lidar.setLayout(2, 8, pcf8574Addresses, sensorMap);
    lidar.setTimeout(100);
    lidar.setVL53L4CDTiming(50, 0);
    lidar.begin();
}

void loop() {
    LidarReading reading = lidar.readReadingById(10, true);
    Serial.println(reading.distanceMm);
}
```

This is the recommended API for non-dense layouts and applications that need public sensor IDs.

### Initialization Debug

The library can trace the full initialization sequence through any `Print`, including `Serial`.

```cpp
LidarArrayDebugConfig debugConfig =
    LidarArrayDebugConfig::verbose(&Serial, true, true, true, 500, 1500);

lidar.setDebugConfig(debugConfig);
lidar.begin();
```

Main debug options:

- `scanBeforeInit`
- `scanEachStep`
- `bootDelayMs`
- `animationDelayMs`

Expected sequence:

1. scan the bus before the library changes sensor state
2. shut down all ToF devices
3. bring sensors back one by one
4. assign final addresses and keep scanning if enabled

### Header Hover Docs

The public header `LidarArray.h` includes English Doxygen-style comments for the main public types, fields, and methods. In Arduino IDE and PlatformIO, these comments should appear as hover help for the current API.

### Shipped Examples

- `examples/basic`: basic legacy dense `VL53L0X` usage
- `examples/usingTwoPCF`: dense layout across two `PCF8574` expanders
- `examples/vl53l4cdConfig`: minimal current `VL53L4CD` setup
- `examples/logicalRemap`: reorder logical indices without rewiring hardware
- `examples/guidedDebug`: walk through boot, scans, and debug timing
- `examples/addressOverview`: inspect automatic and manual final addresses
- `examples/sparseSensorMap`: sparse mapping with `LidarSensorSlot` and public IDs

### Notes and Common Pitfalls

- One `LidarArray` instance supports only one sensor model at a time.
- Sensors above `numSensors` remain in shutdown.
- `getSensor()` is only for `VL53L0X` compatibility.
- `readSensor()` and `readSensorNB()` may apply legacy fallback and clamp behavior when legacy mode is active.
- `Vector.h` remains in the repository, but the library no longer depends on it internally.
- Seeing `0x29` before initialization is normal if a ToF sensor is already awake.
- `status = 254` indicates a library-side not-ready or unavailable condition.
- Partial initialization does not prevent ready sensors from being used.

### Public API Reference

#### Public Types

- `LidarSensorModel`: selects `VL53L0X` or `VL53L4CD`
- `LidarDebugLevel`: selects the debug verbosity
- `LidarSensorSlot`: sparse physical slot description
- `LidarReading`: structured measurement result
- `LidarArrayConfig`: full array configuration
- `LidarArrayDebugConfig`: initialization and scan debug configuration

#### Config Builders

- `LidarArrayConfig::defaults(model)`
- `LidarArrayConfig::forModel(..., xshutPins, wire, sensorAddresses)`
- `LidarArrayConfig::forModel(..., sensorMap, wire)`
- `LidarArrayConfig::forVL53L0X(..., xshutPins, wire, sensorAddresses)`
- `LidarArrayConfig::forVL53L0X(..., sensorMap, wire)`
- `LidarArrayConfig::forVL53L4CD(..., xshutPins, wire, sensorAddresses)`
- `LidarArrayConfig::forVL53L4CD(..., sensorMap, wire)`
- `LidarArrayDebugConfig::verbose(...)`

#### Constructors

- `LidarArray(LidarSensorModel model = LidarSensorModel::VL53L0X)`
- `LidarArray(const LidarArrayConfig &config)`
- `LidarArray(uint8_t numPCF, uint8_t numSensors, const uint8_t pcf8574Addresses[], const uint8_t xshutPins[][8])`

#### Initialization

- `begin()`
- `initSensors()`
- `initSensors(int measurementT)`
- `initSensors(int measurementT, uint8_t preRange, uint8_t finalRange)`
- `initSensors(int measurementT, uint8_t preRange, uint8_t finalRange, int timeout)`

#### Configuration Access

- `config()`
- `config() const`
- `debug()`
- `debug() const`

#### Configuration Setters

- `setLayout(..., xshutPins)`
- `setLayout(..., sensorMap)`
- `setSensorAddresses(...)`
- `setWire(...)`
- `setTimeout(...)`
- `setVL53L4CDTiming(...)`
- `setMeasurementTimingBudget(...)`
- `setVcselPulsePeriod(...)`

#### Reading API

- `readSensor(index)`
- `readSensorNB(index)`
- `readSensorById(sensorId)`
- `readSensorNBById(sensorId)`
- `readReading(index, blocking)`
- `readReadingById(sensorId, blocking)`

#### Direct Driver Access

- `getSensor(index)`
- `getVL53L0XSensor(index)`
- `getVL53L4CDSensor(index)`

#### Debug API

- `setDebugConfig(...)`
- `setDebugOutput(...)`
- `setDebugLevel(...)`
- `setDebugScanBeforeInit(...)`
- `setDebugScanEachStep(...)`
- `setDebugBootDelay(...)`
- `setDebugStepDelay(...)`
- `scanI2C()`

#### Diagnostics

- `getSensorCount()`
- `getInitializedSensorCount()`
- `isSensorReady(index)`
- `getSensorId(index)`
- `indexOfSensorId(sensorId)`
- `getSensorSlot(index)`

---

## PT-BR

`LidarArray` e uma biblioteca Arduino para gerenciar arrays de sensores ToF `VL53L0X` ou `VL53L4CD` atraves de expansores `PCF8574` que controlam os pinos `XSHUT`.

A versao `1.2.0` inclui:

- suporte a `VL53L0X` e `VL53L4CD`
- preservacao do fluxo legado denso para `VL53L0X`
- mapeamento fisico esparso com `LidarSensorSlot`
- IDs logicos publicos e leituras por ID
- enderecamento final automatico ou manual
- scan I2C nativo e debug passo a passo da inicializacao
- selecao configuravel de `TwoWire`
- documentacao de hover em ingles dentro de `LidarArray.h`

### Dependencias

- Pololu `VL53L0X`
- Pololu `VL53L4CD`

O sketch continua responsavel por chamar `Wire.begin()` ou `Wire.begin(SDA, SCL)` antes de usar a biblioteca.

### Include Recomendado

```cpp
#include <LidarArray.h>
```

O wrapper raiz `lidarArray.h` continua disponivel por compatibilidade.

### Modos de Layout

`LidarArray` suporta dois estilos publicos de layout:

- layout denso legado com `xshutPins`
- layout esparso com `LidarSensorSlot`

Use o layout denso quando os sensores seguem o mapeamento sequencial classico por bloco de PCF. Use o layout esparso quando os sensores estiverem distribuidos em pinos arbitrarios de um ou mais `PCF8574` e voce quiser declarar o mapeamento fisico explicitamente.

### Cola do LidarSensorSlot

A API de mapeamento esparso usa a seguinte ordem de campos:

```cpp
{pcfIndex, pin, address, sensorId}
```

Exemplo:

```cpp
const LidarSensorSlot sensorMap[] = {
    {0, 3, 0, 0},
    {0, 4, 0, 1},
    {0, 7, 0, 2},
    {1, 4, 0x36, 10},
    {1, 6, 0, 11}
};
```

Significado dos campos:

- `pcfIndex`: qual `PCF8574` controla o `XSHUT`
- `pin`: pino fisico do `PCF8574`, sempre `0..7`
- `address`: endereco final do sensor, `0` para modo automatico
- `sensorId`: ID logico publico, `-1` para reaproveitar o indice interno

### Indice Interno vs sensorId vs Posicao Fisica

A biblioteca separa tres conceitos:

- indice interno: sempre `0..N-1`
- `sensorId` publico: rotulo opcional como `10`, `11`, `12`
- posicao fisica: o par `pcfIndex + pin`

Assim, a biblioteca pode manter arrays internos compactos enquanto voce expoe nomes de sensores que fazem sentido para o robo ou para a aplicacao.

### Regras de Enderecamento

Comportamento padrao:

- endereco de boot do sensor: `0x29`
- endereco final: `0x30 + indice interno`

Tambem existe enderecamento manual:

- no layout denso legado, use `setSensorAddresses(...)` ou `config.sensorAddresses`
- no layout esparso, defina o endereco final dentro de cada `LidarSensorSlot`

Regras:

- todo endereco final precisa ser unico
- enderecos finais nao podem colidir com nenhum `PCF8574`
- `0x29` nao pode ser usado como endereco final
- nao combine `sensorMap` com `setSensorAddresses()`

### Inicio Rapido

#### VL53L0X Legado

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

void loop() {
    for (uint8_t i = 0; i < lidar.getSensorCount(); ++i) {
        Serial.println(lidar.readSensor(i));
    }
}
```

Use esse caminho quando quiser o fluxo denso original de `VL53L0X` e o comportamento legado de `readSensor()`.

#### Configuracao Atual com VL53L4CD

```cpp
#include <LidarArray.h>

const uint8_t pcf8574Addresses[] = {0x20};
const uint8_t xshutPins[] = {0, 1, 2, 3, 4, 5, 6, 7};

LidarArray lidar(LidarSensorModel::VL53L4CD);

void setup() {
    Serial.begin(115200);
    Wire.begin();

    lidar.setLayout(1, 4, pcf8574Addresses, xshutPins);
    lidar.setTimeout(100);
    lidar.setVL53L4CDTiming(50, 0);
    lidar.begin();
}
```

Informacoes minimas necessarias:

- modelo do sensor
- quantidade de `PCF8574`
- quantidade de sensores ativos
- enderecos dos `PCF8574`
- `xshutPins` ou `sensorMap`

Todo o resto pode ficar nos defaults.

#### Layout Esparso com IDs Publicos

```cpp
#include <LidarArray.h>

const uint8_t pcf8574Addresses[] = {0x20, 0x21};
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

void setup() {
    Serial.begin(115200);
    Wire.begin();

    lidar.setLayout(2, 8, pcf8574Addresses, sensorMap);
    lidar.setTimeout(100);
    lidar.setVL53L4CDTiming(50, 0);
    lidar.begin();
}

void loop() {
    LidarReading reading = lidar.readReadingById(10, true);
    Serial.println(reading.distanceMm);
}
```

Essa e a API recomendada para layouts nao densos e para aplicacoes que precisam de IDs publicos.

### Debug da Inicializacao

A biblioteca consegue mostrar a sequencia completa de inicializacao em qualquer `Print`, incluindo `Serial`.

```cpp
LidarArrayDebugConfig debugConfig =
    LidarArrayDebugConfig::verbose(&Serial, true, true, true, 500, 1500);

lidar.setDebugConfig(debugConfig);
lidar.begin();
```

Principais opcoes de debug:

- `scanBeforeInit`
- `scanEachStep`
- `bootDelayMs`
- `animationDelayMs`

Sequencia esperada:

1. scan do barramento antes da biblioteca mudar o estado dos sensores
2. shutdown de todos os ToF
3. retorno dos sensores um por vez
4. atribuicao dos enderecos finais e novos scans, se habilitados

### Documentacao de Hover no Header

O header publico `LidarArray.h` inclui comentarios em ingles no estilo Doxygen para os principais tipos, campos e metodos publicos. No Arduino IDE e no PlatformIO, esses comentarios devem aparecer como ajuda de hover para a API atual.

### Exemplos Publicos

- `examples/basic`: uso basico do fluxo legado denso de `VL53L0X`
- `examples/usingTwoPCF`: layout denso em dois expansores `PCF8574`
- `examples/vl53l4cdConfig`: configuracao minima atual para `VL53L4CD`
- `examples/logicalRemap`: remapeio da ordem logica sem refazer os cabos
- `examples/guidedDebug`: boot guiado com scans e tempos de debug
- `examples/addressOverview`: inspecao de enderecos finais automaticos e manuais
- `examples/sparseSensorMap`: mapeamento esparso com `LidarSensorSlot` e IDs publicos

### Observacoes e Armadilhas Comuns

- Uma instancia de `LidarArray` suporta apenas um modelo de sensor por vez.
- Sensores acima de `numSensors` permanecem em shutdown.
- `getSensor()` existe apenas para compatibilidade com `VL53L0X`.
- `readSensor()` e `readSensorNB()` podem aplicar fallback e clamp quando o modo legado estiver ativo.
- `Vector.h` continua no repositorio, mas a biblioteca nao depende mais dele internamente.
- Ver `0x29` antes da inicializacao e normal se um ToF ja estiver acordado.
- `status = 254` indica um estado interno de nao pronto ou indisponivel.
- Inicializacao parcial nao impede o uso dos sensores que ficaram prontos.

### Referencia da API Publica

#### Tipos Publicos

- `LidarSensorModel`: seleciona `VL53L0X` ou `VL53L4CD`
- `LidarDebugLevel`: seleciona a verbosidade do debug
- `LidarSensorSlot`: descricao de um slot fisico esparso
- `LidarReading`: resultado estruturado de leitura
- `LidarArrayConfig`: configuracao completa do array
- `LidarArrayDebugConfig`: configuracao de debug da inicializacao e dos scans

#### Builders de Configuracao

- `LidarArrayConfig::defaults(model)`
- `LidarArrayConfig::forModel(..., xshutPins, wire, sensorAddresses)`
- `LidarArrayConfig::forModel(..., sensorMap, wire)`
- `LidarArrayConfig::forVL53L0X(..., xshutPins, wire, sensorAddresses)`
- `LidarArrayConfig::forVL53L0X(..., sensorMap, wire)`
- `LidarArrayConfig::forVL53L4CD(..., xshutPins, wire, sensorAddresses)`
- `LidarArrayConfig::forVL53L4CD(..., sensorMap, wire)`
- `LidarArrayDebugConfig::verbose(...)`

#### Construtores

- `LidarArray(LidarSensorModel model = LidarSensorModel::VL53L0X)`
- `LidarArray(const LidarArrayConfig &config)`
- `LidarArray(uint8_t numPCF, uint8_t numSensors, const uint8_t pcf8574Addresses[], const uint8_t xshutPins[][8])`

#### Inicializacao

- `begin()`
- `initSensors()`
- `initSensors(int measurementT)`
- `initSensors(int measurementT, uint8_t preRange, uint8_t finalRange)`
- `initSensors(int measurementT, uint8_t preRange, uint8_t finalRange, int timeout)`

#### Acesso a Configuracao

- `config()`
- `config() const`
- `debug()`
- `debug() const`

#### Setters de Configuracao

- `setLayout(..., xshutPins)`
- `setLayout(..., sensorMap)`
- `setSensorAddresses(...)`
- `setWire(...)`
- `setTimeout(...)`
- `setVL53L4CDTiming(...)`
- `setMeasurementTimingBudget(...)`
- `setVcselPulsePeriod(...)`

#### API de Leitura

- `readSensor(index)`
- `readSensorNB(index)`
- `readSensorById(sensorId)`
- `readSensorNBById(sensorId)`
- `readReading(index, blocking)`
- `readReadingById(sensorId, blocking)`

#### Acesso Direto aos Drivers

- `getSensor(index)`
- `getVL53L0XSensor(index)`
- `getVL53L4CDSensor(index)`

#### API de Debug

- `setDebugConfig(...)`
- `setDebugOutput(...)`
- `setDebugLevel(...)`
- `setDebugScanBeforeInit(...)`
- `setDebugScanEachStep(...)`
- `setDebugBootDelay(...)`
- `setDebugStepDelay(...)`
- `scanI2C()`

#### Diagnostico

- `getSensorCount()`
- `getInitializedSensorCount()`
- `isSensorReady(index)`
- `getSensorId(index)`
- `indexOfSensorId(sensorId)`
- `getSensorSlot(index)`

## License

MIT.
