/*
 * Solar Tracker ESP32-C6 Hardware Controller
 * Hardware: XIAO ESP32-C6 + SeeedStudio RS485 Expansion Board
 * 
 * Motors: Dual-axis control
 * - Azimuth: Slewing drive (BTS7960 driver)
 * - Elevation: Linear actuator (BTS7960 driver)
 * 
 * Components:
 * - WitMotion HWT905 9-axis IMU (RS485/Modbus)
 * - 2x BTS7960 43A Motor Drivers
 * - 2x Limit switches (East/West for azimuth)
 * 
 * Pin Configuration:
 * 
 * RS485 (HWT905 IMU):
 * - TX: D4 (GPIO22)
 * - RX: D5 (GPIO23)
 * - DE: D2 (GPIO2)
 * 
 * BTS7960 #1 - Azimuth (Slewing Drive):
 * - RPWM (CW):  D8 (GPIO8)
 * - LPWM (CCW): D9 (GPIO9)
 * - R_EN: 5V
 * - L_EN: 5V
 * 
 * BTS7960 #2 - Elevation (Linear Actuator):
 * - RPWM (Up):   D10 (GPIO10)
 * - LPWM (Down): D1 (GPIO1)
 * - R_EN: 5V
 * - L_EN: 5V
 * 
 * Limit Switches:
 * - East: D6 (GPIO6)
 * - West: D7 (GPIO7)
 */

#include <WiFi.h>
#include <WebServer.h>
#include <HardwareSerial.h>
#include <ArduinoJson.h>

// ==================== Configuration ====================

// WiFi credentials - CHANGE THESE!
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// RS485 pins (HWT905 IMU)
#define RS485_TX D4  // GPIO22
#define RS485_RX D5  // GPIO23
#define RS485_DE D2  // GPIO2

// BTS7960 Driver #1 - Azimuth Motor (Slewing Drive)
#define AZ_RPWM D8   // GPIO8  - Clockwise (East)
#define AZ_LPWM D9   // GPIO9  - Counter-clockwise (West)

// BTS7960 Driver #2 - Elevation Motor (Linear Actuator)
#define EL_RPWM D10  // GPIO10 - Extend (Up)
#define EL_LPWM D1   // GPIO1  - Retract (Down)

// Limit Switches
#define LIMIT_EAST D6  // GPIO6
#define LIMIT_WEST D7  // GPIO7

// User LED
#define USER_LED LED_BUILTIN

// PWM Configuration
#define PWM_FREQ 1000  // 1kHz works well for BTS7960
#define PWM_RES 8      // 0-255

// PWM Channels
#define CH_AZ_R 0  // Azimuth forward
#define CH_AZ_L 1  // Azimuth reverse
#define CH_EL_R 2  // Elevation up
#define CH_EL_L 3  // Elevation down

// HWT905 Settings
#define HWT905_ADDR 0x50
#define HWT905_BAUD 115200

// ==================== Global Objects ====================

HardwareSerial RS485Serial(1);
WebServer server(80);

// ==================== Data Structures ====================

struct IMUData {
  float accel_x, accel_y, accel_z;
  float gyro_x, gyro_y, gyro_z;
  float mag_x, mag_y, mag_z;
  float roll, pitch, yaw;
  float temperature;
  unsigned long lastUpdate;
  bool valid;
} imu = {0};

struct MotorState {
  int azimuth_speed;    // -100 to 100
  int elevation_speed;  // -100 to 100
  bool stopped;
} motors = {0, 0, true};

struct LimitSwitches {
  bool east;
  bool west;
} limits = {false, false};

// ==================== Function Prototypes ====================

uint16_t calculateCRC16(uint8_t *data, int length);
bool readHWT905();
void parseHWT905(uint8_t* data, int len);
void setAzimuthMotor(int speed);
void setElevationMotor(int speed);
void stopAllMotors();
void readLimits();
void checkLimits();

// ==================== Setup ====================

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  
  unsigned long start = millis();
  while (!Serial && (millis() - start < 3000)) delay(10);
  delay(500);
  
  Serial.println("\n================================");
  Serial.println("Solar Tracker ESP32-C6");
  Serial.println("Dual-Axis with BTS7960 Drivers");
  Serial.println("================================\n");
  
  // RS485 for HWT905
  Serial.printf("Configuring UART1: RX=GPIO%d, TX=GPIO%d\n", RS485_RX, RS485_TX);
  if (!RS485Serial.setPins(RS485_RX, RS485_TX, -1, -1)) {
    Serial.println("ERROR: UART1 pin config failed!");
  }
  RS485Serial.begin(HWT905_BAUD, SERIAL_8N1, RS485_RX, RS485_TX);
  
  pinMode(RS485_DE, OUTPUT);
  digitalWrite(RS485_DE, LOW);
  pinMode(USER_LED, OUTPUT);
  digitalWrite(USER_LED, HIGH);
  
  Serial.println("RS485 initialized for HWT905");
  
  // BTS7960 Motor Drivers
  // For BTS7960: RPWM = forward, LPWM = reverse, both LOW = stopped
  
  // Azimuth (Slewing Drive)
  ledcSetup(CH_AZ_R, PWM_FREQ, PWM_RES);
  ledcSetup(CH_AZ_L, PWM_FREQ, PWM_RES);
  ledcAttachPin(AZ_RPWM, CH_AZ_R);
  ledcAttachPin(AZ_LPWM, CH_AZ_L);
  
  // Elevation (Linear Actuator)
  ledcSetup(CH_EL_R, PWM_FREQ, PWM_RES);
  ledcSetup(CH_EL_L, PWM_FREQ, PWM_RES);
  ledcAttachPin(EL_RPWM, CH_EL_R);
  ledcAttachPin(EL_LPWM, CH_EL_L);
  
  stopAllMotors();
  
  Serial.println("BTS7960 Motor Drivers initialized:");
  Serial.printf("  Azimuth:   RPWM=D8(GPIO%d), LPWM=D9(GPIO%d)\n", AZ_RPWM, AZ_LPWM);
  Serial.printf("  Elevation: RPWM=D10(GPIO%d), LPWM=D1(GPIO%d)\n", EL_RPWM, EL_LPWM);
  
  // Limit Switches
  pinMode(LIMIT_EAST, INPUT_PULLUP);
  pinMode(LIMIT_WEST, INPUT_PULLUP);
  Serial.println("Limit switches configured");
  
  // WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  
  // Web Server
  server.on("/", handleRoot);
  server.on("/api/sensors", handleGetSensors);
  server.on("/api/motor", HTTP_POST, handleSetMotor);
  server.on("/api/stop", HTTP_POST, handleStop);
  server.on("/api/stow", HTTP_POST, handleStow);
  server.on("/api/limits", handleGetLimits);
  server.on("/status", handleStatus);
  
  server.begin();
  Serial.println("HTTP server started on port 80");
  Serial.println("\nReady!\n");
}

// ==================== Main Loop ====================

void loop() {
  server.handleClient();
  
  static unsigned long lastRead = 0;
  if (millis() - lastRead > 100) {  // 10Hz
    readHWT905();
    readLimits();
    checkLimits();
    lastRead = millis();
  }
}

// ==================== Motor Control ====================

void setAzimuthMotor(int speed) {
  // speed: -100 to 100
  // Negative = CCW (West), Positive = CW (East)
  speed = constrain(speed, -100, 100);
  motors.azimuth_speed = speed;
  
  int pwm = map(abs(speed), 0, 100, 0, 255);
  
  if (speed > 0) {
    // Clockwise (East)
    ledcWrite(CH_AZ_R, pwm);
    ledcWrite(CH_AZ_L, 0);
  } else if (speed < 0) {
    // Counter-clockwise (West)
    ledcWrite(CH_AZ_R, 0);
    ledcWrite(CH_AZ_L, pwm);
  } else {
    // Stop
    ledcWrite(CH_AZ_R, 0);
    ledcWrite(CH_AZ_L, 0);
  }
  
  motors.stopped = (motors.azimuth_speed == 0 && motors.elevation_speed == 0);
}

void setElevationMotor(int speed) {
  // speed: -100 to 100
  // Negative = Retract (Down), Positive = Extend (Up)
  speed = constrain(speed, -100, 100);
  motors.elevation_speed = speed;
  
  int pwm = map(abs(speed), 0, 100, 0, 255);
  
  if (speed > 0) {
    // Extend (Up)
    ledcWrite(CH_EL_R, pwm);
    ledcWrite(CH_EL_L, 0);
  } else if (speed < 0) {
    // Retract (Down)
    ledcWrite(CH_EL_R, 0);
    ledcWrite(CH_EL_L, pwm);
  } else {
    // Stop
    ledcWrite(CH_EL_R, 0);
    ledcWrite(CH_EL_L, 0);
  }
  
  motors.stopped = (motors.azimuth_speed == 0 && motors.elevation_speed == 0);
}

void stopAllMotors() {
  ledcWrite(CH_AZ_R, 0);
  ledcWrite(CH_AZ_L, 0);
  ledcWrite(CH_EL_R, 0);
  ledcWrite(CH_EL_L, 0);
  motors.azimuth_speed = 0;
  motors.elevation_speed = 0;
  motors.stopped = true;
}

// ==================== Limit Switches ====================

void readLimits() {
  // Active LOW (closed = LOW)
  limits.east = !digitalRead(LIMIT_EAST);
  limits.west = !digitalRead(LIMIT_WEST);
}

void checkLimits() {
  // Stop if moving into limit
  if (limits.east && motors.azimuth_speed > 0) {
    Serial.println("East limit hit!");
    setAzimuthMotor(0);
  }
  if (limits.west && motors.azimuth_speed < 0) {
    Serial.println("West limit hit!");
    setAzimuthMotor(0);
  }
}

// ==================== Modbus CRC16 ====================

uint16_t calculateCRC16(uint8_t *data, int length) {
  uint16_t crc = 0xFFFF;
  for (int i = 0; i < length; i++) {
    crc ^= (uint16_t)data[i];
    for (int j = 0; j < 8; j++) {
      if (crc & 0x0001) {
        crc >>= 1;
        crc ^= 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }
  return crc;
}

// ==================== HWT905 IMU ====================

bool readHWT905() {
  // Read registers 0x34-0x41 (20 registers = 40 bytes)
  // Contains: Accel, Gyro, Mag, Angles, Temperature
  
  uint8_t cmd[8];
  cmd[0] = HWT905_ADDR;
  cmd[1] = 0x03;  // Read Holding Registers
  cmd[2] = 0x00;  // Start address high
  cmd[3] = 0x34;  // Start address low (reg 52)
  cmd[4] = 0x00;  // Count high
  cmd[5] = 0x14;  // Count low (20 regs)
  
  uint16_t crc = calculateCRC16(cmd, 6);
  cmd[6] = crc & 0xFF;
  cmd[7] = (crc >> 8) & 0xFF;
  
  // Transmit
  digitalWrite(RS485_DE, HIGH);
  digitalWrite(USER_LED, LOW);
  delayMicroseconds(50);
  RS485Serial.write(cmd, 8);
  RS485Serial.flush();
  digitalWrite(RS485_DE, LOW);
  digitalWrite(USER_LED, HIGH);
  
  // Clear RX buffer
  while (RS485Serial.available()) RS485Serial.read();
  delay(10);
  
  // Receive response
  uint8_t resp[256];
  int count = 0;
  unsigned long start = millis();
  unsigned long lastByte = millis();
  bool gotFirst = false;
  
  while (millis() - start < 500) {
    if (RS485Serial.available()) {
      resp[count++] = RS485Serial.read();
      lastByte = millis();
      gotFirst = true;
      if (count >= 256) break;
    } else if (gotFirst && (millis() - lastByte > 100)) {
      break;
    }
  }
  
  // Expected: 45 bytes total
  if (count >= 45 && resp[0] == HWT905_ADDR && resp[1] == 0x03) {
    uint16_t rxCrc = resp[count-2] | (resp[count-1] << 8);
    uint16_t calcCrc = calculateCRC16(resp, count-2);
    
    if (rxCrc == calcCrc) {
      parseHWT905(resp, count);
      imu.valid = true;
      imu.lastUpdate = millis();
      return true;
    } else {
      Serial.println("HWT905: CRC error");
    }
  }
  
  if (millis() - imu.lastUpdate > 5000) {
    imu.valid = false;
  }
  
  return false;
}

void parseHWT905(uint8_t* data, int len) {
  uint8_t* p = &data[3];  // Payload starts at byte 3
  
  // Acceleration (0x34-0x36): ±16g
  int16_t ax = (p[0] << 8) | p[1];
  int16_t ay = (p[2] << 8) | p[3];
  int16_t az = (p[4] << 8) | p[5];
  imu.accel_x = ax / 32768.0 * 16.0;
  imu.accel_y = ay / 32768.0 * 16.0;
  imu.accel_z = az / 32768.0 * 16.0;
  
  // Gyroscope (0x37-0x39): ±2000°/s
  int16_t gx = (p[6] << 8) | p[7];
  int16_t gy = (p[8] << 8) | p[9];
  int16_t gz = (p[10] << 8) | p[11];
  imu.gyro_x = gx / 32768.0 * 2000.0;
  imu.gyro_y = gy / 32768.0 * 2000.0;
  imu.gyro_z = gz / 32768.0 * 2000.0;
  
  // Magnetometer (0x3A-0x3C): raw values
  int16_t mx = (p[12] << 8) | p[13];
  int16_t my = (p[14] << 8) | p[15];
  int16_t mz = (p[16] << 8) | p[17];
  imu.mag_x = mx;
  imu.mag_y = my;
  imu.mag_z = mz;
  
  // Angles (0x3D-0x3F): 0.01° resolution
  int16_t roll = (p[18] << 8) | p[19];
  int16_t pitch = (p[20] << 8) | p[21];
  int16_t yaw = (p[22] << 8) | p[23];
  imu.roll = roll / 100.0;
  imu.pitch = pitch / 100.0;
  imu.yaw = yaw / 100.0;
  
  // Temperature (0x40): 0.01°C resolution
  int16_t temp = (p[24] << 8) | p[25];
  imu.temperature = temp / 100.0;
}

// ==================== Web Handlers ====================

void handleRoot() {
  String html = F("<!DOCTYPE html><html><head>");
  html += F("<title>Solar Tracker ESP32-C6</title>");
  html += F("<meta name='viewport' content='width=device-width,initial-scale=1'>");
  html += F("<style>body{font-family:Arial;margin:20px;background:#f5f5f5}");
  html += F(".card{background:#fff;padding:20px;margin:10px 0;border-radius:8px;box-shadow:0 2px 4px rgba(0,0,0,.1)}");
  html += F("h1{color:#333;margin-top:0}h2{color:#666;font-size:18px;margin:10px 0}");
  html += F("pre{background:#f0f0f0;padding:10px;border-radius:4px;overflow-x:auto}");
  html += F(".status{display:inline-block;padding:5px 10px;border-radius:4px;font-weight:bold}");
  html += F(".ok{background:#4caf50;color:white}.err{background:#f44336;color:white}");
  html += F("</style></head><body>");
  html += F("<h1>☀️ Solar Tracker</h1>");
  html += F("<div class='card'><h2>Status</h2>");
  html += F("<p>Board: XIAO ESP32-C6</p>");
  html += F("<p>IP: "); html += WiFi.localIP().toString(); html += F("</p>");
  html += F("<p>Motors: <span class='status ");
  html += motors.stopped ? F("ok'>Stopped") : F("err'>Running");
  html += F("</span></p>");
  html += F("<p>IMU: <span class='status ");
  html += imu.valid ? F("ok'>OK") : F("err'>No Data");
  html += F("</span></p>");
  html += F("</div><div class='card'><h2>API Endpoints</h2><pre>");
  html += F("GET  /api/sensors - IMU data\n");
  html += F("POST /api/motor   - Control motors\n");
  html += F("POST /api/stop    - Emergency stop\n");
  html += F("GET  /api/limits  - Limit switches\n");
  html += F("GET  /status      - System status");
  html += F("</pre></div></body></html>");
  
  server.send(200, "text/html", html);
}

void handleGetSensors() {
  StaticJsonDocument<512> doc;
  
  JsonObject accel = doc.createNestedObject("accelerometer");
  accel["x"] = imu.accel_x;
  accel["y"] = imu.accel_y;
  accel["z"] = imu.accel_z;
  
  JsonObject gyro = doc.createNestedObject("gyroscope");
  gyro["x"] = imu.gyro_x;
  gyro["y"] = imu.gyro_y;
  gyro["z"] = imu.gyro_z;
  
  JsonObject mag = doc.createNestedObject("magnetometer");
  mag["x"] = imu.mag_x;
  mag["y"] = imu.mag_y;
  mag["z"] = imu.mag_z;
  
  JsonObject angles = doc.createNestedObject("angles");
  angles["roll"] = imu.roll;
  angles["pitch"] = imu.pitch;
  angles["yaw"] = imu.yaw;
  
  doc["temperature"] = imu.temperature;
  doc["imu_valid"] = imu.valid;
  doc["last_update"] = imu.lastUpdate;
  doc["wind_speed"] = 0;  // From HA
  
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
  if (deserializeJson(doc, server.arg("plain"))) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }
  
  String motor = doc["motor"].as<String>();
  int speed = doc["speed"];
  
  if (motor == "azimuth") {
    setAzimuthMotor(speed);
  } else if (motor == "elevation") {
    setElevationMotor(speed);
  } else {
    server.send(400, "application/json", "{\"error\":\"Invalid motor\"}");
    return;
  }
  
  server.send(200, "application/json", "{\"success\":true}");
}

void handleStop() {
  stopAllMotors();
  Serial.println("Emergency stop!");
  server.send(200, "application/json", "{\"success\":true}");
}

void handleStow() {
  stopAllMotors();
  Serial.println("Stow commanded");
  server.send(200, "application/json", "{\"success\":true}");
}

void handleGetLimits() {
  StaticJsonDocument<200> doc;
  
  doc["azimuth_cw_limit"] = limits.east;
  doc["azimuth_ccw_limit"] = limits.west;
  doc["elevation_up_limit"] = false;
  doc["elevation_down_limit"] = false;
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleStatus() {
  StaticJsonDocument<300> doc;
  
  doc["ip"] = WiFi.localIP().toString();
  doc["motor_stopped"] = motors.stopped;
  doc["azimuth_speed"] = motors.azimuth_speed;
  doc["elevation_speed"] = motors.elevation_speed;
  doc["imu_valid"] = imu.valid;
  doc["east_limit"] = limits.east;
  doc["west_limit"] = limits.west;
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}
