#include "HX711.h"

// === Pin Definitions ===
const int potPin = A0;
const int LOADCELL_DOUT_PIN = 3;
const int LOADCELL_SCK_PIN = 2;
const int buttonPin = 4;

// RGB LED Pins
const int redPin = 5;
const int greenPin = 6;
const int bluePin = 7;

HX711 scale;

const float calibrationFactor = -30100.0;
const float maxDeflection = 50.0;
const int rawMin = 0;
const int rawMax = 1023;

bool isSampling = false;
bool previousButtonState;

unsigned long previousSampleTime = 0;
const unsigned long sampleInterval = 100;
unsigned long startTime = 0;

int potZeroOffset = 0;

void setup() {
  Serial.begin(9600);

  pinMode(potPin, INPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(calibrationFactor);
  scale.tare();

  setLEDColor(0, 255, 0);  // Green: Idle
  Serial.println("System ready. Press button to start/stop sampling.");
}

void loop() {
  bool currentButtonState = digitalRead(buttonPin);
  if (!currentButtonState && previousButtonState) {
    isSampling = !isSampling;

    if (isSampling) {
      Serial.println("Sampling started...");
      scale.tare();
      potZeroOffset = analogRead(potPin);
      startTime = millis();
      previousSampleTime = 0;
      setLEDColor(0, 0, 255);  // Blue: Sampling
    } else {
      Serial.println("Sampling stopped.");
      setLEDColor(0, 255, 0);  // Green: Idle
    }
  }
  previousButtonState = currentButtonState;

  if (isSampling && millis() - startTime - previousSampleTime >= sampleInterval) {
    previousSampleTime = millis() - startTime;

    int rawValue = analogRead(potPin);
    float deflection = (float)(rawValue) * maxDeflection / (rawMax - rawMin);
    deflection = constrain(deflection, 0.0, maxDeflection);

    float forceLbf = scale.get_units(1);
    float forceN = forceLbf * 4.44822;  

    float timeSec = previousSampleTime / 1000.0;

    Serial.print("Time: ");
    Serial.print(timeSec, 1);
    Serial.print(" s | Force: ");
    Serial.print(forceN, 2);
    Serial.print(" N | Deflection: ");
    Serial.print(deflection, 2);
    Serial.println(" mm");
  }
}

void setLEDColor(int redVal, int greenVal, int blueVal) {
  analogWrite(redPin, redVal);
  analogWrite(greenPin, greenVal);
  analogWrite(bluePin, blueVal);
}

