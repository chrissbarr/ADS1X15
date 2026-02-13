# ADS1X15 Arduino Library

[![PlatformIO CI](https://github.com/chrissbarr/ADS1X15/actions/workflows/main.yml/badge.svg)](https://github.com/chrissbarr/ADS1X15/actions/workflows/main.yml)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![PlatformIO Registry](https://badges.registry.platformio.org/packages/chrissb/library/ADS1X15.svg)](https://registry.platformio.org/libraries/chrissb/ADS1X15)

This is an Arduino library for the [ADS1015 (12-bit)](https://www.ti.com/product/ADS1015) and [ADS1115 (16-bit)](https://www.ti.com/product/ADS1115) I2C ADC chips. It is based on the [Adafruit ADS1X15 Library](https://github.com/adafruit/Adafruit_ADS1X15) and has a very similar interface.

## Key Features
- **Dual chip support:** ADS1015 (12-bit) and ADS1115 (16-bit) with a common API
- **Single-ended and differential:** Read individual channels or differential pairs
- **Configurable PGA:** Programmable gain amplifier with six voltage ranges (±0.256V to ±6.144V)
- **Flexible I2C support:** Works with any Wire-compatible library via C++ templates (hardware or software I2C)
- **Non-blocking reads:** Start conversions asynchronously and poll for completion
- **Comparator mode:** Hardware threshold comparator with configurable alert pin
- **Configurable data rate:** 128–3300 SPS (ADS1015) or 8–860 SPS (ADS1115)

## Chip Comparison

| Feature | ADS1015 | ADS1115 |
|---------|---------|---------|
| Resolution | 12-bit | 16-bit |
| Max sample rate | 3300 SPS | 860 SPS |
| Default sample rate | 1600 SPS | 128 SPS |
| Input channels | 4 single-ended / 2 differential | 4 single-ended / 2 differential |
| I2C address range | 0x48–0x4B | 0x48–0x4B |
| Supply voltage | 2.0–5.5V | 2.0–5.5V |

## API Reference

### Enums

All enums are in the `ADS1X15` namespace.

| Enum | Values | Description |
|------|--------|-------------|
| `Gain` | `TWOTHIRDS_6144MV`, `ONE_4096MV`, `TWO_2048MV`, `FOUR_1024MV`, `EIGHT_512MV`, `SIXTEEN_256MV` | PGA voltage range selection (default: ±6.144V) |
| `Rate` | `ADS1015_128SPS` – `ADS1015_3300SPS`, `ADS1115_8SPS` – `ADS1115_860SPS` | Conversion rate (separate values for each chip) |
| `DifferentialPair` | `PAIR_01`, `PAIR_03`, `PAIR_13`, `PAIR_23` | Input multiplexer differential pair selection |

### Methods

**Initialisation**
- `void begin(uint8_t address = 0x48)` — Initialise with I2C address (default 0x48).

**Configuration**
- `void setGain(Gain gain)` — Set PGA gain/voltage range.
- `Gain getGain() const` — Get current gain setting.
- `void setDataRate(Rate rate)` — Set conversion sample rate.
- `Rate getDataRate() const` — Get current sample rate.

**Blocking ADC Reads**
- `int16_t readADCSingleEnded(uint8_t channel)` — Read a single channel (0–3). Blocks until conversion completes.
- `int16_t readADCDifferential(DifferentialPair pair)` — Read a differential pair. Blocks until conversion completes.

**Non-Blocking ADC Reads**
- `void startADCReading(uint16_t mux, bool continuous)` — Start a single or continuous conversion.
- `bool conversionComplete()` — Check if a conversion has finished.
- `int16_t getLastConversionResults()` — Retrieve the result of the last conversion.

**Comparator Mode**
- `void startComparatorSingleEnded(uint8_t channel, int16_t threshold)` — Start comparator on a channel with a threshold value.

**Utility**
- `float computeVolts(int16_t count) const` — Convert a raw ADC count to voltage.

## Installation

**PlatformIO:**
Add to your `platformio.ini`:
```ini
lib_deps = chrissb/ADS1X15
```

**Manual:**
Clone or download this repository into your Arduino `libraries/` folder.

## Quick Start

Here's a minimal example reading all four single-ended channels:

```cpp
#include <Wire.h>
#include "ADS1X15.h"

using namespace ADS1X15;

ADS1015<TwoWire> ads(Wire);

void setup() {
  Serial.begin(9600);
  ads.begin();
}

void loop() {
  for (uint8_t ch = 0; ch < 4; ch++) {
    int16_t raw = ads.readADCSingleEnded(ch);
    float volts = ads.computeVolts(raw);
    Serial.print("AIN"); Serial.print(ch);
    Serial.print(": "); Serial.print(volts); Serial.println("V");
  }
  delay(1000);
}
```

For 16-bit resolution, use `ADS1115<TwoWire>` instead of `ADS1015<TwoWire>`.

## I2C Address

The ADS1X15 I2C address is configured using the ADDR pin:

| ADDR Pin | I2C Address |
|----------|-------------|
| GND      | 0x48        |
| VDD      | 0x49        |
| SDA      | 0x4A        |
| SCL      | 0x4B        |

**Default address:** 0x48 (ADDR pin tied to GND)

**Multiple devices:** Up to 4 ADS1X15 chips can share a single I2C bus by configuring different addresses.

## Wiring

Basic connection to an Arduino:

| Arduino Pin | ADS1X15 Pin | Notes |
|-------------|-------------|-------|
| 5V or 3.3V  | VDD        | Supply voltage (2.0–5.5V) |
| GND         | GND        | Ground |
| SDA (A4 on Uno) | SDA    | I2C data line |
| SCL (A5 on Uno) | SCL    | I2C clock line |
| GND         | ADDR       | I2C address select (see table above) |
| Digital Pin (optional) | ALRT | Alert/data-ready output (active low, open-drain) |
| Analog input | AIN0–AIN3 | Analog input channels |

## Advanced Usage

### Using Alternative I2C Implementations

The library uses C++ templates to support different I2C implementations. Pass the I2C interface type as a template parameter:

**Hardware I2C (Arduino Wire):**
```cpp
#include <Wire.h>
#include "ADS1X15.h"

ADS1X15::ADS1015<TwoWire> ads(Wire);
```

**Software I2C (AceWire):**
```cpp
#include <AceWire.h>
#include "ADS1X15.h"

using ace_wire::SimpleWireInterface;
SimpleWireInterface wire(SDA_PIN, SCL_PIN, DELAY_MICROS);
ADS1X15::ADS1015<SimpleWireInterface> ads(wire);
```

**Software I2C (SoftwareWire):**
```cpp
#include <SoftwareWire.h>
#include "ADS1X15.h"

SoftwareWire wire(SDA_PIN, SCL_PIN);
ADS1X15::ADS1015<SoftwareWire> ads(wire);
```

See the [softi2c-acewire](examples/softi2c-acewire) and [softi2c-softwarewire](examples/softi2c-softwarewire) examples for complete code.

### Differential Readings

Read the voltage difference between two input pins:

```cpp
using namespace ADS1X15;

int16_t raw = ads.readADCDifferential(DifferentialPair::PAIR_01);
float volts = ads.computeVolts(raw);
```

Available pairs: `PAIR_01` (AIN0–AIN1), `PAIR_03` (AIN0–AIN3), `PAIR_13` (AIN1–AIN3), `PAIR_23` (AIN2–AIN3).

### Continuous / Non-Blocking Reads

Start a conversion and poll for completion without blocking:

```cpp
ads.startADCReading(ADS1X15_REG_CONFIG_MUX_SINGLE_0, true);

// Later, in loop:
if (ads.conversionComplete()) {
  int16_t result = ads.getLastConversionResults();
}
```

See the [continuous](examples/continuous) example for a complete implementation with interrupt-driven data-ready notification.

### Comparator Mode

Set up a hardware comparator to assert the ALRT pin when a threshold is exceeded:

```cpp
ads.startComparatorSingleEnded(0, 1000);  // Channel 0, threshold ~3.0V

// In loop — reading the result de-asserts the alert:
int16_t result = ads.getLastConversionResults();
```

See the [comparator](examples/comparator) example for complete code.

## Examples

The following example sketches are included:

| Example | Description |
|---------|-------------|
| [singleended](examples/singleended) | Read all four single-ended channels and print voltages |
| [differential](examples/differential) | Read differential voltage between an input pair |
| [continuous](examples/continuous) | Continuous conversion with interrupt-driven data-ready |
| [comparator](examples/comparator) | Hardware comparator mode with alert pin |
| [softi2c-acewire](examples/softi2c-acewire) | Software I2C via AceWire library |
| [softi2c-softwarewire](examples/softi2c-softwarewire) | Software I2C via SoftwareWire library |

## Platform Compatibility

This library has been tested on the following platforms:

| Platform | Architecture | Status |
|----------|-------------|--------|
| Arduino Uno | AVR (ATmega328P) | Tested |
| Teensy 3.1 | ARM Cortex-M4 | Tested |
| Arduino Due | ARM Cortex-M3 | Tested |
| ESP32 | Xtensa LX6 | Tested |

The library should work on any platform that supports the Arduino Wire library or compatible I2C implementations.

## Dependencies

The library makes no assumptions about platform and does not include any platform-specific headers.

The library class is a C++ template parameterised on the I2C interface type (`template <typename WIRE>`), so it works with any I2C implementation that provides the standard Arduino TwoWire API (`begin()`, `beginTransmission()`, `write()`, etc.).

Known compatible libraries include:
- Arduino Wire library (included with most Arduino-compatible platforms)
- [AceWire](https://github.com/bxparks/AceWire) — software I2C support
- [SoftwareWire](https://github.com/Testato/SoftwareWire) — alternative software I2C

## Contributing

Contributions are welcome! Here's how you can help:

1. **Report bugs:** Open an issue on [GitHub Issues](https://github.com/chrissbarr/ADS1X15/issues)
2. **Submit pull requests:**
   - Ensure all CI tests pass
   - Follow the existing code style (uses clang-format)
   - Add examples for new features when applicable

## License

This library is licensed under [GPL-3.0-or-later](LICENSE.txt).
