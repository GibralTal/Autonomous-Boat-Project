/*
 * Boat Control System - FULL SPEED Autonomous (V22)
 * Hardware: Arduino Nano 33 IoT
 * Receiver: RadioLink R8EF (Red LED / PWM Mode)
 *
 * Update: Autonomous speeds set to MAX POWER.
 */

#include <Servo.h>

// --- Input Pins ---
const int PIN_IN_SERVO  = A0; 
const int PIN_IN_MAIN   = A1; 
const int PIN_IN_MIDDLE = A2; 
const int PIN_IN_MODE   = A3; 

// --- Output Pins ---
const int PIN_OUT_SERVO      = 3;
const int PIN_OUT_MIDDLE     = 8;
const int PIN_OUT_LEFT       = 9;
const int PIN_OUT_RIGHT      = 10;

// ==========================================
// --- SPEED SETTINGS (MAX POWER EDITION) ---
// ==========================================

// 1. AUTONOMOUS SPEEDS (Set to 100% Power)
// Previously these were 1350/1650 (Low Speed).
// Now set to limits (1000/2000) for FULL SPEED.
const int AUTO_SPEED_FWD  = 1000;  // Max Forward 'F'
const int AUTO_SPEED_REV  = 2000;  // Max Backward 'B'

// 2. AUTONOMOUS ROTATION SPEEDS
const int AUTO_ROT_LEFT_PWM  = 2000; // Max Power Rotate Left 'Z'
const int AUTO_ROT_RIGHT_PWM = 1000; // Max Power Rotate Right 'X'

// 3. MANUAL SPEED LIMIT
const int MANUAL_LIMIT_PCT = 100; // 100% Power from stick

// 4. SERVO ANGLES
const int ANGLE_TURN_LEFT  = 0;   
const int ANGLE_TURN_RIGHT = 135;  
const int ANGLE_CENTER     = 55;   

// ==========================================

// --- CALIBRATION ---
const int DEADZONE = 50;       
const int STICK_MIN = 1100;
const int STICK_MAX = 1900;

Servo escLeft;
Servo escRight;
Servo escMiddle;
Servo myServo;

void setup() {
  Serial.begin(115200);

  pinMode(PIN_IN_SERVO, INPUT);
  pinMode(PIN_IN_MAIN, INPUT);
  pinMode(PIN_IN_MIDDLE, INPUT);
  pinMode(PIN_IN_MODE, INPUT);

  escLeft.attach(PIN_OUT_LEFT);
  escRight.attach(PIN_OUT_RIGHT);
  escMiddle.attach(PIN_OUT_MIDDLE);
  myServo.attach(PIN_OUT_SERVO);

  stopAll();
  Serial.println("System Ready - V22 (FULL SPEED AUTONOMOUS)");
}

void loop() {
  int chServo  = pulseIn(PIN_IN_SERVO, HIGH);
  int chMain   = pulseIn(PIN_IN_MAIN, HIGH);
  int chMiddle = pulseIn(PIN_IN_MIDDLE, HIGH);
  int chMode   = pulseIn(PIN_IN_MODE, HIGH);

  if (chServo == 0 && chMain == 0) {
    stopAll();
    return;
  }

  // --- DEBUG TOOL ---
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 200) {
    Serial.print("Mode: "); Serial.print(chMode > 1500 ? "AUTO" : "MANUAL");
    Serial.print(" | Mid Stick: "); Serial.println(chMiddle);
    lastPrint = millis();
  }

  if (chMode > 1500) {
    runAutonomousLoop();
  } else {
    runManualLoop(chServo, chMain, chMiddle);
  }
}

void runManualLoop(int inServo, int inMain, int inMiddle) {
  
  // 1. Main Motors
  int mainPWM = inMain;
  
  if (abs(mainPWM - 1500) < DEADZONE) {
    mainPWM = 1500;
  } else {
    if (mainPWM > 1500) {
       mainPWM = map(mainPWM, 1500, 2000, 1500, 1500 + (500 * MANUAL_LIMIT_PCT / 100));
    } else {
       mainPWM = map(mainPWM, 1500, 1000, 1500, 1500 - (500 * MANUAL_LIMIT_PCT / 100));
    }
  }

  escLeft.writeMicroseconds(mainPWM);
  escRight.writeMicroseconds(mainPWM);

  // 2. Servo / Steering
  int cleanInput = constrain(inServo, STICK_MIN, STICK_MAX);
  int servoAngle = map(cleanInput, STICK_MIN, STICK_MAX, ANGLE_TURN_LEFT, ANGLE_TURN_RIGHT);
  myServo.write(servoAngle);

  // 3. Middle Motor (Manual)
  int midPWM = 1500;
  if (inMiddle > (1500 + DEADZONE)) { 
    midPWM = map(inMiddle, 1500, 2000, 1500, 1500 - (400 * MANUAL_LIMIT_PCT / 100)); 
  } 
  else if (inMiddle < (1500 - DEADZONE)) {
    midPWM = map(inMiddle, 1500, 1000, 1500, 1500 + (400 * MANUAL_LIMIT_PCT / 100)); 
  }
  escMiddle.writeMicroseconds(midPWM);
}

void runAutonomousLoop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    if (cmd == '\n' || cmd == '\r') return;

    switch (cmd) {
      case 'F': // Forward
        escLeft.writeMicroseconds(AUTO_SPEED_FWD); 
        escRight.writeMicroseconds(AUTO_SPEED_FWD); 
        break;
      
      case 'B': // Backward
        escLeft.writeMicroseconds(AUTO_SPEED_REV); 
        escRight.writeMicroseconds(AUTO_SPEED_REV); 
        break;
      
      case 'L': // Rudder Left
        myServo.write(ANGLE_TURN_LEFT); 
        break;
      
      case 'R': // Rudder Right
        myServo.write(ANGLE_TURN_RIGHT); 
        break;
      
      case 'Z': // Rotate LEFT (Middle Motor)
        escMiddle.writeMicroseconds(AUTO_ROT_LEFT_PWM); 
        break;

      case 'X': // Rotate RIGHT (Middle Motor)
        escMiddle.writeMicroseconds(AUTO_ROT_RIGHT_PWM); 
        break;
      
      case 'S': // Stop
        stopAll(); 
        break;
      
      case 'C': // Center
        myServo.write(ANGLE_CENTER); 
        escMiddle.writeMicroseconds(1500); 
        break;
    }
  }
}

void stopAll() {
  escLeft.writeMicroseconds(1500);
  escRight.writeMicroseconds(1500);
  escMiddle.writeMicroseconds(1500);
  myServo.write(ANGLE_CENTER);
}
