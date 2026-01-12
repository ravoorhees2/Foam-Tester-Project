const int potPin = A0;             // Potentiometer analog input
const int buttonPin = 4;           // Button input pin

// Calibration values (adjust as needed)
const int rawMin = 0;
const int rawMax = 1023;
const float maxDeflection = 50.0;  // mm

bool isSampling = false;
int currentButtonState = HIGH;
bool previousButtonState;

unsigned long prevSampleTime = 0;
const unsigned long sampleInterval = 100; // Sampling interval (ms)

void setup() {
  Serial.begin(9600);
  pinMode(potPin, INPUT);
  pinMode(buttonPin, INPUT_PULLUP); // Assuming active LOW button
  Serial.println("Press button to start or stop sampling.");
}

void loop() {
  unsigned long currentMillis = millis();

  // Check for button press (falling edge)
  currentButtonState = digitalRead(buttonPin);
  if (!currentButtonState && previousButtonState) {
    isSampling = !isSampling;  // Toggle sampling state
    if (isSampling) {
      Serial.println("Sampling started.");
    } else {
      Serial.println("Sampling stopped.");
    }
  }
  previousButtonState = currentButtonState;

  // If sampling, read and print deflection
  if (isSampling && (currentMillis - prevSampleTime >= sampleInterval)) {
    prevSampleTime = currentMillis;

    int rawValue = analogRead(potPin);
    float deflection = (float)(rawValue - rawMin) * maxDeflection / (rawMax - rawMin);
    deflection = constrain(deflection, 0.0, maxDeflection);

    Serial.print("Deflection: ");
    Serial.print(deflection, 2);
    Serial.println(" mm");
  }
}
