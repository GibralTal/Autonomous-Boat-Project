/*
 * Boat Autonomous System - Low Level Control
 * Hardware: Arduino Nano / Arduino Nano 33 IoT
 * Description: Controls 3 ESCs (Brushless Motors) and 1 Servo based on Serial commands.
 */

#include <Servo.h>

// --- Pin Definitions ---
const int PIN_ESC_LEFT = 9;
const int PIN_ESC_RIGHT = 10;
const int PIN_ESC_BACK = 8;
const int PIN_SERVO = 3;

// --- Servo Objects ---
Servo escLeft;
Servo escRight;
Servo escBack;
Servo myServo;

// --- PWM Settings ---
const int STOP_PWM = 1500;      // Mid-point (Neutral)
const int FORWARD_PWM = 1600;   // Forward Speed
const int BACKWARD_PWM = 1400;  // Backward Speed

// --- Steering Angles ---
// Adjust these if servo direction is inverted
const int ANGLE_CENTER = 90;
const int ANGLE_LEFT = 60;
const int ANGLE_RIGHT = 120;

void setup() {
  Serial.begin(115200); // Communication with Jetson Orin

  // Attach Motors & Servo
  escLeft.attach(PIN_ESC_LEFT);
  escRight.attach(PIN_ESC_RIGHT);
  escBack.attach(PIN_ESC_BACK);
  myServo.attach(PIN_SERVO);

  // Arming Sequence (Safety)
  stopAll();
  delay(3000); // Wait for ESCs to initialize
}

void loop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    
    // Ignore newline characters
    if (cmd == '\n' || cmd == '\r') return;

    switch (cmd) {
      case 'F': // Forward
        moveForward();
        break;
      case 'S': // Stop
        stopAll();
        break;
      case 'B': // Backward
        moveBackward();
        break;
      case 'L': // Turn Left
        turnLeft();
        break;
      case 'R': // Turn Right
        turnRight();
        break;
      case 'C': // Center Steering
        centerSteering();
        break;
    }
  }
}

// --- Control Functions ---

void stopAll() {
  escLeft.writeMicroseconds(STOP_PWM);
  escRight.writeMicroseconds(STOP_PWM);
  escBack.writeMicroseconds(STOP_PWM);
}

void moveForward() {
  escLeft.writeMicroseconds(FORWARD_PWM);
  escRight.writeMicroseconds(FORWARD_PWM);
}

void moveBackward() {
  escLeft.writeMicroseconds(BACKWARD_PWM);
  escRight.writeMicroseconds(BACKWARD_PWM);
}

void turnLeft() {
  myServo.write(ANGLE_LEFT);
}

void turnRight() {
  myServo.write(ANGLE_RIGHT);
}

void centerSteering() {
  myServo.write(ANGLE_CENTER);
}
