"""
Boat Autonomous System - High Level Control
Platform: NVIDIA Jetson Orin
Description: Sends commands to Arduino via USB Serial.
"""

import serial
import time
import sys

# Serial Configuration
# Note: Use 'ls /dev/tty*' to find your correct port (ACM0 or USB0)
ARDUINO_PORT = '/dev/ttyACM0' 
BAUD_RATE = 115200

def main():
    print(f"Connecting to Arduino on {ARDUINO_PORT}...")

    try:
        # Initialize Serial Connection
        ser = serial.Serial(ARDUINO_PORT, BAUD_RATE, timeout=1)
        time.sleep(2) # Wait for Arduino restart
        
        print("\n--- BOAT CONTROL SYSTEM READY ---")
        print("Controls: F=Forward, B=Back, S=Stop")
        print("Steering: L=Left, R=Right, C=Center")
        print("Q=Quit")
        print("---------------------------------")

        while True:
            # Get user command
            command = input("Command > ").upper()
            
            if command == 'Q':
                print("Stopping system...")
                ser.write(b'S') # Ensure motors stop before quitting
                break
                
            elif command in ['F', 'B', 'S', 'L', 'R', 'C']:
                # Send command to Arduino
                ser.write(command.encode())
                print(f"Sent: {command}")
            else:
                print("Invalid Command. Use: F, B, S, L, R, C")

    except serial.SerialException as e:
        print(f"Error opening serial port: {e}")
        print("Check if Arduino is connected and permissions (sudo).")
        
    except KeyboardInterrupt:
        print("\nForce Exit detected.")
        
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()
            print("Serial connection closed.")

if __name__ == "__main__":
    main()
