#include <Servo.h>
#include <LiquidCrystal.h>

// Hardware connections
const int servoPin = 11;
const int trigPin = 13;
const int echoPin = 12;
const int redPin = A0;
const int greenPin = A1;
const int bluePin = A2;
const int buzzerPin = A5;

Servo scanner;
LiquidCrystal lcd(10, 9, 5, 4, 3, 2);

enum Mode { SCANNING, TRACKING };
Mode currentMode = SCANNING;

const int minAngle = 20;
const int maxAngle = 160;
int angle = minAngle;
int direction = 1;

const unsigned long servoInterval = 25;
unsigned long previousServoTime = 0;
const unsigned long sensorInterval = 100;
unsigned long previousSensorTime = 0;

float distanceInches = 0;
const float warningDistance = 48.0;
const float dangerDistance = 24.0;

// Tracking: physical LEFT is increasing servo angle.
const int trackingOffset = 8;
const unsigned long trackingWait = 150;
unsigned long previousTrackingTime = 0;
int trackingStep = 0;
int trackingCenterAngle = 90;
float middleTrackMeasure = 0;
float leftTrackMeasure = 0;
float rightTrackMeasure = 0;
int lostTrackingCount = 0;
const int lostTrackingLimit = 3;

unsigned long previousBeepTime = 0;
bool beepOn = false;
unsigned long previousLCDTime = 0;

// Take a fresh ultrasonic measurement. Zero means no echo.
float readDistanceNow() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  unsigned long duration = pulseIn(echoPin, HIGH, 25000);
  if (duration == 0) return 0;
  return duration * 0.0135 / 2.0;
}

void measureDistance() {
  unsigned long now = millis();
  if (now - previousSensorTime >= sensorInterval) {
    previousSensorTime = now;
    distanceInches = readDistanceNow();
  }
}

void updateServo() {
  unsigned long now = millis();
  if (now - previousServoTime < servoInterval) return;
  previousServoTime = now;
  angle += direction;
  if (angle >= maxAngle) { angle = maxAngle; direction = -1; }
  if (angle <= minAngle) { angle = minAngle; direction = 1; }
  scanner.write(angle);
}

void trackObject() {
  unsigned long now = millis();

  // Step 0: command center and give servo time to arrive.
  if (trackingStep == 0) {
    scanner.write(trackingCenterAngle);
    previousTrackingTime = now;
    trackingStep = 1;
  }
  // Step 1: measure center, then look physically LEFT (+8 degrees).
  else if (trackingStep == 1 && now - previousTrackingTime >= trackingWait) {
    middleTrackMeasure = readDistanceNow();
    scanner.write(constrain(trackingCenterAngle + trackingOffset, minAngle, maxAngle));
    previousTrackingTime = millis();
    trackingStep = 2;
  }
  // Step 2: measure left, then look physically RIGHT (-8 degrees).
  else if (trackingStep == 2 && now - previousTrackingTime >= trackingWait) {
    leftTrackMeasure = readDistanceNow();
    scanner.write(constrain(trackingCenterAngle - trackingOffset, minAngle, maxAngle));
    previousTrackingTime = millis();
    trackingStep = 3;
  }
  // Step 3: measure right, compare, then repeat.
  else if (trackingStep == 3 && now - previousTrackingTime >= trackingWait) {
    rightTrackMeasure = readDistanceNow();

    Serial.print("LEFT: "); Serial.print(leftTrackMeasure);
    Serial.print("  MIDDLE: "); Serial.print(middleTrackMeasure);
    Serial.print("  RIGHT: "); Serial.println(rightTrackMeasure);

    bool middleLost = middleTrackMeasure <= 0 || middleTrackMeasure > warningDistance;
    bool leftLost = leftTrackMeasure <= 0 || leftTrackMeasure > warningDistance;
    bool rightLost = rightTrackMeasure <= 0 || rightTrackMeasure > warningDistance;

    if (middleLost && leftLost && rightLost) lostTrackingCount++;
    else lostTrackingCount = 0;

    if (lostTrackingCount >= lostTrackingLimit) {
      lostTrackingCount = 0;
      trackingStep = 0;
      angle = trackingCenterAngle;
      currentMode = SCANNING;
      previousSensorTime = millis();
      return;
    }

    // Original experimental rule: point toward the smallest VALID reading.
    float closest = 100000.0;
    int nextCenter = trackingCenterAngle;
    if (!middleLost && middleTrackMeasure < closest) {
      closest = middleTrackMeasure;
      nextCenter = trackingCenterAngle;
    }
    if (!leftLost && leftTrackMeasure < closest) {
      closest = leftTrackMeasure;
      nextCenter = trackingCenterAngle + trackingOffset;
    }
    if (!rightLost && rightTrackMeasure < closest) {
      closest = rightTrackMeasure;
      nextCenter = trackingCenterAngle - trackingOffset;
    }

    trackingCenterAngle = constrain(nextCenter, minAngle, maxAngle);
    angle = trackingCenterAngle;
    // Use the center reading for LCD and warnings while tracking.
    distanceInches = middleTrackMeasure;
    trackingStep = 0;
  }
}

void setRGB(bool red, bool green, bool blue) {
  digitalWrite(redPin, red ? HIGH : LOW);
  digitalWrite(greenPin, green ? HIGH : LOW);
  digitalWrite(bluePin, blue ? HIGH : LOW);
}

void updateWarningSystem() {
  unsigned long now = millis();
  if (distanceInches <= 0 || distanceInches > warningDistance) {
    setRGB(false, true, false);
    noTone(buzzerPin);
    beepOn = false;
    return;
  }

  unsigned long beepInterval;
  int frequency;
  if (distanceInches <= dangerDistance) {
    setRGB(true, false, false);
    beepInterval = 150;
    frequency = 1500;
  } else {
    setRGB(true, true, false);
    beepInterval = 700;
    frequency = 800;
  }

  if (now - previousBeepTime >= beepInterval) {
    previousBeepTime = now;
    beepOn = !beepOn;
    if (beepOn) tone(buzzerPin, frequency);
    else noTone(buzzerPin);
  }
}

void updateLCD() {
  unsigned long now = millis();
  if (now - previousLCDTime < 200) return;
  previousLCDTime = now;
  lcd.setCursor(0, 0);
  lcd.print(currentMode == SCANNING ? "SCANNING        " : "TRACKING        ");
  lcd.setCursor(0, 1);
  if (distanceInches <= 0) {
    lcd.print("No echo         ");
  } else {
    lcd.print(distanceInches, 1);
    lcd.print(" in          ");
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  scanner.attach(servoPin);
  scanner.write(angle);
  lcd.begin(16, 2);
  lcd.print("Starting...");
}

void loop() {
  if (currentMode == SCANNING) {
    updateServo();
    measureDistance();
    if (distanceInches > 0 && distanceInches <= warningDistance) {
      currentMode = TRACKING;
      trackingCenterAngle = angle;
      trackingStep = 0;
      lostTrackingCount = 0;
    }
  } else {
    trackObject();
  }

  updateWarningSystem();
  updateLCD();
}
