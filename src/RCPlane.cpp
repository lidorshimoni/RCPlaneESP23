#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_ICM20948.h> // Example IMU library

const char* ssid = "ESP32_RC_Plane";
const char* password = "12345678";

WebServer server(80);

int joystickX = 0;
int joystickY = 0;

// Log buffer
String logBuffer = "";

// Function to add messages to the log buffer
void addToLog(const String& message) {
  logBuffer += message + "<br>";
  if (logBuffer.length() > 5000) { // Limit log size to 5000 characters
    logBuffer = logBuffer.substring(logBuffer.length() - 5000);
  }
}

// Serve the log messages
void handleLogs() {
  server.send(200, "text/html", logBuffer);
}

// Motor Controller Pins (DRV8835DSSR) 
#define MOTOR_A_PHASE_PIN 43 
// Channel A Phase 
#define MOTOR_A_ENABLE_PIN 3 
// Channel A Enable 
#define MOTOR_B_PHASE_PIN 44 
// Channel B Phase 
#define MOTOR_B_ENABLE_PIN 1 
// Channel B Enable 
// IMU Pins (ICM-42670-P) 
#define IMU_SDA_PIN 5 
#define IMU_SCL_PIN 6 
#define IMU_I2C_ADDR 0x68

Adafruit_ICM20948 icm; // IMU object

String imuData = "Initializing..."; // IMU data to be sent to the web UI

void setupIMU() {
  if (!icm.begin_I2C(IMU_I2C_ADDR, &Wire)) {
    imuData = "IMU not detected!";
    return;
  }
  imuData = "IMU initialized!";
}

void readIMU() {
  sensors_event_t accel, gyro, temp;
  icm.getEvent(&accel, &gyro, &temp);

  // Use snprintf for efficient string formatting
  char buffer[256];
  snprintf(buffer, sizeof(buffer),
           "Accel: X=%.2f Y=%.2f Z=%.2f m/s²<br>Gyro: X=%.2f Y=%.2f Z=%.2f rad/s",
           accel.acceleration.x, accel.acceleration.y, accel.acceleration.z,
           gyro.gyro.x, gyro.gyro.y, gyro.gyro.z);
  imuData = String(buffer);
}

void handleIMU() {
  server.send(200, "text/plain", imuData); // Send precomputed IMU data
}

// Store joystick HTML UI
const char MAIN_page[] PROGMEM = R"=====( 
  <!DOCTYPE html> 
  <html> 
  <head> 
    <meta name="viewport" content="width=device-width, initial-scale=1"> 
    <title>ESP32 Joystick</title> 
    <style> 
      body { font-family: sans-serif; text-align: center; margin-top: 40px; } 
      #joystick { width: 200px; height: 200px; background: #eee; border-radius: 50%; margin: auto; touch-action: none; position: relative; } 
      #stick { width: 50px; height: 50px; background: #333; border-radius: 50%; position: absolute; left: 75px; top: 75px; } 
      #log { margin-top: 20px; font-size: 14px; text-align: left; max-height: 200px; overflow-y: auto; border: 1px solid #ccc; padding: 10px; } 
      #imu { margin-top: 20px; font-size: 16px; color: blue; } 
      #rssi { margin-top: 20px; font-size: 16px; color: orange; } 
    </style> 
  </head> 
  <body> 
    <h2>ESP32 Joystick Control</h2> 
    <div id="joystick"><div id="stick"></div></div> 
    <div>
      <label for="reverseLeft">Reverse Left Motor</label>
      <input type="checkbox" id="reverseLeft">
    </div>
    <div>
      <label for="reverseRight">Reverse Right Motor</label>
      <input type="checkbox" id="reverseRight">
    </div>
    <div id="imu">Loading IMU data...</div> 
    <div id="log">Loading logs...</div> 
    <div id="rssi">Signal Strength: Loading...</div> 
  
    <script> 
      const stick = document.getElementById('stick'); 
      const joystick = document.getElementById('joystick'); 
      let joySize = 200, stickSize = 50; 
      let centerX = (joySize - stickSize) / 2; // Horizontal center
      let bottomY = joySize - stickSize; // Bottom position
      let x = 0, y = -100; // Default joystick position (0 thrust)
  
      // Set joystick to default position (bottom)
      stick.style.left = centerX + "px";
      stick.style.top = bottomY + "px";

      // Handle joystick touch events
      joystick.addEventListener('touchstart', (e) => {
        e.preventDefault();
      });

      joystick.addEventListener('touchmove', (e) => {
        e.preventDefault();
        const rect = joystick.getBoundingClientRect();
        const touch = e.touches[0];
        let dx = touch.clientX - rect.left - joySize / 2;
        let dy = touch.clientY - rect.top - joySize / 2;

        // Constrain joystick movement within the circular area
        const distance = Math.sqrt(dx * dx + dy * dy);
        if (distance > joySize / 2) {
          const angle = Math.atan2(dy, dx);
          dx = Math.cos(angle) * joySize / 2;
          dy = Math.sin(angle) * joySize / 2;
        }

        // Update stick position
        stick.style.left = (dx + joySize / 2 - stickSize / 2) + "px";
        stick.style.top = (dy + joySize / 2 - stickSize / 2) + "px";

        // Map joystick position to x and y values (-100 to 100)
        x = Math.round((dx / (joySize / 2)) * 100);
        y = Math.round((dy / (joySize / 2)) * -100);
      });

      joystick.addEventListener('touchend', (e) => {
        e.preventDefault();
        // Reset joystick to default position (bottom)
        x = 0;
        y = -100;
        stick.style.left = centerX + "px";
        stick.style.top = bottomY + "px";
      });

      function sendPosition() { 
        const xhr = new XMLHttpRequest(); 
        xhr.open("GET", `/control?x=${x}&y=${y}`, true); 
        xhr.send(); 
      } 
  
      function fetchLogs() { 
        const xhr = new XMLHttpRequest(); 
        xhr.open("GET", "/logs", true); 
        xhr.onload = () => { 
          if (xhr.status === 200) { 
            log.innerHTML = xhr.responseText; 
            log.scrollTop = log.scrollHeight; // Auto-scroll to bottom
          } 
        }; 
        xhr.send(); 
      } 
  
      function fetchIMU() { 
        const xhr = new XMLHttpRequest(); 
        xhr.open("GET", "/imu", true); 
        xhr.onload = () => { 
          if (xhr.status === 200) { 
            imu.innerHTML = xhr.responseText; 
          } 
        }; 
        xhr.send(); 
      } 

      function fetchRSSI() { 
        const xhr = new XMLHttpRequest(); 
        xhr.open("GET", "/rssi", true); 
        xhr.onload = () => { 
          if (xhr.status === 200) { 
            rssi.innerHTML = "Signal Strength: " + xhr.responseText + " dBm"; 
          } 
        }; 
        xhr.send(); 
      } 

      const reverseLeft = document.getElementById('reverseLeft');
      const reverseRight = document.getElementById('reverseRight');

      // Send toggle state to the ESP32
      function sendToggleState() {
        const xhr = new XMLHttpRequest();
        xhr.open("GET", `/reverse?left=${reverseLeft.checked ? 1 : 0}&right=${reverseRight.checked ? 1 : 0}`, true);
        xhr.send();
      }

      // Add event listeners for the toggle switches
      reverseLeft.addEventListener('change', sendToggleState);
      reverseRight.addEventListener('change', sendToggleState);

      // Fetch logs, IMU data, and RSSI every 0.1 seconds
      setInterval(fetchLogs, 1000); 
      setInterval(fetchIMU, 200); 
      setInterval(fetchRSSI, 1000); 
      setInterval(sendPosition, 100); // Send joystick data every 100ms
    </script> 
  </body> 
  </html> 
  )=====";

void handleRoot() {
  server.send_P(200, "text/html", MAIN_page);
}

// Update motors based on joystick input
bool reverseLeftMotor = true;
bool reverseRightMotor = false;

void updateMotors(int x, int y) {
  // Map Y-axis (thrust) to motor speed (0 to 255)
  int thrust = map(y, -100, 100, 0, 255);

  // Map X-axis (steering) to differential thrust (-128 to 128)
  int steering = map(x, -100, 100, -128, 128);

  // Calculate motor speeds
  int leftSpeed = thrust - steering;
  int rightSpeed = thrust + steering;

  // Constrain motor speeds to valid PWM range (-255 to 255)
  leftSpeed = constrain(leftSpeed, -255, 255);
  rightSpeed = constrain(rightSpeed, -255, 255);

  // Reverse motor directions if toggled
  if (reverseLeftMotor) {
    leftSpeed = -leftSpeed;
  }
  if (reverseRightMotor) {
    rightSpeed = -rightSpeed;
  }

  // Set motor directions and speeds
  if (leftSpeed >= 0) {
    digitalWrite(MOTOR_A_PHASE_PIN, HIGH); // Forward
    ledcWrite(0, leftSpeed);              // Set speed
  } else {
    digitalWrite(MOTOR_A_PHASE_PIN, LOW); // Reverse
    ledcWrite(0, -leftSpeed);             // Set speed
  }

  if (rightSpeed >= 0) {
    digitalWrite(MOTOR_B_PHASE_PIN, HIGH); // Forward
    ledcWrite(1, rightSpeed);              // Set speed
  } else {
    digitalWrite(MOTOR_B_PHASE_PIN, LOW); // Reverse
    ledcWrite(1, -rightSpeed);            // Set speed
  }

  // Log motor speeds
  addToLog("Motors updated - Left: " + String(leftSpeed) + ", Right: " + String(rightSpeed));
}

// Handle joystick control
void handleControl() {
  if (server.hasArg("x") && server.hasArg("y")) {
    joystickX = server.arg("x").toInt();
    joystickY = server.arg("y").toInt();
    addToLog("Joystick X: " + String(joystickX) + ", Y: " + String(joystickY));
    updateMotors(joystickX, joystickY);
  }
  server.send(200, "text/plain", "OK");
}

void handleReverse() {
  if (server.hasArg("left")) {
    reverseLeftMotor = server.arg("left").toInt() == 1;
  }
  if (server.hasArg("right")) {
    reverseRightMotor = server.arg("right").toInt() == 1;
  }
  addToLog("Reverse Left: " + String(reverseLeftMotor) + ", Reverse Right: " + String(reverseRightMotor));
  server.send(200, "text/plain", "OK");
}

void handleRSSI() {
  int rssi = WiFi.RSSI(); // Get the current RSSI value
  server.send(200, "text/plain", String(rssi));
}

void loop() {
  static unsigned long lastIMURead = 0;

  if (millis() - lastIMURead >= 100) { // Read IMU every 0.1 seconds
    lastIMURead = millis();
    readIMU();
  }

  server.handleClient(); // Handle web server requests
}

void setup() {

  // Configure motor phase pins as outputs
  pinMode(MOTOR_A_PHASE_PIN, OUTPUT);
  pinMode(MOTOR_B_PHASE_PIN, OUTPUT);

  // Configure PWM channels for motor enable pins
  ledcSetup(0, 5000, 8); // Channel 0, 5 kHz, 8-bit resolution
  ledcSetup(1, 5000, 8); // Channel 1, 5 kHz, 8-bit resolution
  ledcAttachPin(MOTOR_A_ENABLE_PIN, 0); // Attach channel 0 to MOTOR_A_ENABLE_PIN
  ledcAttachPin(MOTOR_B_ENABLE_PIN, 1); // Attach channel 1 to MOTOR_B_ENABLE_PIN

  // Start Wi-Fi access point
  WiFi.softAP(ssid, password);
  addToLog("Access Point Started");
  addToLog("IP address: " + WiFi.softAPIP().toString());

  // Configure web server routes
  server.on("/", handleRoot);
  server.on("/control", handleControl);
  server.on("/logs", handleLogs);
  server.on("/imu", handleIMU);
  server.on("/rssi", handleRSSI);
  server.on("/reverse", handleReverse); // New route for motor direction toggle

  server.begin();
  addToLog("Web server running");

  // Initialize IMU
  Wire.begin(IMU_SDA_PIN, IMU_SCL_PIN, 400000); // Set I2C clock to 400 kHz
  setupIMU();
}