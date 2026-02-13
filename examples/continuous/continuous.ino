#include "ADS1X15.h"
#include <Arduino.h>
#include <Wire.h>

using namespace ADS1X15;

ADS1015<TwoWire> ads(Wire); /* Use this for the 12-bit version */
// ADS1115<TwoWire> ads(Wire); /* Use this for the 16-bit version */

// Pin connected to the ALERT/RDY signal for new sample notification.
constexpr int READY_PIN = 3;

// This is required on ESP32 to put the ISR in IRAM. Define as
// empty for other platforms. Be careful - other platforms may have
// other requirements.
#ifndef IRAM_ATTR
#define IRAM_ATTR
#endif

volatile bool new_data = false;
void IRAM_ATTR NewDataReadyISR() { new_data = true; }

void setup(void) {
  Serial.begin(9600);
  Serial.println("Hello!");

  Serial.println("Getting single-ended readings from AIN0..3");
  Serial.println("ADC Range: +/- 6.144V (1 bit = 3mV/ADS1015, 0.1875mV/ADS1115)");

  ads.begin();
  ads.setGain(Gain::TWOTHIRDS_6144MV);
  ads.setDataRate(Rate::ADS1015_250SPS);

  pinMode(READY_PIN, INPUT);
  // We get a falling edge every time a new sample is ready.
  attachInterrupt(digitalPinToInterrupt(READY_PIN), NewDataReadyISR, FALLING);

  // Start continuous conversions.
  ads.startADCReading(ADS1X15_REG_CONFIG_MUX_SINGLE_0, /*continuous=*/true);
}

void loop(void) {
  // If we don't have new data, skip this iteration.
  if (!new_data) { return; }

  int16_t results = ads.getLastConversionResults();

  Serial.print("AIN0: ");
  Serial.print(results);
  Serial.print("(");
  Serial.print(ads.computeVolts(results));
  Serial.println("mV)");

  new_data = false;

  // In a real application we probably don't want to do a delay here if we are doing interrupt-based sampling, but we
  // have a delay in this example to avoid writing too much data to the serial port.
  delay(1000);
}