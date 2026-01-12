#include <SPI.h>
#include "SdFat.h"
#include "HX711.h"
#include <TM1637Display.h>

// === Pin Definitions ===
const int potPin = A0;
const int LOADCELL_DOUT_PIN = 3;
const int LOADCELL_SCK_PIN = 2;
const int buttonPin = 4;

// RGB LED Pins
const int redPin = 5;
const int greenPin = 6;
const int bluePin = 7;

// TM1637 4-digit 7-segment display
const int displayCLK = 8;
const int displayDIO = 9;
TM1637Display display(displayCLK, displayDIO);

SdFat SD;
File dataFile;

HX711 scale;

const float calibrationFactor = -30100.0;
const float maxDeflection = 50.0;
const int rawMin = 5;
const int rawMax = 1018;

bool isSampling = false;
bool previousButtonState;

unsigned long previousSampleTime = 0;
const unsigned long sampleInterval = 100;
unsigned long startTime = 0;

int potZeroOffset = 0;
int fileIndex = 1;  // Start from 1 instead of 0
char fileName[16];
int sampleCount = 0;
bool displayFileNumber = false;

void setup() {
  pinMode(potPin, INPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(calibrationFactor);
  scale.tare();

  display.setBrightness(7);
  display.showNumberDec(0);

  if (!SD.begin(10)) {
    setLEDColor(255, 0, 0);
    while (1);
  }

  setLEDColor(255, 50, 0);  // Yellow LED on startup
}

void loop() {
  bool currentButtonState = digitalRead(buttonPin);
  if (!currentButtonState && previousButtonState) {
    isSampling = !isSampling;

    if (isSampling) {
      while (true) {
        snprintf(fileName, sizeof(fileName), "DATA%03d.CSV", fileIndex);
        if (!SD.exists(fileName)) break;
        fileIndex++;
      }

      display.showNumberDec(fileIndex);  // Display current file number (1-based index)
      displayFileNumber = true;

      dataFile = SD.open(fileName, FILE_WRITE);
      if (dataFile) {
        dataFile.println("Time (s),Force (N),Deflection (mm)");
        dataFile.close();
      }

      scale.tare();
      potZeroOffset = analogRead(potPin);
      startTime = millis();
      previousSampleTime = 0;
      sampleCount = 0;
      setLEDColor(0, 255, 0);  // Green while sampling
    } else {
      setLEDColor(255, 50, 0);  // Yellow when idle
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

    dataFile = SD.open(fileName, FILE_WRITE);
    if (dataFile) {
      dataFile.print(timeSec, 2);
      dataFile.print(",");
      dataFile.print(forceN, 2);
      dataFile.print(",");
      dataFile.println(deflection, 2);
      dataFile.close();
    }

    sampleCount++;
    if (!displayFileNumber) {
      display.showNumberDec(sampleCount % 10000);
    }
  }
}

void setLEDColor(int redVal, int greenVal, int blueVal) {
  analogWrite(redPin, redVal);
  analogWrite(greenPin, greenVal);
  analogWrite(bluePin, blueVal);
}
