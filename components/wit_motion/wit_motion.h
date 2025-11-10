#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace wit_motion {

// WIT-Motion HWT905 Protocol Constants
static const uint8_t WIT_MOTION_HEADER = 0x55;

// Data packet IDs
static const uint8_t WIT_TIME = 0x50;
static const uint8_t WIT_ACCELERATION = 0x51;
static const uint8_t WIT_ANGULAR_VELOCITY = 0x52;
static const uint8_t WIT_ANGLE = 0x53;
static const uint8_t WIT_MAGNETIC = 0x54;
static const uint8_t WIT_PRESSURE = 0x55;
static const uint8_t WIT_TEMPERATURE = 0x56;

class WitMotionComponent : public Component, public uart::UARTDevice {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  // Sensor setters
  void set_acceleration_x_sensor(sensor::Sensor *sensor) { acceleration_x_sensor_ = sensor; }
  void set_acceleration_y_sensor(sensor::Sensor *sensor) { acceleration_y_sensor_ = sensor; }
  void set_acceleration_z_sensor(sensor::Sensor *sensor) { acceleration_z_sensor_ = sensor; }
  
  void set_angular_velocity_x_sensor(sensor::Sensor *sensor) { angular_velocity_x_sensor_ = sensor; }
  void set_angular_velocity_y_sensor(sensor::Sensor *sensor) { angular_velocity_y_sensor_ = sensor; }
  void set_angular_velocity_z_sensor(sensor::Sensor *sensor) { angular_velocity_z_sensor_ = sensor; }
  
  void set_roll_sensor(sensor::Sensor *sensor) { roll_sensor_ = sensor; }
  void set_pitch_sensor(sensor::Sensor *sensor) { pitch_sensor_ = sensor; }
  void set_heading_sensor(sensor::Sensor *sensor) { heading_sensor_ = sensor; }
  
  void set_temperature_sensor(sensor::Sensor *sensor) { temperature_sensor_ = sensor; }

 protected:
  // Parse received data packet
  void parse_data_packet_(uint8_t packet_type, const uint8_t *data);
  
  // Calculate checksum
  uint8_t calculate_checksum_(const uint8_t *data, size_t len);
  
  // Sensor objects
  sensor::Sensor *acceleration_x_sensor_{nullptr};
  sensor::Sensor *acceleration_y_sensor_{nullptr};
  sensor::Sensor *acceleration_z_sensor_{nullptr};
  
  sensor::Sensor *angular_velocity_x_sensor_{nullptr};
  sensor::Sensor *angular_velocity_y_sensor_{nullptr};
  sensor::Sensor *angular_velocity_z_sensor_{nullptr};
  
  sensor::Sensor *roll_sensor_{nullptr};
  sensor::Sensor *pitch_sensor_{nullptr};
  sensor::Sensor *heading_sensor_{nullptr};
  
  sensor::Sensor *temperature_sensor_{nullptr};
  
  // Buffer for incoming data
  std::vector<uint8_t> rx_buffer_;
};

}  // namespace wit_motion
}  // namespace esphome
