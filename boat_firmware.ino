/*
 * Boat Control System V5.0 - RadioLink Manual Implementation
 * BASED ON RADIOLINK T8FB MANUAL
 * * IMPORTANT: Ensure Receiver R8EF LED is RED (PWM Mode).
 * Press binding button twice quickly to switch modes if Blue.
 * * WIRING:
 * Receiver CH1 (Right Stick Horizontal) -> Arduino D11
 * Receiver CH2 (Right Stick Vertical)   -> Arduino D12
 * Receiver CH4 (Left Stick Horizontal)  -> Arduino D6
 * Receiver CH5 (Switch)                 -> Arduino D7
 */

#include <Servo.h>

// --- כניסות (INPUTS) ---
const int PIN_RX_SERVO  = 11; // CH1 - Aileron
const int PIN_RX_MAIN   = 12; // CH2 - Elevator (Right Stick Up/Down)
const int PIN_RX_MIDDLE = 6;  // CH4 - Rudder (Left Stick Left/Right)
const int PIN_RX_MODE   = 7;  // CH5 - Switch

// --- יציאות (OUTPUTS) ---
const int PIN_ESC_LEFT   = 9;
const int PIN_ESC_RIGHT  = 10;
const int PIN_ESC_MIDDLE = 8; 
const int PIN_SERVO      = 3;

// --- גבולות וכיול ---
const int PWM_CENTER   = 1500;
const int PWM_DEADZONE = 50;   // התעלמות מרעידות במרכז הסטיק
const int SERVO_LEFT   = 60;   // זווית מקסימלית שמאלה
const int SERVO_RIGHT  = 120;  // זווית מקסימלית ימינה

Servo escLeft;
Servo escRight;
Servo escMiddle;
Servo myServo;

// משתנים לשמירת ערכי השלט
int valServo  = 0;
int valMain   = 0;
int valMiddle = 0;
int valMode   = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("--- Boat V5.0 (RadioLink Mapped) ---");
  Serial.println("Verify R8EF LED is RED!");

  pinMode(PIN_RX_SERVO, INPUT);
  pinMode(PIN_RX_MAIN, INPUT);
  pinMode(PIN_RX_MIDDLE, INPUT);
  pinMode(PIN_RX_MODE, INPUT);

  escLeft.attach(PIN_ESC_LEFT);
  escRight.attach(PIN_ESC_RIGHT);
  escMiddle.attach(PIN_ESC_MIDDLE);
  myServo.attach(PIN_SERVO);

  stopAll();
  delay(1000);
}

void loop() {
  // 1. קריאת ערוצים
  valServo  = pulseIn(PIN_RX_SERVO, HIGH, 20000);
  valMain   = pulseIn(PIN_RX_MAIN, HIGH, 20000);
  valMiddle = pulseIn(PIN_RX_MIDDLE, HIGH, 20000);
  valMode   = pulseIn(PIN_RX_MODE, HIGH, 20000);

  // 2. דיאגנוסטיקה (הדפסה למסך לבדיקה)
  // תפתח Serial Monitor ותראה אם המספרים משתנים נכון
  static int c = 0;
  if (c++ > 10) {
    Serial.print("MODE: "); Serial.print(valMode > 1500 ? "AUTO" : "MANUAL");
    Serial.print(" | MAIN(R-Vert): "); Serial.print(valMain);
    Serial.print(" | MID(L-Horz): "); Serial.print(valMiddle);
    Serial.print(" | SERVO(R-Horz): "); Serial.println(valServo);
    c = 0;
  }

  // 3. החלטה ובקרה
  if (valMode > 1500) {
    runAutonomousLoop();
  } else {
    runManualLoop();
  }
  delay(10);
}

void runManualLoop() {
  // הגנה: אם השלט כבוי (אין קריאות), עצור הכל
  if (valMain < 800 && valMiddle < 800) {
    stopAll();
    return;
  }

  // --- שליטה במנועים ראשיים (סטיק ימני אנכי - CH2) ---
  int mainSpeed = valMain;
  if (abs(mainSpeed - 1500) < PWM_DEADZONE) mainSpeed = 1500;
  
  // אם הסטיק עובד הפוך (למעלה זה אחורה), תוריד את ההערה מהשורה הבאה:
  // mainSpeed = map(valMain, 1000, 2000, 2000, 1000); 

  escLeft.writeMicroseconds(mainSpeed);
  escRight.writeMicroseconds(mainSpeed);

  // --- שליטה בסרובו (סטיק ימני אופקי - CH1) ---
  int servoAng = map(valServo, 1000, 2000, SERVO_LEFT, SERVO_RIGHT);
  myServo.write(servoAng);

  // --- שליטה במנוע אמצעי (סטיק שמאלי אופקי - CH4) ---
  // ביקשת: שמאלה -> מנוע FORWARD
  // הערה: בדרך כלל שמאלה = 1000, ימינה = 2000.
  // מנוע: Forward = 1600-2000.
  // לכן צריך מיפוי "הפוך": ערך נמוך בשלט -> ערך גבוה במנוע.
  
  int midSpeed = 1500;
  
  if (valMiddle < (1500 - PWM_DEADZONE)) {
     // סטיק שמאלה (ערך נמוך) -> סע קדימה
     // ממירים את הטווח 1000-1450 לטווח 1550-2000
     midSpeed = map(valMiddle, 1500, 1000, 1500, 1900); 
  } 
  else if (valMiddle > (1500 + PWM_DEADZONE)) {
     // סטיק ימינה (ערך גבוה) -> סע אחורה
     // ממירים את הטווח 1550-2000 לטווח 1450-1100
     midSpeed = map(valMiddle, 1500, 2000, 1500, 1100);
  }

  escMiddle.writeMicroseconds(midSpeed);
}

void runAutonomousLoop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    if (cmd == '\n' || cmd == '\r') return;

    switch (cmd) {
      case 'F': // Forward
        escLeft.writeMicroseconds(1600);
        escRight.writeMicroseconds(1600);
        break;
      case 'B': // Back
        escLeft.writeMicroseconds(1400);
        escRight.writeMicroseconds(1400);
        break;
      case 'S': // Stop
        stopAll();
        break;
      case 'L': // Left
        myServo.write(SERVO_LEFT);
        // אופציה: הפעלת מנוע אמצעי לעזרה בסיבוב
        // escMiddle.writeMicroseconds(1700); 
        break;
      case 'R': // Right
        myServo.write(SERVO_RIGHT);
        break;
      case 'C': // Center
        myServo.write(90);
        escMiddle.writeMicroseconds(1500);
        break;
    }
  }
}

void stopAll() {
  escLeft.writeMicroseconds(1500);
  escRight.writeMicroseconds(1500);
  escMiddle.writeMicroseconds(1500);
  myServo.write(90);
}
