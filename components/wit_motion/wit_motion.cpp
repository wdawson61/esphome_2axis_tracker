#include "wit_motion.h"
#include "esphome/core/log.h"

namespace esphome {
namespace wit_motion {

static const char *const TAG = "wit_motion";

void WitMotionComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up WIT-Motion HWT905...");
}

void WitMotionComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "WIT-Motion HWT905:");
  LOG_SENSOR("  ", "Acceleration X", this->acceleration_x_sensor_);
  LOG_SENSOR("  ", "Acceleration Y", this->acceleration_y_sensor_);
  LOG_SENSOR("  ", "Acceleration Z", this->acceleration_z_sensor_);
  LOG_SENSOR("  ", "Angular Velocity X", this->angular_velocity_x_sensor_);
  LOG_SENSOR("  ", "Angular Velocity Y", this->angular_velocity_y_sensor_);
  LOG_SENSOR("  ", "Angular Velocity Z", this->angular_velocity_z_sensor_);
  LOG_SENSOR("  ", "Roll", this->roll_sensor_);
  LOG_SENSOR("  ", "Pitch", this->pitch_sensor_);
  LOG_SENSOR("  ", "Heading", this->heading_sensor_);
  LOG_SENSOR("  ", "Temperature", this->temperature_sensor_);
}

void WitMotionComponent::loop() {
  // Read available data from UART
  while (this->available()) {
    uint8_t byte;
    this->read_byte(&byte);
    
    // Look for packet header (0x55)
    if (this->rx_buffer_.empty() && byte != WIT_MOTION_HEADER) {
      continue;  // Skip until we find header
    }
    
    this->rx_buffer_.push_back(byte);
    
    // WIT-Motion packet is 11 bytes: 0x55 + Type + 8 data bytes + Checksum
    if (this->rx_buffer_.size() >= 11) {
      // Verify checksum
      uint8_t calculated_checksum = this->calculate_checksum_(this->rx_buffer_.data(), 10);
      uint8_t received_checksum = this->rx_buffer_[10];
      
      if (calculated_checksum == received_checksum) {
        // Parse the packet
        uint8_t packet_type = this->rx_buffer_[1];
        this->parse_data_packet_(packet_type, &this->rx_buffer_[2]);
      } else {
        ESP_LOGW(TAG, "Checksum mismatch: calculated=0x%02X, received=0x%02X", 
                 calculated_checksum, received_checksum);
      }
      
      // Clear buffer for next packet
      this->rx_buffer_.clear();
    }
  }
}

uint8_t WitMotionComponent::calculate_checksum_(const uint8_t *data, size_t len) {
  uint8_t sum = 0;
  for (size_t i = 0; i < len; i++) {
    sum += data[i];
  }
  return sum;
}

void WitMotionComponent::parse_data_packet_(uint8_t packet_type, const uint8_t *data) {
  int16_t raw_x, raw_y, raw_z;
  float value_x, value_y, value_z;
  
  switch (packet_type) {
    case WIT_ACCELERATION:
      // Acceleration data: 8 bytes (3x int16 + int16 temperature)
      // Scale: 16-bit signed / 32768.0 * 16g = actual g-value
      // Convert to m/s² by multiplying by 9.81
      raw_x = (int16_t)((data[1] << 8) | data[0]);
      raw_y = (int16_t)((data[3] << 8) | data[2]);
      raw_z = (int16_t)((data[5] << 8) | data[4]);
      
      value_x = (raw_x / 32768.0f) * 16.0f * 9.81f;
      value_y = (raw_y / 32768.0f) * 16.0f * 9.81f;
      value_z = (raw_z / 32768.0f) * 16.0f * 9.81f;
      
      if (this->acceleration_x_sensor_ != nullptr)
        this->acceleration_x_sensor_->publish_state(value_x);
      if (this->acceleration_y_sensor_ != nullptr)
        this->acceleration_y_sensor_->publish_state(value_y);
      if (this->acceleration_z_sensor_ != nullptr)
        this->acceleration_z_sensor_->publish_state(value_z);
      
      ESP_LOGV(TAG, "Acceleration: X=%.3f, Y=%.3f, Z=%.3f m/s²", value_x, value_y, value_z);
      break;
      
    case WIT_ANGULAR_VELOCITY:
      // Angular velocity (gyroscope) data
      // Scale: 16-bit signed / 32768.0 * 2000 deg/s
      raw_x = (int16_t)((data[1] << 8) | data[0]);
      raw_y = (int16_t)((data[3] << 8) | data[2]);
      raw_z = (int16_t)((data[5] << 8) | data[4]);
      
      value_x = (raw_x / 32768.0f) * 2000.0f;
      value_y = (raw_y / 32768.0f) * 2000.0f;
      value_z = (raw_z / 32768.0f) * 2000.0f;
      
      if (this->angular_velocity_x_sensor_ != nullptr)
        this->angular_velocity_x_sensor_->publish_state(value_x);
      if (this->angular_velocity_y_sensor_ != nullptr)
        this->angular_velocity_y_sensor_->publish_state(value_y);
      if (this->angular_velocity_z_sensor_ != nullptr)
        this->angular_velocity_z_sensor_->publish_state(value_z);
      
      ESP_LOGV(TAG, "Angular Velocity: X=%.2f, Y=%.2f, Z=%.2f °/s", value_x, value_y, value_z);
      break;
      
    case WIT_ANGLE:
      // Angle (orientation) data
      // Scale: 16-bit signed / 32768.0 * 180 degrees
      raw_x = (int16_t)((data[1] << 8) | data[0]);  // Roll
      raw_y = (int16_t)((data[3] << 8) | data[2]);  // Pitch
      raw_z = (int16_t)((data[5] << 8) | data[4]);  // Yaw (Heading)
      
      value_x = (raw_x / 32768.0f) * 180.0f;  // Roll
      value_y = (raw_y / 32768.0f) * 180.0f;  // Pitch
      value_z = (raw_z / 32768.0f) * 180.0f;  // Yaw/Heading
      
      if (this->roll_sensor_ != nullptr)
        this->roll_sensor_->publish_state(value_x);
      if (this->pitch_sensor_ != nullptr)
        this->pitch_sensor_->publish_state(value_y);
      if (this->heading_sensor_ != nullptr)
        this->heading_sensor_->publish_state(value_z);
      
      ESP_LOGV(TAG, "Angle: Roll=%.2f, Pitch=%.2f, Heading=%.2f °", value_x, value_y, value_z);
      break;
      
    case WIT_TEMPERATURE:
      // Temperature data
      // Temperature is in bytes 0-1 as int16
      raw_x = (int16_t)((data[1] << 8) | data[0]);
      value_x = raw_x / 100.0f;  // Temperature in Celsius
      
      if (this->temperature_sensor_ != nullptr)
        this->temperature_sensor_->publish_state(value_x);
      
      ESP_LOGV(TAG, "Temperature: %.1f °C", value_x);
      break;
      
    default:
      ESP_LOGV(TAG, "Unknown packet type: 0x%02X", packet_type);
      break;
  }
}

}  // namespace wit_motion
}  // namespace esphome
