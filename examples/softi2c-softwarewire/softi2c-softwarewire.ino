#include "ADS1X15.h"
#include <Arduino.h>
#include <SoftwareWire.h>

using namespace ADS1X15;

constexpr int PIN_SDA = 1;
constexpr int PIN_SCL = 2;

SoftwareWire wire(PIN_SDA, PIN_SCL);

ADS1015<SoftwareWire> ads(wire); /* Use this for the 12-bit version */
// ADS1115<SoftwareWire> ads(wire); /* Use this for the 16-bit version */

void setup(void) {
  Serial.begin(9600);
  ads.begin();
  ads.setGain(Gain::TWOTHIRDS_6144MV);
  ads.setDataRate(Rate::ADS1015_250SPS);
}

void loop(void) {
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
  Serial.print("AIN0: ");
  Serial.print(adc0);
  Serial.print("  ");
  Serial.print(volts0);
  Serial.println("V");
  Serial.print("AIN1: ");
  Serial.print(adc1);
  Serial.print("  ");
  Serial.print(volts1);
  Serial.println("V");
  Serial.print("AIN2: ");
  Serial.print(adc2);
  Serial.print("  ");
  Serial.print(volts2);
  Serial.println("V");
  Serial.print("AIN3: ");
  Serial.print(adc3);
  Serial.print("  ");
  Serial.print(volts3);
  Serial.println("V");

  delay(1000);
}