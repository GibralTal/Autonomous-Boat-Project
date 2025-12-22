import serial
import pynmea2
import math
import time

# --- הגדרות חיבורים (Setup) ---
# וודא שהפורטים נכונים!
GPS_PORT = '/dev/ttyACM1'      # הפורט של ה-GPS
ARDUINO_PORT = '/dev/ttyACM0'  # הפורט של הארדואינו
BAUD_RATE = 115200             # מהירות תקשורת לארדואינו
GPS_BAUD = 9600                # מהירות תקשורת ל-GPS

# --- הגדרת המשימה (נקודת היעד) ---
# שנה את המספרים האלו לנקודה באגם אליה אתה רוצה להגיע!
TARGET_LAT = 32.085300  # דוגמה: קו רוחב
TARGET_LON = 34.781800  # דוגמה: קו אורך
DISTANCE_THRESHOLD = 5.0 # מרחק במטרים שבו הסירה עוצרת (הגיעה ליעד)

# --- משתנים גלובליים ---
current_lat = 0.0
current_lon = 0.0
current_heading = 0.0 # כיוון החרטום (מחושב ע"י תנועה)

def get_distance_metres(lat1, lon1, lat2, lon2):
    """ חישוב מרחק במטרים בין שתי נקודות (Haversine Formula) """
    R = 6371000 # רדיוס כדור הארץ במטרים
    phi1 = math.radians(lat1)
    phi2 = math.radians(lat2)
    delta_phi = math.radians(lat2 - lat1)
    delta_lambda = math.radians(lon2 - lon1)
    a = math.sin(delta_phi/2)**2 + math.cos(phi1)*math.cos(phi2) * math.sin(delta_lambda/2)**2
    c = 2 * math.atan2(math.sqrt(a), math.sqrt(1-a))
    return R * c

def get_bearing(lat1, lon1, lat2, lon2):
    """ חישוב הזווית (Azimuth) ליעד """
    y = math.sin(math.radians(lon2-lon1)) * math.cos(math.radians(lat2))
    x = math.cos(math.radians(lat1))*math.sin(math.radians(lat2)) - \
        math.sin(math.radians(lat1))*math.cos(math.radians(lat2))*math.cos(math.radians(lon2-lon1))
    bearing = math.degrees(math.atan2(y, x))
    return (bearing + 360) % 360

def main():
    global current_lat, current_lon, current_heading
    
    print("--- מתחיל מערכת ניווט ---")
    
    # חיבור לרכיבים
    try:
        ser_gps = serial.Serial(GPS_PORT, GPS_BAUD, timeout=1)
        ser_arduino = serial.Serial(ARDUINO_PORT, BAUD_RATE, timeout=1)
        print("הרכיבים מחוברים בהצלחה.")
    except Exception as e:
        print(f"שגיאת חיבור: {e}")
        return

    # המתנה קצרה לאתחול
    time.sleep(2)

    while True:
        try:
            # 1. קריאה מה-GPS
            line = ser_gps.readline().decode('utf-8', errors='ignore')
            
            # אנו מחפשים הודעות מסוג RMC או GGA שמכילות מיקום
            if line.startswith('$GNRMC') or line.startswith('$GPRMC'):
                msg = pynmea2.parse(line)
                
                # עדכון מיקום רק אם ה-GPS נעול
                if msg.status == 'A': # A = Data Valid
                    current_lat = float(msg.latitude)
                    current_lon = float(msg.longitude)
                    
                    # ה-GPS נותן גם כיוון תנועה (Course Over Ground)
                    # זה יעזור לנו לדעת לאן הסירה פונה כרגע
                    if hasattr(msg, 'true_course'):
                         # ממיר ל-float בטוח
                        try:
                            current_heading = float(msg.true_course)
                        except:
                            pass 

                    # 2. חישובים לניווט
                    dist_to_target = get_distance_metres(current_lat, current_lon, TARGET_LAT, TARGET_LON)
                    target_bearing = get_bearing(current_lat, current_lon, TARGET_LAT, TARGET_LON)
                    
                    # חישוב השגיאה: כמה מעלות אני צריך לתקן?
                    heading_error = target_bearing - current_heading
                    # נרמול השגיאה לטווח של -180 עד 180
                    if heading_error > 180: heading_error -= 360
                    if heading_error < -180: heading_error += 360

                    print(f"מרחק: {dist_to_target:.1f}m | כיוון נוכחי: {current_heading:.0f} | יעד: {target_bearing:.0f}")

                    # 3. קבלת החלטות ושליחה לארדואינו
                    if dist_to_target < DISTANCE_THRESHOLD:
                        print("הגענו ליעד! עוצר מנועים.")
                        ser_arduino.write(b'S') # Stop
                        break
                    
                    else:
                        # לוגיקה פשוטה להיגוי
                        if heading_error > 20: 
                            # היעד מימין -> פנה ימינה
                            print("פונה ימינה >>>")
                            ser_arduino.write(b'R') 
                        elif heading_error < -20:
                            # היעד משמאל -> פנה שמאלה
                            print("<<< פונה שמאלה")
                            ser_arduino.write(b'L')
                        else:
                            # סע ישר
                            print("^^^ נוסע ישר")
                            ser_arduino.write(b'C')
                        
                        # הוספת גז (נסיעה קדימה) בנפרד (כדי שהסירה תתקדם תוך כדי היגוי)
                        # הערה: זה דורש שקוד הארדואינו ידע לקבל רצף פקודות, או פשוט לשלוח F מדי פעם
                        ser_arduino.write(b'F')

        except Exception as e:
            print(f"Error in loop: {e}")

if __name__ == "__main__":
    main()
