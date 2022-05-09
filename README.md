# ADS1X15 Arduino Library
This is an Arduino library for the ADS1015 and ADS1115 I2C ADCs. 

This library is based on the [Adafruit ADS1X15 Library](https://github.com/adafruit/Adafruit_ADS1X15) and has a very similar interface. The main difference is that this library supports the use of software I2C if desired.

## Key Features
- Supports ADS1015 12-bit ADC and ADS1115 16-bit ADC
- Simple interface for retrieving single readings (single-ended or differential) from ADC
- Support for continuous and non-blocking reading (refer examples)
- Supports hardware I2C via Arduino Wire library, or software I2C via third-party libraries (see examples). 
  - Software I2C allows ADS1X15 to be controlled by any two GPIO pins on microcontroller.

## Examples
Refer to the examples folder for more detailed examples on usage. 

### Single Input
This example demonstrates reading a single input pin and printing to the serial port.
```
#include <Arduino.h>
#include <Wire.h>
#include "ADS1X15.h"

using namespace ADS1X15;

ADS1015<TwoWire> ads(Wire);

void setup(void)
{
  Serial.begin(9600);
  ads.begin();
}

void loop(void)
{
  int16_t adc0, adc1, adc2, adc3;
  float volts0, volts1, volts2, volts3;

  adc0 = ads.readADCSingleEnded(0);
  adc1 = ads.readADCSingleEnded(1);
  adc2 = ads.readADCSingleEnded(2);
  adc3 = ads.readADCSingleEnded(3);

  volts0 = ads.computeVolts(adc0);
  volts1 = ads.computeVolts(adc1);
  volts2 = ads.computeVolts(adc2);
  volts3 = ads.computeVolts(adc3);

  Serial.println("-----------------------------------------------------------");
  Serial.print("AIN0: "); Serial.print(adc0); Serial.print("  "); Serial.print(volts0); Serial.println("V");
  Serial.print("AIN1: "); Serial.print(adc1); Serial.print("  "); Serial.print(volts1); Serial.println("V");
  Serial.print("AIN2: "); Serial.print(adc2); Serial.print("  "); Serial.print(volts2); Serial.println("V");
  Serial.print("AIN3: "); Serial.print(adc3); Serial.print("  "); Serial.print(volts3); Serial.println("V");

  delay(1000);
}
```