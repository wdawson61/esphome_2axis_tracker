/*
 * Solar Tracker ESP32 Hardware Controller
 * 
 * This firmware runs on the ESP32 and provides:
 * - 9-axis IMU reading (MPU9250 or similar)
 * - Motor control (PWM for azimuth and elevation)
 * - Limit switch monitoring
 * - RS485/Modbus communication
 * - HTTP API for Home Assistant add-on
 * - Web interface for direct control
 * 
 * Hardware connections:
 * - MPU9250 IMU: I2C (SDA=21, SCL=22)
 * - Azimuth Motor: PWM pins (DIR=25, PWM=26)
 * - Elevation Motor: PWM pins (DIR=27, PWM=14)
 * - Limit Switches: GPIO pins (pull-up)
 * - RS485: UART (see earlier configuration)
 */

#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <ArduinoJson.h>

// WiFi credentials - CHANGE THESE
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Motor control pins
#define MOTOR_AZ_DIR 25
#define MOTOR_AZ_PWM 26
#define MOTOR_EL_DIR 27
#define MOTOR_EL_PWM 14

// Limit switch pins (active LOW with internal pull-up)
#define LIMIT_AZ_CW 32
#define LIMIT_AZ_CCW 33
#define LIMIT_EL_UP 34
#define LIMIT_EL_DOWN 35

// I2C pins for IMU
#define I2C_SDA 21
#define I2C_SCL 22

// MPU9250 I2C address
#define MPU9250_ADDR 0x68
#define AK8963_ADDR 0x0C  // Magnetometer

// PWM settings
#define PWM_FREQ 5000
#define PWM_RESOLUTION 8  // 0-255
#define PWM_CHANNEL_AZ 0
#define PWM_CHANNEL_EL 1

// Web server
WebServer server(80);

// IMU data structure
struct IMUData {
  float accel_x, accel_y, accel_z;
  float gyro_x, gyro_y, gyro_z;
  float mag_x, mag_y, mag_z;
  float temperature;
};

IMUData imuData;

// Motor state
struct MotorState {
  int azimuth_speed;   // -100 to 100
  int elevation_speed; // -100 to 100
  bool stopped;
};

MotorState motorState = {0, 0, true};

// Limit switch states
struct LimitSwitches {
  bool az_cw_limit;
  bool az_ccw_limit;
  bool el_up_limit;
  bool el_down_limit;
};

LimitSwitches limits;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\nSolar Tracker ESP32 Controller");
  
  // Initialize I2C for IMU
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(400000);  // 400kHz
  
  // Initialize IMU
  if (!initIMU()) {
    Serial.println("ERROR: Failed to initialize IMU!");
  } else {
    Serial.println("IMU initialized successfully");
  }
  
  // Initialize motor control pins
  pinMode(MOTOR_AZ_DIR, OUTPUT);
  pinMode(MOTOR_EL_DIR, OUTPUT);
  
  // Setup PWM
  ledcSetup(PWM_CHANNEL_AZ, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CHANNEL_EL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(MOTOR_AZ_PWM, PWM_CHANNEL_AZ);
  ledcAttachPin(MOTOR_EL_PWM, PWM_CHANNEL_EL);
  
  // Stop motors initially
  stopMotors();
  
  // Initialize limit switches
  pinMode(LIMIT_AZ_CW, INPUT_PULLUP);
  pinMode(LIMIT_AZ_CCW, INPUT_PULLUP);
  pinMode(LIMIT_EL_UP, INPUT_PULLUP);
  pinMode(LIMIT_EL_DOWN, INPUT_PULLUP);
  
  // Connect to WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  // Setup web server routes
  server.on("/", handleRoot);
  server.on("/api/sensors", handleGetSensors);
  server.on("/api/motor", HTTP_POST, handleSetMotor);
  server.on("/api/stop", HTTP_POST, handleStop);
  server.on("/api/stow", HTTP_POST, handleStow);
  server.on("/api/limits", handleGetLimits);
  server.on("/status", handleStatus);
  
  server.begin();
  Serial.println("HTTP server started");
  Serial.println("Ready!");
}

void loop() {
  server.handleClient();
  
  // Read sensors periodically
  static unsigned long lastSensorRead = 0;
  if (millis() - lastSensorRead > 100) {  // 10Hz
    readIMU();
    readLimitSwitches();
    lastSensorRead = millis();
  }
  
  // Safety: stop motors if limit switches hit
  checkLimitsAndStopIfNeeded();
}

// ==================== IMU Functions ====================

bool initIMU() {
  // Wake up MPU9250
  Wire.beginTransmission(MPU9250_ADDR);
  Wire.write(0x6B);  // PWR_MGMT_1
  Wire.write(0x00);  // Clear sleep bit
  if (Wire.endTransmission() != 0) {
    return false;
  }
  delay(100);
  
  // Configure accelerometer (±8g)
  Wire.beginTransmission(MPU9250_ADDR);
  Wire.write(0x1C);  // ACCEL_CONFIG
  Wire.write(0x10);  // ±8g
  Wire.endTransmission();
  
  // Configure gyroscope (±1000°/s)
  Wire.beginTransmission(MPU9250_ADDR);
  Wire.write(0x1B);  // GYRO_CONFIG
  Wire.write(0x10);  // ±1000°/s
  Wire.endTransmission();
  
  // Enable magnetometer bypass
  Wire.beginTransmission(MPU9250_ADDR);
  Wire.write(0x37);  // INT_PIN_CFG
  Wire.write(0x02);  // Bypass enable
  Wire.endTransmission();
  
  delay(100);
  
  // Initialize magnetometer
  Wire.beginTransmission(AK8963_ADDR);
  Wire.write(0x0A);  // Control register
  Wire.write(0x16);  // 16-bit, 100Hz
  Wire.endTransmission();
  
  return true;
}

void readIMU() {
  // Read accelerometer
  Wire.beginTransmission(MPU9250_ADDR);
  Wire.write(0x3B);  // ACCEL_XOUT_H
  Wire.endTransmission(false);
  Wire.requestFrom(MPU9250_ADDR, 6, true);
  
  int16_t ax = (Wire.read() << 8) | Wire.read();
  int16_t ay = (Wire.read() << 8) | Wire.read();
  int16_t az = (Wire.read() << 8) | Wire.read();
  
  // Convert to g (±8g range)
  imuData.accel_x = ax / 4096.0;
  imuData.accel_y = ay / 4096.0;
  imuData.accel_z = az / 4096.0;
  
  // Read gyroscope
  Wire.beginTransmission(MPU9250_ADDR);
  Wire.write(0x43);  // GYRO_XOUT_H
  Wire.endTransmission(false);
  Wire.requestFrom(MPU9250_ADDR, 6, true);
  
  int16_t gx = (Wire.read() << 8) | Wire.read();
  int16_t gy = (Wire.read() << 8) | Wire.read();
  int16_t gz = (Wire.read() << 8) | Wire.read();
  
  // Convert to °/s (±1000°/s range)
  imuData.gyro_x = gx / 32.8;
  imuData.gyro_y = gy / 32.8;
  imuData.gyro_z = gz / 32.8;
  
  // Read temperature
  Wire.beginTransmission(MPU9250_ADDR);
  Wire.write(0x41);  // TEMP_OUT_H
  Wire.endTransmission(false);
  Wire.requestFrom(MPU9250_ADDR, 2, true);
  
  int16_t temp = (Wire.read() << 8) | Wire.read();
  imuData.temperature = (temp / 333.87) + 21.0;
  
  // Read magnetometer
  Wire.beginTransmission(AK8963_ADDR);
  Wire.write(0x03);  // MAG_XOUT_L
  Wire.endTransmission(false);
  Wire.requestFrom(AK8963_ADDR, 7, true);
  
  int16_t mx = Wire.read() | (Wire.read() << 8);
  int16_t my = Wire.read() | (Wire.read() << 8);
  int16_t mz = Wire.read() | (Wire.read() << 8);
  Wire.read();  // ST2 register
  
  // Convert to μT (approximate)
  imuData.mag_x = mx * 0.15;
  imuData.mag_y = my * 0.15;
  imuData.mag_z = mz * 0.15;
}

// ==================== Motor Control Functions ====================

void setMotorSpeed(String motor, int speed) {
  // Clamp speed to -100 to 100
  speed = constrain(speed, -100, 100);
  
  if (motor == "azimuth") {
    motorState.azimuth_speed = speed;
    
    // Set direction
    digitalWrite(MOTOR_AZ_DIR, speed >= 0 ? HIGH : LOW);
    
    // Set PWM (0-255)
    int pwm = map(abs(speed), 0, 100, 0, 255);
    ledcWrite(PWM_CHANNEL_AZ, pwm);
    
    motorState.stopped = false;
    
  } else if (motor == "elevation") {
    motorState.elevation_speed = speed;
    
    // Set direction
    digitalWrite(MOTOR_EL_DIR, speed >= 0 ? HIGH : LOW);
    
    // Set PWM (0-255)
    int pwm = map(abs(speed), 0, 100, 0, 255);
    ledcWrite(PWM_CHANNEL_EL, pwm);
    
    motorState.stopped = false;
  }
}

void stopMotors() {
  ledcWrite(PWM_CHANNEL_AZ, 0);
  ledcWrite(PWM_CHANNEL_EL, 0);
  motorState.azimuth_speed = 0;
  motorState.elevation_speed = 0;
  motorState.stopped = true;
  Serial.println("Motors stopped");
}

void stowPosition() {
  // Move to stow position (this is handled by add-on)
  // ESP32 just stops motors when commanded
  stopMotors();
  Serial.println("Stow position commanded");
}

// ==================== Limit Switch Functions ====================

void readLimitSwitches() {
  // Active LOW (switch closed = LOW)
  limits.az_cw_limit = !digitalRead(LIMIT_AZ_CW);
  limits.az_ccw_limit = !digitalRead(LIMIT_AZ_CCW);
  limits.el_up_limit = !digitalRead(LIMIT_EL_UP);
  limits.el_down_limit = !digitalRead(LIMIT_EL_DOWN);
}

void checkLimitsAndStopIfNeeded() {
  // Stop azimuth motor if hitting limit
  if (limits.az_cw_limit && motorState.azimuth_speed > 0) {
    Serial.println("Azimuth CW limit hit!");
    setMotorSpeed("azimuth", 0);
  }
  if (limits.az_ccw_limit && motorState.azimuth_speed < 0) {
    Serial.println("Azimuth CCW limit hit!");
    setMotorSpeed("azimuth", 0);
  }
  
  // Stop elevation motor if hitting limit
  if (limits.el_up_limit && motorState.elevation_speed > 0) {
    Serial.println("Elevation UP limit hit!");
    setMotorSpeed("elevation", 0);
  }
  if (limits.el_down_limit && motorState.elevation_speed < 0) {
    Serial.println("Elevation DOWN limit hit!");
    setMotorSpeed("elevation", 0);
  }
}

// ==================== Web Server Handlers ====================

void handleRoot() {
  String html = F("<!DOCTYPE html><html><head><title>Solar Tracker ESP32</title>");
  html += F("<meta name='viewport' content='width=device-width,initial-scale=1'>");
  html += F("<style>body{font-family:Arial;margin:20px;background:#f0f0f0}");
  html += F(".card{background:white;padding:20px;margin:10px 0;border-radius:8px;box-shadow:0 2px 4px rgba(0,0,0,0.1)}");
  html += F("h1{color:#333}h2{color:#666;font-size:18px}</style></head><body>");
  html += F("<h1>☀️ Solar Tracker ESP32</h1>");
  html += F("<div class='card'><h2>Status</h2>");
  html += F("<p>IP: "); html += WiFi.localIP().toString(); html += F("</p>");
  html += F("<p>Motors: "); html += motorState.stopped ? "Stopped" : "Running"; html += F("</p>");
  html += F("</div><div class='card'><h2>API Endpoints</h2>");
  html += F("<p>GET /api/sensors - Read IMU sensors</p>");
  html += F("<p>POST /api/motor - Set motor speed</p>");
  html += F("<p>POST /api/stop - Stop all motors</p>");
  html += F("<p>GET /api/limits - Read limit switches</p>");
  html += F("</div></body></html>");
  
  server.send(200, "text/html", html);
}

void handleGetSensors() {
  StaticJsonDocument<512> doc;
  
  JsonObject accel = doc.createNestedObject("accelerometer");
  accel["x"] = imuData.accel_x;
  accel["y"] = imuData.accel_y;
  accel["z"] = imuData.accel_z;
  
  JsonObject gyro = doc.createNestedObject("gyroscope");
  gyro["x"] = imuData.gyro_x;
  gyro["y"] = imuData.gyro_y;
  gyro["z"] = imuData.gyro_z;
  
  JsonObject mag = doc.createNestedObject("magnetometer");
  mag["x"] = imuData.mag_x;
  mag["y"] = imuData.mag_y;
  mag["z"] = imuData.mag_z;
  
  doc["temperature"] = imuData.temperature;
  doc["wind_speed"] = 0;  // Not connected - will use HA entity
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleSetMotor() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\":\"No body\"}");
    return;
  }
  
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, server.arg("plain"));
  
  if (error) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }
  
  String motor = doc["motor"].as<String>();
  int speed = doc["speed"];
  
  setMotorSpeed(motor, speed);
  
  server.send(200, "application/json", "{\"success\":true}");
}

void handleStop() {
  stopMotors();
  server.send(200, "application/json", "{\"success\":true}");
}

void handleStow() {
  stowPosition();
  server.send(200, "application/json", "{\"success\":true}");
}

void handleGetLimits() {
  StaticJsonDocument<200> doc;
  
  doc["azimuth_cw_limit"] = limits.az_cw_limit;
  doc["azimuth_ccw_limit"] = limits.az_ccw_limit;
  doc["elevation_up_limit"] = limits.el_up_limit;
  doc["elevation_down_limit"] = limits.el_down_limit;
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleStatus() {
  StaticJsonDocument<200> doc;
  
  doc["ip"] = WiFi.localIP().toString();
  doc["motors_stopped"] = motorState.stopped;
  doc["azimuth_speed"] = motorState.azimuth_speed;
  doc["elevation_speed"] = motorState.elevation_speed;
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}
