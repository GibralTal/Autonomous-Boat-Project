# Field Operations Manual

This document outlines the standard operating procedures (SOP) for deploying the autonomous surface vehicle (ASV) in a field environment. It covers pre-deployment checks, target calibration, and mission execution.

## 1. Pre-Deployment Checklist
Ensure all items are verified before transporting the system to the test site.

* **Power Systems:**
    * Motor LiPo batteries fully charged and voltage balanced.
    * Jetson power source (Power bank/Battery) fully charged.
    * Laptop battery charged (for field access).
* **Hardware Integrity:**
    * Propellers secured and free of debris.
    * Waterproof seals and hatches inspected.
    * USB connections (Arduino, GPS, Camera) secured.
* **Software:**
    * Latest code pulled from the repository.
    * Python dependencies installed (`pyserial`, `pynmea2`).

## 2. Phase 1: Target Acquisition
To navigate autonomously, the system requires precise GPS coordinates of the target destination (e.g., a buoy).

1.  **Positioning:** Physically move the vessel (or the GPS unit) to the desired target location.
2.  **Data Collection:**
    * Connect via SSH or terminal.
    * Execute the coordinate tool:
        ```bash
        sudo python3 gps_test.py
        ```
3.  **Logging:** Wait for a valid GPS lock (non-zero values). Record the `LATITUDE` and `LONGITUDE` values displayed on the console.

## 3. Phase 2: Mission Configuration
Update the navigation logic with the coordinates obtained in Phase 1.

1.  **Edit Configuration:**
    Open the main navigation script:
    ```bash
    nano simple_nav.py
    ```
2.  **Update Waypoints:**
    Locate the configuration section at the top of the file. Replace the existing values with the new coordinates:
    ```python
    # Mission Configuration
    TARGET_LAT = 32.123456
    TARGET_LON = 34.987654
    ```
3.  **Save:** Press `Ctrl+O`, `Enter`, then `Ctrl+X` to save and exit.

## 4. Phase 3: Mission Launch
**Safety Warning:** Ensure all personnel are clear of the propellers before arming the system.

1.  **Initialization:**
    * Place the boat in the water at the Start Point.
    * Connect the main LiPo battery to the ESCs.
    * Listen for the ESC initialization tone.
2.  **Execution:**
    Run the autonomous navigation mission:
    ```bash
    sudo python3 simple_nav.py
    ```
3.  **Monitoring:**
    Monitor the terminal output for telemetry:
    * Distance to target (meters).
    * Heading error and steering corrections.
    * System status.

## 5. Emergency Procedures
In the event of erratic behavior or immediate danger:

* **Software Abort:** Press `Ctrl+C` in the terminal to terminate the Python script immediately.
* **Manual Override (If RC connected):** Toggle the safety switch on the RC transmitter to disengage the autonomous controller.
* **Physical Recovery:** Use the safety tether to retrieve the vessel manually.
