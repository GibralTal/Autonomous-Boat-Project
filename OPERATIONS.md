# Field Operations Manual

This document outlines the standard operating procedures (SOP) for deploying the autonomous surface vehicle (ASV) using the hybrid RC/Jetson control system.

## 1. Pre-Deployment Checklist
Ensure all items are verified before transporting the system to the test site.

* **Power Systems:**
    * Motor LiPo batteries fully charged.
    * Jetson power source fully charged.
    * **RC Transmitter batteries** charged/replaced.
* **Hardware Integrity:**
    * Propellers secured.
    * Waterproof seals inspected.
    * USB connections (Arduino, GPS) secured.
* **Control Check:**
    * Verify RC Receiver LED is **RED** (PWM Mode).
    * Verify Arduino takes manual stick commands when Switch is LOW.

## 2. Phase 1: Target Acquisition
To navigate autonomously, the system requires precise GPS coordinates.

1.  **Positioning:** Move the vessel to the desired target location.
2.  **Data Collection:**
    * Execute the GPS tool on the Jetson:
        ```bash
        sudo python3 gps_test.py
        ```
3.  **Logging:** Record the `LATITUDE` and `LONGITUDE`.

## 3. Phase 2: Mission Configuration
Update the navigation logic with the coordinates obtained.

1.  **Edit Configuration:**
    ```bash
    nano simple_gps_nav.py
    ```
2.  **Update Waypoints:**
    Replace existing coordinates:
    ```python
    TARGET_LAT = 32.123456
    TARGET_LON = 34.987654
    ```
3.  **Save:** `Ctrl+O`, `Enter`, `Ctrl+X`.

## 4. Phase 3: Mission Launch
**Safety Warning:** Keep hands clear of propellers.

1.  **Initialization:**
    * Turn on the **RC Transmitter**.
    * Set **Switch (CH5)** to **LOW (Manual Mode)**.
    * Connect the main LiPo battery to the boat.
    * Verify manual control (Left Stick moves motors/servo).
2.  **Arming:**
    * Place the boat in the water.
    * Run the Python script on the Jetson:
        ```bash
        sudo python3 simple_gps_nav.py
        ```
3.  **Engagement:**
    * Toggle **Switch (CH5)** to **HIGH (Autonomous Mode)**.
    * The boat should now respond to the Python script commands.

## 5. Emergency Procedures
In the event of erratic behavior or collision risk:

* **IMMEDIATE MANUAL OVERRIDE:**
    Toggle the RC Switch (CH5) to **LOW**. This instantly cuts off the Jetson's control and returns full authority to the pilot sticks.
* **Software Abort:**
    If connected via terminal, press `Ctrl+C`.
* **Physical Recovery:**
    Use manual control to bring the boat back to shore.
