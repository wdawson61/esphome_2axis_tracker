# ESPHome 2-Axis Solar Tracker

ESPHome firmware project to implement a controller for a dual-axis solar tracker using ESP32-C6 and WIT-Motion HWT905 IMU sensor.

## Features

- **ESP32-C6 Support**: Optimized for ESP32-C6 hardware
- **WIT-Motion HWT905 Integration**: RS485 communication for reading:
  - 3-axis acceleration data
  - 3-axis angular velocity (gyroscope)
  - Orientation angles (roll, pitch, heading/yaw)
  - Temperature sensor
- **Motor Control**: GPIO-based H-bridge control for:
  - Elevation axis (linear actuator)
  - Azimuth axis (for future implementation)
- **Home Assistant Integration**: Full API support
- **Web Interface**: Built-in web server for monitoring and control

## Hardware Requirements

- ESP32-C6 development board (e.g., ESP32-C6-DevKitC-1)
- WIT-Motion HWT905 IMU sensor with RS485 interface
- 2x H-bridge motor driver boards
- Linear actuator for elevation control
- Stepper or DC motor for azimuth control (optional)

## Pin Configuration

### UART (RS485 for WIT-Motion HWT905)
- TX: GPIO16
- RX: GPIO17
- Baud rate: 115200

### Motor Control GPIOs
- **Elevation Motor**:
  - Forward: GPIO4
  - Backward: GPIO5
- **Azimuth Motor** (future use):
  - Forward: GPIO6
  - Backward: GPIO7

## Installation

1. Install ESPHome:
```bash
pip install esphome
```

2. Clone this repository:
```bash
git clone https://github.com/wdawson61/esphome_2axis_tracker.git
cd esphome_2axis_tracker
```

3. Create your secrets file:
```bash
cp secrets.yaml.template secrets.yaml
```

4. Edit `secrets.yaml` with your WiFi credentials and API keys:
```yaml
wifi_ssid: "YourWiFiSSID"
wifi_password: "YourWiFiPassword"
ap_password: "YourAPPassword"
api_encryption_key: "YourAPIEncryptionKey"
ota_password: "YourOTAPassword"
```

5. Compile and upload the firmware:
```bash
esphome run solar_tracker.yaml
```

## Configuration

The main configuration file is `solar_tracker.yaml`. You can customize:

- GPIO pin assignments
- UART settings
- Update intervals for sensors
- Motor control parameters
- Network settings

## Usage

### Setting Elevation Angle

The tracker provides a number input entity called "Target Elevation Angle" that accepts values from -90° to 90°. When you set a new target angle:

1. The system calculates the difference between current and target angles
2. Activates the appropriate motor direction (forward/backward)
3. Runs the motor for the calculated duration
4. Stops the motor when the target is reached

### Manual Control

Use the "Stop Elevation Motor" button to immediately stop the elevation motor.

### Monitoring

The tracker provides several sensors:
- **Acceleration**: X, Y, Z axes in m/s²
- **Angular Velocity**: X, Y, Z axes in °/s
- **Orientation**: Roll, Pitch, Heading in degrees
- **Temperature**: IMU temperature in °C
- **Status**: Current and target elevation angles

## WIT-Motion HWT905 Protocol

The custom component implements the WIT-Motion serial protocol:
- Packet structure: Header (0x55) + Type + 8 data bytes + Checksum
- Supported packet types:
  - 0x51: Acceleration data
  - 0x52: Angular velocity data
  - 0x53: Angle data (roll, pitch, yaw/heading)
  - 0x56: Temperature data

## Customization

### Adjusting Motor Speed

The script calculates motor runtime assuming 1°/second movement. Modify this in the `set_elevation_angle` script:

```yaml
# Current: 1 degree per second
float run_time_ms = abs(angle_diff) * 1000.0;

# For faster movement (e.g., 2 degrees per second):
float run_time_ms = abs(angle_diff) * 500.0;
```

### Adding Azimuth Control

The configuration includes GPIO pins for azimuth motor control (GPIO6/GPIO7). You can create similar scripts and actions for azimuth control following the elevation pattern.

## Troubleshooting

### No Data from WIT-Motion Sensor
- Check RS485 wiring (TX, RX, GND)
- Verify baud rate (default: 115200)
- Check UART pin configuration
- Enable verbose logging in `logger` section

### Motor Not Moving
- Verify H-bridge connections
- Check GPIO pin assignments
- Ensure adequate power supply for motors
- Review motor control logic in logs

### WiFi Connection Issues
- Verify credentials in `secrets.yaml`
- Check WiFi signal strength
- Use fallback AP mode to access web interface

## Development

### Project Structure
```
esphome_2axis_tracker/
├── solar_tracker.yaml          # Main ESPHome configuration
├── secrets.yaml.template       # Template for secrets
├── components/
│   └── wit_motion/            # Custom WIT-Motion component
│       ├── sensor.py          # Python configuration
│       ├── wit_motion.h       # C++ header
│       └── wit_motion.cpp     # C++ implementation
└── README.md                  # This file
```

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

This project is open source. Please check the repository for license details.

## References

- [ESPHome Documentation](https://esphome.io/)
- [WIT-Motion HWT905 Datasheet](https://www.wit-motion.com/)
- [ESP32-C6 Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/)
