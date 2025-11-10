#pragma once

#include "esphome.h"
#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"

using namespace esphome;

// HWT905 Protocol Constants
#define HWT905_TIME_PACKET      0x50
#define HWT905_ACCEL_PACKET     0x51
#define HWT905_GYRO_PACKET      0x52
#define HWT905_ANGLE_PACKET     0x53
#define HWT905_MAG_PACKET       0x54

#define HWT905_HEADER           0x55
#define HWT905_PACKET_SIZE      11

#define HWT905_CMD_SAVE         0x00
#define HWT905_CMD_CALIBRATE    0x01
#define HWT905_CMD_EXIT_CALIB   0x00

// Calibration commands as per HWT905 documentation
#define HWT905_UNLOCK_REG       0x69
#define HWT905_ACCEL_CALIB      0x01
#define HWT905_MAG_CALIB        0x07
#define HWT905_EXIT_CALIB       0x00
#define HWT905_SAVE_CONFIG      0x00

/**
 * HWT905 9-axis IMU Sensor Component
 * Communicates via RS485 UART
 * Provides roll, pitch, yaw (heading), and acceleration data
 */
class HWT905Sensor : public PollingComponent, public uart::UARTDevice {
 public:
  sensor::Sensor *elevation_sensor = new sensor::Sensor();
  sensor::Sensor *heading_sensor = new sensor::Sensor();
  sensor::Sensor *accel_x_sensor = new sensor::Sensor();
  sensor::Sensor *accel_y_sensor = new sensor::Sensor();
  sensor::Sensor *accel_z_sensor = new sensor::Sensor();

  HWT905Sensor(uart::UARTComponent *parent) : PollingComponent(100), uart::UARTDevice(parent) {
    memset(rx_buffer_, 0, sizeof(rx_buffer_));
    rx_index_ = 0;
  }

  void setup() override {
    ESP_LOGCONFIG("HWT905", "Setting up HWT905 sensor...");
    
    // Wait for sensor to initialize
    delay(1000);
    
    // Request initial data
    request_data();
  }

  void update() override {
    // Read available data from UART
    while (available()) {
      uint8_t byte;
      read_byte(&byte);
      process_byte(byte);
    }
    
    // Periodically request data
    request_data_counter_++;
    if (request_data_counter_ >= 10) {  // Every ~1 second
      request_data();
      request_data_counter_ = 0;
    }
  }

  void loop() override {
    // Process any incoming bytes
    while (available()) {
      uint8_t byte;
      read_byte(&byte);
      process_byte(byte);
    }
  }

  /**
   * Calibrate the sensor (accelerometer and magnetometer)
   * As per HWT905 documentation
   */
  void calibrate() {
    ESP_LOGI("HWT905", "Starting calibration sequence...");
    
    // Unlock configuration registers
    send_command(0x69, HWT905_UNLOCK_REG, 0xB5, 0x88);
    delay(100);
    
    // Start accelerometer calibration
    ESP_LOGI("HWT905", "Calibrating accelerometer - keep device level and stable");
    send_command(0x01, HWT905_ACCEL_CALIB, 0x00, 0x00);
    delay(5000);  // Wait 5 seconds for accel calibration
    
    // Exit accelerometer calibration
    send_command(0x01, HWT905_EXIT_CALIB, 0x00, 0x00);
    delay(100);
    
    // Start magnetometer calibration
    ESP_LOGI("HWT905", "Calibrating magnetometer - rotate device in figure-8 pattern");
    send_command(0x01, HWT905_MAG_CALIB, 0x00, 0x00);
    delay(15000);  // Wait 15 seconds for mag calibration
    
    // Exit magnetometer calibration
    send_command(0x01, HWT905_EXIT_CALIB, 0x00, 0x00);
    delay(100);
    
    // Save configuration
    ESP_LOGI("HWT905", "Saving calibration data...");
    send_command(0x00, HWT905_SAVE_CONFIG, 0x00, 0x00);
    delay(500);
    
    ESP_LOGI("HWT905", "Calibration complete!");
  }

  float get_current_elevation() {
    return current_elevation_;
  }

  float get_current_heading() {
    return current_heading_;
  }

 private:
  uint8_t rx_buffer_[HWT905_PACKET_SIZE];
  uint8_t rx_index_;
  int request_data_counter_ = 0;
  
  float current_elevation_ = 0.0;
  float current_heading_ = 0.0;
  float current_roll_ = 0.0;

  void process_byte(uint8_t byte) {
    // Looking for packet header
    if (rx_index_ == 0) {
      if (byte == HWT905_HEADER) {
        rx_buffer_[rx_index_++] = byte;
      }
    } else if (rx_index_ == 1) {
      // Second byte is packet type
      rx_buffer_[rx_index_++] = byte;
      
      // Validate packet type
      if (byte < 0x50 || byte > 0x5F) {
        rx_index_ = 0;  // Reset on invalid packet type
      }
    } else {
      rx_buffer_[rx_index_++] = byte;
      
      // Full packet received
      if (rx_index_ >= HWT905_PACKET_SIZE) {
        if (validate_checksum(rx_buffer_, HWT905_PACKET_SIZE)) {
          parse_packet(rx_buffer_);
        } else {
          ESP_LOGW("HWT905", "Checksum failed");
        }
        rx_index_ = 0;
      }
    }
  }

  bool validate_checksum(uint8_t *data, uint8_t len) {
    uint8_t sum = 0;
    for (int i = 0; i < len - 1; i++) {
      sum += data[i];
    }
    return sum == data[len - 1];
  }

  void parse_packet(uint8_t *data) {
    uint8_t packet_type = data[1];
    
    switch (packet_type) {
      case HWT905_ACCEL_PACKET: {
        // Acceleration data (16-bit signed, LSB first)
        int16_t ax = (int16_t)(data[3] << 8 | data[2]);
        int16_t ay = (int16_t)(data[5] << 8 | data[4]);
        int16_t az = (int16_t)(data[7] << 8 | data[6]);
        
        // Convert to m/s² (range ±16g)
        float accel_x = (ax / 32768.0) * 16.0 * 9.81;
        float accel_y = (ay / 32768.0) * 16.0 * 9.81;
        float accel_z = (az / 32768.0) * 16.0 * 9.81;
        
        accel_x_sensor->publish_state(accel_x);
        accel_y_sensor->publish_state(accel_y);
        accel_z_sensor->publish_state(accel_z);
        
        ESP_LOGV("HWT905", "Accel: X=%.3f, Y=%.3f, Z=%.3f m/s²", accel_x, accel_y, accel_z);
        break;
      }
      
      case HWT905_ANGLE_PACKET: {
        // Angle data (16-bit signed, LSB first)
        int16_t roll_raw = (int16_t)(data[3] << 8 | data[2]);
        int16_t pitch_raw = (int16_t)(data[5] << 8 | data[4]);
        int16_t yaw_raw = (int16_t)(data[7] << 8 | data[6]);
        
        // Convert to degrees (range ±180°)
        current_roll_ = (roll_raw / 32768.0) * 180.0;
        float pitch = (pitch_raw / 32768.0) * 180.0;
        float yaw = (yaw_raw / 32768.0) * 180.0;
        
        // For solar tracker:
        // Elevation = pitch angle (tilt up/down)
        // Heading = yaw angle (rotation left/right)
        current_elevation_ = pitch;
        current_heading_ = yaw;
        
        // Normalize heading to 0-360
        if (current_heading_ < 0) {
          current_heading_ += 360.0;
        }
        
        elevation_sensor->publish_state(current_elevation_);
        heading_sensor->publish_state(current_heading_);
        
        ESP_LOGD("HWT905", "Angles: Roll=%.2f°, Pitch(Elev)=%.2f°, Yaw(Head)=%.2f°", 
                 current_roll_, current_elevation_, current_heading_);
        break;
      }
      
      default:
        ESP_LOGV("HWT905", "Received packet type: 0x%02X", packet_type);
        break;
    }
  }

  void request_data() {
    // The HWT905 automatically sends data, but we can request specific packets
    // For now, rely on automatic transmission
  }

  void send_command(uint8_t reg, uint8_t data_high, uint8_t data_low1, uint8_t data_low2) {
    uint8_t cmd[5];
    cmd[0] = 0xFF;
    cmd[1] = 0xAA;
    cmd[2] = reg;
    cmd[3] = data_high;
    cmd[4] = data_low1;
    
    write_array(cmd, 5);
    flush();
    
    ESP_LOGV("HWT905", "Sent command: REG=0x%02X, DATA=0x%02X 0x%02X 0x%02X", 
             reg, data_high, data_low1, data_low2);
  }
};

/**
 * Motor Controller for Solar Tracker
 * Controls elevation (linear actuator) and azimuth (slewing drive)
 * Uses H-bridge drivers connected to GPIO pins
 */
class SolarTrackerMotorController : public Component {
 public:
  SolarTrackerMotorController(int elev_fwd, int elev_bwd, int azi_cw, int azi_ccw, int home_pin)
      : elevation_forward_pin_(elev_fwd),
        elevation_backward_pin_(elev_bwd),
        azimuth_cw_pin_(azi_cw),
        azimuth_ccw_pin_(azi_ccw),
        home_switch_pin_(home_pin) {}

  void setup() override {
    ESP_LOGCONFIG("MotorController", "Setting up motor controller...");
    
    // Configure GPIO pins
    pinMode(elevation_forward_pin_, OUTPUT);
    pinMode(elevation_backward_pin_, OUTPUT);
    pinMode(azimuth_cw_pin_, OUTPUT);
    pinMode(azimuth_ccw_pin_, OUTPUT);
    pinMode(home_switch_pin_, INPUT_PULLUP);
    
    // Ensure all motors are stopped
    stop_all_motors();
    
    ESP_LOGCONFIG("MotorController", "Motor controller initialized");
  }

  void loop() override {
    // Handle homing sequence
    if (homing_active_) {
      update_homing_sequence();
    }
    
    // Handle elevation movement
    if (elevation_active_) {
      update_elevation_movement();
    }
    
    // Handle azimuth movement
    if (azimuth_active_) {
      update_azimuth_movement();
    }
    
    // Safety timeout check
    check_safety_timeout();
  }

  /**
   * Set elevation angle
   * Runs linear actuator until target angle is reached
   */
  void set_elevation(float target_angle) {
    if (emergency_stop_active_) {
      ESP_LOGW("MotorController", "Emergency stop active - ignoring elevation command");
      return;
    }
    
    target_elevation_ = constrain(target_angle, 0.0f, 90.0f);
    elevation_active_ = true;
    elevation_start_time_ = millis();
    
    ESP_LOGI("MotorController", "Setting elevation to %.2f°", target_elevation_);
  }

  /**
   * Set azimuth/heading angle
   * Runs slewing drive until target heading is reached
   */
  void set_azimuth(float target_angle) {
    if (emergency_stop_active_) {
      ESP_LOGW("MotorController", "Emergency stop active - ignoring azimuth command");
      return;
    }
    
    if (!azimuth_homed_) {
      ESP_LOGW("MotorController", "Azimuth not homed - please run home_azimuth() first");
      return;
    }
    
    // Normalize to 0-360
    while (target_angle < 0) target_angle += 360.0;
    while (target_angle >= 360.0) target_angle -= 360.0;
    
    target_azimuth_ = target_angle;
    azimuth_active_ = true;
    azimuth_start_time_ = millis();
    azimuth_last_read_time_ = millis();
    
    ESP_LOGI("MotorController", "Setting azimuth to %.2f°", target_azimuth_);
  }

  /**
   * Home the azimuth axis
   * Rotates CCW until home switch is triggered, then sets that position as 0°
   */
  void home_azimuth() {
    if (emergency_stop_active_) {
      ESP_LOGW("MotorController", "Emergency stop active - ignoring homing command");
      return;
    }
    
    ESP_LOGI("MotorController", "Starting azimuth homing sequence...");
    
    homing_active_ = true;
    homing_phase_ = HOMING_MOVE_OFF_SWITCH;
    homing_start_time_ = millis();
    azimuth_homed_ = false;
    
    // If already on switch, back off first
    if (is_home_switch_pressed()) {
      ESP_LOGD("MotorController", "Already on home switch, backing off...");
      homing_phase_ = HOMING_MOVE_OFF_SWITCH;
    } else {
      ESP_LOGD("MotorController", "Moving to home switch...");
      homing_phase_ = HOMING_SEEK_SWITCH;
    }
  }

  void stop_all_motors() {
    stop_elevation_motor();
    stop_azimuth_motor();
    
    elevation_active_ = false;
    azimuth_active_ = false;
    homing_active_ = false;
    
    ESP_LOGI("MotorController", "All motors stopped");
  }

  void emergency_stop() {
    emergency_stop_active_ = true;
    stop_all_motors();
    
    ESP_LOGW("MotorController", "EMERGENCY STOP ACTIVATED");
  }

  void reset_emergency_stop() {
    emergency_stop_active_ = false;
    ESP_LOGI("MotorController", "Emergency stop reset");
  }

 private:
  // Pin assignments
  int elevation_forward_pin_;
  int elevation_backward_pin_;
  int azimuth_cw_pin_;
  int azimuth_ccw_pin_;
  int home_switch_pin_;
  
  // Target angles
  float target_elevation_ = 0.0;
  float target_azimuth_ = 0.0;
  float azimuth_home_offset_ = 0.0;  // Offset to apply after homing
  
  // State flags
  bool elevation_active_ = false;
  bool azimuth_active_ = false;
  bool emergency_stop_active_ = false;
  bool homing_active_ = false;
  bool azimuth_homed_ = false;
  
  // Homing phases
  enum HomingPhase {
    HOMING_MOVE_OFF_SWITCH,
    HOMING_SEEK_SWITCH,
    HOMING_SLOW_APPROACH,
    HOMING_SET_ZERO,
    HOMING_COMPLETE
  };
  HomingPhase homing_phase_ = HOMING_MOVE_OFF_SWITCH;
  
  // Timing
  unsigned long elevation_start_time_ = 0;
  unsigned long azimuth_start_time_ = 0;
  unsigned long azimuth_last_read_time_ = 0;
  unsigned long homing_start_time_ = 0;
  unsigned long homing_phase_start_ = 0;
  
  // Constants
  const float ELEVATION_TOLERANCE = 0.5;  // degrees
  const float AZIMUTH_TOLERANCE = 2.0;    // degrees
  const unsigned long MOTOR_TIMEOUT = 120000;  // 2 minutes max runtime
  const unsigned long AZIMUTH_READ_INTERVAL = 500;  // Read heading every 500ms
  const unsigned long AZIMUTH_BURST_TIME = 300;  // Run motor for 300ms bursts
  const unsigned long HOMING_TIMEOUT = 180000;  // 3 minutes for homing
  const unsigned long HOMING_BACKOFF_TIME = 2000;  // 2 seconds to move off switch
  const unsigned long HOMING_SLOW_APPROACH_TIME = 100;  // 100ms pulses for slow approach
  const unsigned long HOMING_SETTLE_TIME = 500;  // Wait time after finding home

  void update_elevation_movement() {
    auto hwt905 = App.get_component_by_name<HWT905Sensor*>("hwt905_sensor");
    if (!hwt905) {
      ESP_LOGW("MotorController", "HWT905 sensor not found");
      stop_elevation_motor();
      elevation_active_ = false;
      return;
    }
    
    float current_elevation = hwt905->get_current_elevation();
    float error = target_elevation_ - current_elevation;
    
    ESP_LOGV("MotorController", "Elevation: Current=%.2f°, Target=%.2f°, Error=%.2f°", 
             current_elevation, target_elevation_, error);
    
    if (abs(error) < ELEVATION_TOLERANCE) {
      // Target reached
      stop_elevation_motor();
      elevation_active_ = false;
      ESP_LOGI("MotorController", "Elevation target reached: %.2f°", current_elevation);
    } else if (error > 0) {
      // Need to move forward (increase angle)
      run_elevation_forward();
    } else {
      // Need to move backward (decrease angle)
      run_elevation_backward();
    }
  }

  void update_azimuth_movement() {
    // Check if it's time to read heading and adjust
    if (millis() - azimuth_last_read_time_ < AZIMUTH_READ_INTERVAL) {
      return;
    }
    
    auto hwt905 = App.get_component_by_name<HWT905Sensor*>("hwt905_sensor");
    if (!hwt905) {
      ESP_LOGW("MotorController", "HWT905 sensor not found");
      stop_azimuth_motor();
      azimuth_active_ = false;
      return;
    }
    
    azimuth_last_read_time_ = millis();
    
    float current_azimuth = get_corrected_azimuth();
    float error = calculate_azimuth_error(current_azimuth, target_azimuth_);
    
    ESP_LOGV("MotorController", "Azimuth: Current=%.2f°, Target=%.2f°, Error=%.2f°", 
             current_azimuth, target_azimuth_, error);
    
    if (abs(error) < AZIMUTH_TOLERANCE) {
      // Target reached
      stop_azimuth_motor();
      azimuth_active_ = false;
      ESP_LOGI("MotorController", "Azimuth target reached: %.2f°", current_azimuth);
    } else {
      // Run motor in burst mode
      if (error > 0) {
        // Need to rotate clockwise
        run_azimuth_burst_cw();
      } else {
        // Need to rotate counter-clockwise
        run_azimuth_burst_ccw();
      }
    }
  }

  float calculate_azimuth_error(float current, float target) {
    // Calculate shortest path error considering 0/360 wraparound
    float error = target - current;
    
    if (error > 180.0) {
      error -= 360.0;
    } else if (error < -180.0) {
      error += 360.0;
    }
    
    return error;
  }

  void update_homing_sequence() {
    // Check for timeout
    if (millis() - homing_start_time_ > HOMING_TIMEOUT) {
      ESP_LOGW("MotorController", "Homing timeout - failed to find home switch");
      stop_azimuth_motor();
      homing_active_ = false;
      return;
    }
    
    switch (homing_phase_) {
      case HOMING_MOVE_OFF_SWITCH:
        // If we're on the switch, back off slowly
        if (is_home_switch_pressed()) {
          run_azimuth_cw();  // Move away from switch
          
          if (millis() - homing_phase_start_ > HOMING_BACKOFF_TIME) {
            ESP_LOGW("MotorController", "Failed to move off home switch");
            stop_azimuth_motor();
            homing_active_ = false;
          }
        } else {
          // Successfully moved off switch
          stop_azimuth_motor();
          delay(200);
          homing_phase_ = HOMING_SEEK_SWITCH;
          homing_phase_start_ = millis();
          ESP_LOGD("MotorController", "Moved off switch, now seeking...");
        }
        break;
        
      case HOMING_SEEK_SWITCH:
        // Move CCW until switch is found
        if (is_home_switch_pressed()) {
          // Found the switch! Move to slow approach
          stop_azimuth_motor();
          delay(200);
          homing_phase_ = HOMING_SLOW_APPROACH;
          homing_phase_start_ = millis();
          ESP_LOGD("MotorController", "Home switch found, backing off for slow approach...");
          
          // Back off a bit
          run_azimuth_cw();
          delay(500);
          stop_azimuth_motor();
          delay(200);
        } else {
          // Keep moving CCW
          run_azimuth_ccw();
        }
        break;
        
      case HOMING_SLOW_APPROACH:
        // Slowly approach the switch with pulses
        if (is_home_switch_pressed()) {
          // Switch triggered!
          stop_azimuth_motor();
          homing_phase_ = HOMING_SET_ZERO;
          homing_phase_start_ = millis();
          ESP_LOGI("MotorController", "Home position found!");
        } else {
          // Pulse CCW slowly
          run_azimuth_ccw();
          delay(HOMING_SLOW_APPROACH_TIME);
          stop_azimuth_motor();
          delay(100);
        }
        break;
        
      case HOMING_SET_ZERO:
        // Wait for things to settle, then set zero position
        if (millis() - homing_phase_start_ > HOMING_SETTLE_TIME) {
          // Set current position as home (0 degrees)
          set_azimuth_zero();
          homing_phase_ = HOMING_COMPLETE;
          ESP_LOGI("MotorController", "Azimuth homed successfully - zero position set");
        }
        break;
        
      case HOMING_COMPLETE:
        homing_active_ = false;
        azimuth_homed_ = true;
        break;
    }
  }

  bool is_home_switch_pressed() {
    // Switch is active LOW (pressed = LOW, released = HIGH with pullup)
    return digitalRead(home_switch_pin_) == LOW;
  }

  void set_azimuth_zero() {
    // Tell the HWT905 sensor that current position is home (0 degrees)
    // We'll store an offset to apply to all future readings
    auto hwt905 = App.get_component_by_name<HWT905Sensor*>("hwt905_sensor");
    if (hwt905) {
      float current_heading = hwt905->get_current_heading();
      azimuth_home_offset_ = current_heading;
      ESP_LOGI("MotorController", "Home offset set to %.2f°", azimuth_home_offset_);
    } else {
      ESP_LOGW("MotorController", "Could not access HWT905 sensor for zero setting");
    }
  }

  float get_corrected_azimuth() {
    // Get heading with home offset applied
    auto hwt905 = App.get_component_by_name<HWT905Sensor*>("hwt905_sensor");
    if (!hwt905) return 0.0;
    
    float raw_heading = hwt905->get_current_heading();
    float corrected = raw_heading - azimuth_home_offset_;
    
    // Normalize to 0-360
    while (corrected < 0) corrected += 360.0;
    while (corrected >= 360.0) corrected -= 360.0;
    
    return corrected;
  }

  void check_safety_timeout() {
    if (elevation_active_ && (millis() - elevation_start_time_ > MOTOR_TIMEOUT)) {
      ESP_LOGW("MotorController", "Elevation motor timeout - stopping");
      stop_elevation_motor();
      elevation_active_ = false;
    }
    
    if (azimuth_active_ && (millis() - azimuth_start_time_ > MOTOR_TIMEOUT)) {
      ESP_LOGW("MotorController", "Azimuth motor timeout - stopping");
      stop_azimuth_motor();
      azimuth_active_ = false;
    }
    
    if (homing_active_ && (millis() - homing_start_time_ > HOMING_TIMEOUT)) {
      ESP_LOGW("MotorController", "Homing timeout - stopping");
      stop_azimuth_motor();
      homing_active_ = false;
    }
  }

  // Motor control primitives
  void run_elevation_forward() {
    digitalWrite(elevation_forward_pin_, HIGH);
    digitalWrite(elevation_backward_pin_, LOW);
  }

  void run_elevation_backward() {
    digitalWrite(elevation_forward_pin_, LOW);
    digitalWrite(elevation_backward_pin_, HIGH);
  }

  void stop_elevation_motor() {
    digitalWrite(elevation_forward_pin_, LOW);
    digitalWrite(elevation_backward_pin_, LOW);
  }

  void run_azimuth_burst_cw() {
    digitalWrite(azimuth_cw_pin_, HIGH);
    digitalWrite(azimuth_ccw_pin_, LOW);
    delay(AZIMUTH_BURST_TIME);
    stop_azimuth_motor();
  }

  void run_azimuth_burst_ccw() {
    digitalWrite(azimuth_cw_pin_, LOW);
    digitalWrite(azimuth_ccw_pin_, HIGH);
    delay(AZIMUTH_BURST_TIME);
    stop_azimuth_motor();
  }

  void stop_azimuth_motor() {
    digitalWrite(azimuth_cw_pin_, LOW);
    digitalWrite(azimuth_ccw_pin_, LOW);
  }
  
  // Continuous motor control (for homing)
  void run_azimuth_cw() {
    digitalWrite(azimuth_cw_pin_, HIGH);
    digitalWrite(azimuth_ccw_pin_, LOW);
  }
  
  void run_azimuth_ccw() {
    digitalWrite(azimuth_cw_pin_, LOW);
    digitalWrite(azimuth_ccw_pin_, HIGH);
  }
};
