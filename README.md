# Autonomous Boat Control System

This project implements a control and navigation system for an autonomous surface vessel. It uses a **Master-Slave architecture**:
* **Master (Jetson Orin):** Handles GPS navigation, path planning, and logic.
* **Slave (Arduino Nano 33 IoT):** Actuates motors and servos based on serial commands.

## System Architecture

### Hardware Flow
`[GPS Satellite]` -> `[USB GPS Receiver]` -> **Jetson Orin** -> `[USB Serial]` -> **Arduino Nano** -> `[ESCs & Servos]`

### Logic
* **High Level (Python):** Reads NMEA data from the GPS, calculates the heading error to the target waypoint, and sends steering commands (`L`, `R`, `C`) and throttle (`F`) to the Arduino.
* **Low Level (C++):** Maps received characters to PWM signals for the hardware.

## Hardware Connection (Pinout)

### Actuators (Connected to Arduino)
Based on `boat_firmware.ino` and circuit diagram:
* **Left Motor (ESC):** Pin `D9`
* **Right Motor (ESC):** Pin `D10`
* **Back Motor (ESC):** Pin `D8`
* **Steering Servo:** Pin `D3`

### Sensors
* **GPS Module:** U-blox USB GPS connected to Jetson USB port (mapped to `/dev/ttyACM1`).
* **Communication:** Arduino connected to Jetson USB port (mapped to `/dev/ttyACM0`).

## Software Prerequisites

### 1. Jetson Orin (Python Environment)
* Python 3.x
* Required libraries:
    ```bash
    pip install pyserial pynmea2
    ```

### 2. Arduino Nano (Firmware)
* **Board Manager:** Arduino Nano 33 IoT
* **Libraries:** `Servo.h` (Built-in)

## Installation & Setup

1.  **Flash the Firmware:**
    * Open `boat_firmware.ino` in Arduino IDE.
    * Connect the Arduino Nano via USB.
    * Select the correct board and port.
    * Upload the code.

2.  **Configure the Jetson:**
    * Clone this repository.
    * Verify USB permissions (you might need `sudo` or add your user to the `dialout` group).
    * Verify device paths:
        * Arduino is usually `/dev/ttyACM0`.
        * GPS is usually `/dev/ttyACM1`.
    * *Note: If ports differ, update `GPS_PORT` and `ARDUINO_PORT` variables in the Python scripts.*

## Usage

The system has two modes of operation, ran by separate scripts.

### Mode A: Manual Control (Keyboard)
Use this for testing motors and steering manually.
1.  Run the controller:
    ```bash
    sudo python3 boat_controller.py
    ```
2.  **Controls:**
    * `F` - Forward
    * `B` - Backward
    * `L` / `R` - Turn Left / Right
    * `C` - Center Steering
    * `S` - Stop
    * `Q` - Quit

### Mode B: Autonomous Navigation (GPS)
Use this for waypoint missions.
1.  **Set Target:** Open `simple_gps_nav.py` and edit the configuration section:
    ```python
    TARGET_LAT = 32.085300  # Your Target Latitude
    TARGET_LON = 34.781800  # Your Target Longitude
    ```
2.  **Launch Mission:**
    ```bash
    sudo python3 simple_gps_nav.py
    ```
3.  **Behavior:**
    * The script prints the distance and heading to the target.
    * It automatically steers (`L`/`R`/`C`) to face the target.
    * It stops (`S`) when within `DISTANCE_THRESHOLD` (5 meters).

## Safety
* **Always test props-off first.**
* The `S` command acts as an emergency stop in manual mode.
* In autonomous mode, use `Ctrl+C` to kill the script (Note: The Arduino handles the last command received, so ensure to kill power or send a stop signal if needed).
