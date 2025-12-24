import serial
import pynmea2
import time

# Port settings (Usually ttyACM1 if the Arduino is on ACM0)
GPS_PORT = '/dev/ttyACM0'
BAUD_RATE = 115200

def main():
    print(f"--- GPS Coordinate Finder ---")
    print(f"Connecting to {GPS_PORT}...")
    
    try:
        ser = serial.Serial(GPS_PORT, BAUD_RATE, timeout=1)
        print("Waiting for satellite lock (this may take time outdoors)...")
        
        while True:
            line = ser.readline().decode('utf-8', errors='ignore')
            print(f"RAW DATA: {line.strip()}")
            # Check for location messages (GNRMC or GNGGA)
            if line.startswith('$GNRMC') or line.startswith('$GNGGA'):
                try:
                    msg = pynmea2.parse(line)
                    # Print only if there is a valid GPS fix
                    if hasattr(msg, 'latitude') and float(msg.latitude) != 0.0:
                        print(f" CURRENT LOCATION:")
                        print(f"LAT: {msg.latitude}")
                        print(f"LON: {msg.longitude}")
                        print("-" * 20)
                except:
                    pass
                    
    except Exception as e:
        print(f"Error: {e}")
        print("Tip: Check if GPS is connected and port is correct.")

if __name__ == "__main__":
    main()
