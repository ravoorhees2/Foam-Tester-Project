#include "HX711.h"

// HX711 wiring
const int LOADCELL_DOUT_PIN = 3;
const int LOADCELL_SCK_PIN = 2;

HX711 scale;

// Replace this with your actual calibration factor
float calibration_factor = -30100;

void setup() {
  Serial.begin(9600);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(calibration_factor);

  Serial.println("Taring... Remove any weight.");
  delay(2000);
  scale.tare();
  Serial.println("Tare complete. Starting continuous readings...");
}

void loop() {
  if (scale.is_ready()) {
    float weight = scale.get_units(5);  // average of 5 readings
    Serial.print("Weight: ");
    Serial.print(weight, 3);
    Serial.println(" units");
  } else {
    Serial.println("HX711 not found.");
  }

  delay(200); // adjust this delay to control sampling rate
}
