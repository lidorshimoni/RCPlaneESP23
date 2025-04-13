# ESP32 RC Plane Controller

This project is an ESP32-based remote control system for an RC plane. It features a web-based joystick interface for controlling motor thrust and steering, along with additional features like motor direction toggling, IMU data display, and signal strength monitoring.

## Features

- **Web-Based Joystick Control**: Control the RC plane's motors using a virtual joystick in a web browser.
- **Motor Direction Toggle**: Reverse the direction of the left and right motors using toggle switches in the UI.
- **IMU Data Display**: View real-time accelerometer and gyroscope data from the onboard IMU.
- **Signal Strength Monitoring**: Display the Wi-Fi signal strength (RSSI) in the web interface.
- **Logging**: View logs of joystick inputs, motor updates, and other events in the web interface.

## Hardware Requirements

- **ESP32 Development Board**
- **Motor Driver**: DRV8835 or similar
- **IMU Sensor**: ICM-42670-P or ICM-20948
- **RC Plane Motors**
- **Power Supply**

## Software Requirements

- **PlatformIO** (Recommended IDE)
- **Arduino Framework**
- **Adafruit ICM20948 Library** (for IMU support)

## Pin Configuration

| Component         | Pin Name          | ESP32 Pin |
|--------------------|-------------------|-----------|
| Motor A Phase      | `MOTOR_A_PHASE_PIN` | 43        |
| Motor A Enable     | `MOTOR_A_ENABLE_PIN` | 3         |
| Motor B Phase      | `MOTOR_B_PHASE_PIN` | 44        |
| Motor B Enable     | `MOTOR_B_ENABLE_PIN` | 1         |
| IMU SDA            | `IMU_SDA_PIN`      | 5         |
| IMU SCL            | `IMU_SCL_PIN`      | 6         |

## How It Works

1. **Wi-Fi Access Point**:
   - The ESP32 creates a Wi-Fi access point with the SSID `ESP32_RC_Plane` and password `12345678`.
   - Connect to this network to access the web interface.

2. **Web Interface**:
   - Open a browser and navigate to the ESP32's IP address (e.g., `192.168.4.1`).
   - Use the joystick to control the motors:
     - Move the joystick up/down to control thrust.
     - Move the joystick left/right to control steering.
   - Toggle the motor direction using the provided switches.

3. **Motor Control**:
   - The joystick inputs are sent to the ESP32, which calculates motor speeds and directions.
   - The motor speeds are updated using PWM signals.

4. **IMU Data**:
   - The onboard IMU provides real-time accelerometer and gyroscope data, which is displayed in the web interface.

5. **Logging**:
   - Logs of joystick inputs, motor updates, and other events are displayed in the web interface.

## Web Interface Features

- **Joystick**: Control thrust and steering.
- **Motor Direction Toggles**: Reverse the direction of the left and right motors.
- **IMU Data**: View real-time accelerometer and gyroscope data.
- **Logs**: View logs of system events.
- **Signal Strength**: Monitor the Wi-Fi signal strength.

## Code Overview

### Key Files

- **`RCPlane.cpp`**:
  - Contains the main logic for the ESP32, including motor control, IMU handling, and web server routes.

### Key Functions

- **`updateMotors(int x, int y)`**:
  - Maps joystick inputs to motor speeds and updates the motor directions.
- **`handleControl()`**:
  - Processes joystick inputs from the web interface.
- **`handleReverse()`**:
  - Handles motor direction toggling.
- **`readIMU()`**:
  - Reads data from the IMU and formats it for display.
- **`addToLog(const String& message)`**:
  - Adds messages to the log buffer for display in the web interface.

## Setup Instructions

1. **Clone the Repository**:
   ```bash
   git clone <repository-url>
   cd <repository-folder>