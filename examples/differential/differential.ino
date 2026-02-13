#include "ADS1X15.h"
#include <Arduino.h>
#include <Wire.h>

using namespace ADS1X15;

ADS1015<TwoWire> ads(Wire); /* Use this for the 12-bit version */
// ADS1115<TwoWire> ads(Wire); /* Use this for the 16-bit version */

void setup(void) {
  Serial.begin(9600);
  Serial.println("Hello!");

  Serial.println("Getting differential readings from AIN0 (P) and AIN1 (N)");
  Serial.println("ADC Range: +/- 6.144V (1 bit = 3mV/ADS1015, 0.1875mV/ADS1115)");

  // The ADC input range (or gain) can be changed via the following
  // functions, but be careful never to exceed VDD +0.3V max, or to
  // exceed the upper and lower limits if you adjust the input range!
  // Setting these values incorrectly may destroy your ADC!
  //                                                                       ADS1015  ADS1115
  //                                                                       -------  -------
  // ads.setGain(Gain::TWOTHIRDS_6144MV); // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  // ads.setGain(Gain::ONE_4096MV);       // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  // ads.setGain(Gain::TWO_2048MV);       // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // ads.setGain(Gain::FOUR_1024MV);      // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads.setGain(Gain::EIGHT_512MV);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads.setGain(Gain::SIXTEEN_256MV);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV

  ads.begin();
  ads.setGain(Gain::TWOTHIRDS_6144MV);
}

void loop(void) {
  int16_t results;

  results = ads.readADCDifferential(DifferentialPair::PAIR_01);

  Serial.print("Differential: ");
  Serial.print(results);
  Serial.print("(");
  Serial.print(ads.computeVolts(results));
  Serial.println("V)");

  delay(1000);
}