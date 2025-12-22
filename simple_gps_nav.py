import serial
import pynmea2
import math
import time

# --- Connection Setup ---
# Ensure ports are correct!
GPS_PORT = '/dev/ttyACM1'      # GPS Serial Port
ARDUINO_PORT = '/dev/ttyACM0'  # Arduino Serial Port
BAUD_RATE = 115200             # Arduino Baud Rate
GPS_BAUD = 9600                # GPS Baud Rate

# --- Mission Configuration (Target Waypoint) ---
# Update these coordinates to your desired target location.
TARGET_LAT = 32.085300  # Example: Latitude
TARGET_LON = 34.781800  # Example: Longitude
DISTANCE_THRESHOLD = 5.0 # Stop motors when within this distance (meters)

# --- Global State ---
current_lat = 0.0
current_lon = 0.0
current_heading = 0.0 # Course Over Ground (Derived from GPS movement)

def get_distance_metres(lat1, lon1, lat2, lon2):
    """ Calculate distance in meters between two points (Haversine Formula) """
    R = 6371000 # Earth radius in meters
    phi1 = math.radians(lat1)
    phi2 = math.radians(lat2)
    delta_phi = math.radians(lat2 - lat1)
    delta_lambda = math.radians(lon2 - lon1)
    a = math.sin(delta_phi/2)**2 + math.cos(phi1)*math.cos(phi2) * math.sin(delta_lambda/2)**2
    c = 2 * math.atan2(math.sqrt(a), math.sqrt(1-a))
    return R * c

def get_bearing(lat1, lon1, lat2, lon2):
    """ Calculate the bearing (Azimuth) to the target """
    y = math.sin(math.radians(lon2-lon1)) * math.cos(math.radians(lat2))
    x = math.cos(math.radians(lat1))*math.sin(math.radians(lat2)) - \
        math.sin(math.radians(lat1))*math.cos(math.radians(lat2))*math.cos(math.radians(lon2-lon1))
    bearing = math.degrees(math.atan2(y, x))
    return (bearing + 360) % 360

def main():
    global current_lat, current_lon, current_heading
    
    print("--- Starting Navigation System ---")
    
    # Initialize Component Connections
    try:
        ser_gps = serial.Serial(GPS_PORT, GPS_BAUD, timeout=1)
        ser_arduino = serial.Serial(ARDUINO_PORT, BAUD_RATE, timeout=1)
        print("Components connected successfully.")
    except Exception as e:
        print(f"Connection Error: {e}")
        return

    # Wait for initialization
    time.sleep(2)

    while True:
        try:
            # 1. Read from GPS
            line = ser_gps.readline().decode('utf-8', errors='ignore')
            
            # Look for RMC or GGA messages containing location data
            if line.startswith('$GNRMC') or line.startswith('$GPRMC'):
                msg = pynmea2.parse(line)
                
                # Update location only if GPS has a fix (Status 'A')
                if msg.status == 'A': 
                    current_lat = float(msg.latitude)
                    current_lon = float(msg.longitude)
                    
                    # GPS provides Course Over Ground (COG).
                    # This helps determine the boat's current heading.
                    if hasattr(msg, 'true_course'):
                        try:
                            current_heading = float(msg.true_course)
                        except:
                            pass 

                    # 2. Navigation Calculations
                    dist_to_target = get_distance_metres(current_lat, current_lon, TARGET_LAT, TARGET_LON)
                    target_bearing = get_bearing(current_lat, current_lon, TARGET_LAT, TARGET_LON)
                    
                    # Calculate Heading Error
                    heading_error = target_bearing - current_heading
                    # Normalize error to -180 to 180 range
                    if heading_error > 180: heading_error -= 360
                    if heading_error < -180: heading_error += 360

                    print(f"Dist: {dist_to_target:.1f}m | Heading: {current_heading:.0f} | Target: {target_bearing:.0f}")

                    # 3. Decision Making & Actuation
                    if dist_to_target < DISTANCE_THRESHOLD:
                        print("Target reached! Stopping motors.")
                        ser_arduino.write(b'S') # Stop
                        break
                    
                    else:
                        # Simple Steering Logic
                        if heading_error > 20: 
                            # Target is to the right -> Turn Right
                            print("Turning Right >>>")
                            ser_arduino.write(b'R') 
                        elif heading_error < -20:
                            # Target is to the left -> Turn Left
                            print("<<< Turning Left")
                            ser_arduino.write(b'L')
                        else:
                            # Maintain Course
                            print("^^^ Going Straight")
                            ser_arduino.write(b'C')
                        
                        # Send Forward command to maintain throttle during steering
                        ser_arduino.write(b'F')

        except Exception as e:
            print(f"Error in loop: {e}")

if __name__ == "__main__":
    main()
