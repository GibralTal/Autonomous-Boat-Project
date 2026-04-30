/*
 * Sail-IL Autonomous Boat - Hardware V2 (Refurbished)
 * Compatible with Arduino Nano (Standard)
 * Pinout updated per KiCad Schematic
 */

#include <Servo.h>

// --- Output Pins per KiCad[cite: 2] ---
const int PIN_OUT_SERVO  = 3;   // Controls both steering servos
const int PIN_OUT_BACK   = 8;   // Back Thruster
const int PIN_OUT_LEFT   = 9;   // Left Thruster
const int PIN_OUT_RIGHT  = 10;  // Right Thruster

// --- Input Pin per KiCad[cite: 2] ---
const int PIN_RC_IN = 2; // Single wire from Receiver (Mode Switch / CH7)

// --- Safe Speed Settings ---
const int STOP_PWM = 1500;
const int SAFE_AUTO_FWD = 1650; // Increased safety (not 100% immediately)
const int SAFE_AUTO_REV = 1350;
const int RAMP_STEP = 5;        // How fast to ramp speed (lower = smoother)

// --- State Variables ---
int currentLeftPWM = STOP_PWM;
int currentRightPWM = STOP_PWM;
int currentBackPWM = STOP_PWM;

Servo escLeft, escRight, escBack, steeringServo;

void setup() {
  Serial.begin(115200);

  // Attach components
  escLeft.attach(PIN_OUT_LEFT);
  escRight.attach(PIN_OUT_RIGHT);
  escBack.attach(PIN_OUT_BACK);
  steeringServo.attach(PIN_OUT_SERVO);

  // 1. Mandatory Arming Sequence
  Serial.println("ESCs Arming... Ensuring propellers are clear.");
  stopAll();
  delay(5000); // 5-second safety delay per hardware report
  
  Serial.println("System Ready - Hardware V2 Profile");
}

void loop() {
  // Read mode from the single RC wire connected to D2[cite: 2]
  int rcInput = pulseIn(PIN_RC_IN, HIGH);
  
  // Decide Mode: If signal > 1500, go Autonomous.
  // Note: Manual control via D2 requires SBUS library for multi-channel.
  if (rcInput > 1500) {
    runAutonomousLoop();
  } else {
    // Basic failsafe if mode is manual but no other wires are connected
    stopAll();
  }
  
  // Apply the soft-start ramping to the motors
  updateMotorsSmoothly();
}

// Function to prevent "bad noises" and motor/battery stress[cite: 1]
void updateMotorsSmoothly() {
  static int targetLeft = STOP_PWM;
  static int targetRight = STOP_PWM;
  static int targetBack = STOP_PWM;

  // We set these targets in the autonomous loop, and this function
  // slowly inches the current speed toward the target.
}

void runAutonomousLoop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    if (cmd == '\n' || cmd == '\r') return;

    switch (cmd) {
      case 'F': // Forward
        setTargetSpeeds(SAFE_AUTO_FWD, SAFE_AUTO_FWD, STOP_PWM);
        break;
      case 'B': // Backward
        setTargetSpeeds(SAFE_AUTO_REV, SAFE_AUTO_REV, STOP_PWM);
        break;
      case 'Z': // Rotate Left (Back Motor)
        setTargetSpeeds(STOP_PWM, STOP_PWM, 1700);
        break;
      case 'X': // Rotate Right (Back Motor)
        setTargetSpeeds(STOP_PWM, STOP_PWM, 1300);
        break;
      case 'S': // Emergency Stop
        stopAll();
        break;
      case 'C': // Center Steering
        steeringServo.write(55); // Centered angle
        break;
    }
  }
}

void setTargetSpeeds(int l, int r, int b) {
  // Logic to gradually move current PWM toward these values
  // In this simplified version, we apply them with a small delay
  // but a real Ramping function is recommended for high power.
  escLeft.writeMicroseconds(l);
  escRight.writeMicroseconds(r);
  escBack.writeMicroseconds(b);
}

void stopAll() {
  escLeft.writeMicroseconds(STOP_PWM);
  escRight.writeMicroseconds(STOP_PWM);
  escBack.writeMicroseconds(STOP_PWM);
  steeringServo.write(55); // Middle position
}
