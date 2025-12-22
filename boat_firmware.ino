/*
 * Boat Control System - with RC Failsafe
 * Hardware: Arduino Nano 33 IoT + RadioLink R8EF
 */

#include <Servo.h>

// --- Config ---
// RC Thresholds
const int RC_MIN = 1100;
const int RC_MAX = 1900;
const int RC_DEADZONE = 50;  // Ignore small stick movements
const int PWM_CENTER = 1500;

// --- Pin Definitions ---
// Outputs
const int PIN_ESC_LEFT = 9;
const int PIN_ESC_RIGHT = 10;
const int PIN_ESC_BACK = 8;
const int PIN_SERVO = 3;

// Inputs (RC Receiver) - VIA VOLTAGE DIVIDER!
const int PIN_RC_STEER = A1; // Connect to R8EF Ch1
const int PIN_RC_THROT = A2; // Connect to R8EF Ch2 (Elevator)
const int PIN_RC_MODE = A3;  // Connect to R8EF Ch5 (Switch)

// --- Objects ---
Servo escLeft;
Servo escRight;
Servo escBack;
Servo myServo;

// --- Variables ---
int steerVal = 0;
int throtVal = 0;
int modeVal = 0;
bool isAutonomous = false;

// Steering Angles
const int ANGLE_CENTER = 90;
const int ANGLE_LEFT = 60;   // Check physical limit
const int ANGLE_RIGHT = 120; // Check physical limit

void setup() {
  Serial.begin(115200);

  // Setup Pins
  pinMode(PIN_RC_STEER, INPUT);
  pinMode(PIN_RC_THROT, INPUT);
  pinMode(PIN_RC_MODE, INPUT);

  // Attach Motors
  escLeft.attach(PIN_ESC_LEFT);
  escRight.attach(PIN_ESC_RIGHT);
  escBack.attach(PIN_ESC_BACK);
  myServo.attach(PIN_SERVO);

  stopAll();
  delay(3000); // Safety delay for ESC arming
}

void loop() {
  // 1. Read RC Signals
  // pulseIn waits for the signal (timeout 25000 micros = 25ms)
  steerVal = pulseIn(PIN_RC_STEER, HIGH, 25000);
  throtVal = pulseIn(PIN_RC_THROT, HIGH, 25000);
  modeVal = pulseIn(PIN_RC_MODE, HIGH, 25000);

  // 2. Determine Mode
  // Usually Switch High (>1500) means ON. Adjust based on your switch preference.
  if (modeVal > 1500) {
    isAutonomous = true;
    // Digital LED indication can be added here
  } else {
    isAutonomous = false;
  }

  // 3. Execution
  if (isAutonomous) {
    // --- AUTONOMOUS MODE ---
    // Listen to Jetson via Serial
    if (Serial.available() > 0) {
      char cmd = Serial.read();
      if (cmd == '\n' || cmd == '\r') return;
      
      switch (cmd) {
        case 'F': moveForward(1600); break; // Auto speed
        case 'B': moveBackward(1400); break;
        case 'S': stopAll(); break;
        case 'L': turnLeft(); break;
        case 'R': turnRight(); break;
        case 'C': centerSteering(); break;
      }
    }
  } else {
    // --- MANUAL RC MODE ---
    // Map RC Throttle (approx 1000-2000) to PWM
    // Safety check: if signal is 0 (receiver disconnected), stop.
    if (throtVal == 0) {
      stopAll();
    } else {
      processManualControl();
    }
  }
}

void processManualControl() {
  // Servo Mixing
  int servoAngle = map(steerVal, 1000, 2000, ANGLE_LEFT, ANGLE_RIGHT);
  // Clamp values
  servoAngle = constrain(servoAngle, min(ANGLE_LEFT, ANGLE_RIGHT), max(ANGLE_LEFT, ANGLE_RIGHT));
  myServo.write(servoAngle);

  // Motor Mixing
  // Deadzone check to prevent jitter around center
  if (abs(throtVal - 1500) < RC_DEADZONE) {
    stopMotors(); // Only stop motors, keep servo active
  } else {
    // Send raw PWM from stick directly to ESCs
    // You might need to calibrate ESCs or map this value
    escLeft.writeMicroseconds(throtVal);
    escRight.writeMicroseconds(throtVal);
    // escBack logic can be added here if needed
  }
}

// --- Helpers ---

void stopAll() {
  stopMotors();
  centerSteering();
}

void stopMotors() {
  escLeft.writeMicroseconds(PWM_CENTER);
  escRight.writeMicroseconds(PWM_CENTER);
  escBack.writeMicroseconds(PWM_CENTER);
}

void moveForward(int speed) {
  escLeft.writeMicroseconds(speed);
  escRight.writeMicroseconds(speed);
}

void moveBackward(int speed) {
  escLeft.writeMicroseconds(speed);
  escRight.writeMicroseconds(speed);
}

void turnLeft() { myServo.write(ANGLE_LEFT); }
void turnRight() { myServo.write(ANGLE_RIGHT); }
void centerSteering() { myServo.write(ANGLE_CENTER); }
