# Autonomous Boat Control System

This project implements a control system for an autonomous boat, bridging high-level logic (NVIDIA Jetson Orin) with low-level actuation (Arduino Nano).

## System Architecture
* **High Level**: NVIDIA Jetson Orin running Python (Path Planning, Computer Vision & Navigation Logic).
* **Low Level**: Arduino Nano running C++ (PWM generation for Motors & Servo).
* **Communication**: Serial via USB (UART) @ 115200 baud.

## Hardware Connection (Pinout)
Based on the current wiring diagram:
* **Left Motor (ESC)**: Pin `D9`
* **Right Motor (ESC)**: Pin `D10`
* **Back Motor (ESC)**: Pin `D8`
* **Steering Servo**: Pin `D3`

## Installation & Usage

### 1. Arduino (Firmware)
1.  Open `boat_firmware.ino` in Arduino IDE.
2.  Select your board (e.g., Arduino Nano / Nano 33 IoT).
3.  Upload the sketch to the board.

### 2. Jetson (Controller)
1.  Ensure Python 3 is installed.
2.  Install the required library:
    ```bash
    pip3 install pyserial
    ```
3.  Connect the Arduino via USB.
4.  Run the controller script:
    ```bash
    sudo python3 boat_controller.py
    ```

## Controls (Manual Mode)
Currently, the system supports keyboard commands via the terminal:
* `F`: Move Forward
* `B`: Move Backward
* `S`: Stop all motors (Emergency)
* `L`: Turn Left (Servo)
* `R`: Turn Right (Servo)
* `C`: Center Steering
