# Solar Tracker Controller Add-on for Home Assistant

A comprehensive Home Assistant add-on for controlling solar tracking systems with advanced features including:

- 🌞 Automatic sun position tracking using astronomical calculations
- 🧭 9-axis IMU sensor fusion (accelerometer, gyroscope, magnetometer)
- 🎮 PID control for smooth, accurate tracking
- 🌪️ Wind speed monitoring with automatic stowing
- 📡 RS485/Modbus communication for sensors
- 🎯 Manual positioning mode
- 📊 Real-time web dashboard
- 🔌 MQTT integration with Home Assistant

## Features

### Tracking Modes

1. **Auto Mode**: Automatically tracks the sun throughout the day
   - Calculates sun position based on location and time
   - Accounts for sunrise/sunset
   - Auto-stows at night

2. **Manual Mode**: Directly control azimuth and elevation
   - Set specific target angles
   - Useful for maintenance and testing

3. **Stow Mode**: Park tracker in safe position
   - Triggered manually or automatically during high wind
   - Configurable stow position

4. **Calibrate Mode**: Sensor calibration procedures
   - Gyroscope zero-point calibration
   - Magnetometer hard/soft iron calibration
   - Accelerometer offset correction

### Sensor Fusion

The add-on implements a complementary filter to fuse 9-axis IMU data:

- **Accelerometer**: Provides tilt reference (roll/pitch)
- **Gyroscope**: Measures angular velocity for fast response
- **Magnetometer**: Provides absolute heading (yaw/azimuth)

This fusion provides accurate, drift-free orientation tracking even during movement.

### PID Control

Smooth motor control using PID (Proportional-Integral-Derivative) controllers:

- Separate PID loops for azimuth and elevation
- Configurable gains (Kp, Ki, Kd)
- Deadband to prevent oscillation
- Output limiting for motor protection

### Safety Features

- **Wind monitoring**: Automatically stows when wind exceeds threshold
- **Limit switches**: Respects hardware travel limits
- **Emergency stop**: Immediate motor shutdown via web interface
- **Watchdog**: Auto-stow if communication lost

## Hardware Requirements

### ESP32 Hardware
- ESP32-C3, ESP32-C6, or ESP32-WROOM-32
- RS485-to-TTL converter
- 9-axis IMU sensor (MPU9250, BMI088+QMC5883L, or similar)
- Motor drivers (for azimuth and elevation)
- Optional: Wind speed sensor
- Optional: Limit switches

### Home Assistant
- Home Assistant OS or Supervised installation
- MQTT broker (Mosquitto add-on recommended)

## Installation

### 1. Add Repository

Add this repository URL to your Home Assistant add-on store:

```
https://github.com/yourusername/solar-tracker-addon
```

### 2. Install Add-on

1. Click "Install" on the Solar Tracker Controller add-on
2. Wait for installation to complete

### 3. Configure

Edit the configuration with your settings:

```yaml
esp32_ip: "192.168.1.100"        # Your ESP32 IP address
esp32_port: 80                    # ESP32 web server port
mqtt_broker: "core-mosquitto"     # MQTT broker (default is Mosquitto add-on)
mqtt_port: 1883
mqtt_user: ""                     # Optional MQTT username
mqtt_password: ""                 # Optional MQTT password
latitude: 40.7128                 # Your location latitude
longitude: -74.0060               # Your location longitude
timezone: "America/New_York"      # Your timezone
update_interval: 1.0              # Control loop update rate (seconds)
pid_kp: 1.0                       # PID proportional gain
pid_ki: 0.1                       # PID integral gain
pid_kd: 0.05                      # PID derivative gain
max_wind_speed: 50                # Auto-stow wind threshold (mph)
stow_position_azimuth: 0          # Stow position azimuth (degrees)
stow_position_elevation: 0        # Stow position elevation (degrees)
tracking_mode: "auto"             # Initial mode: auto, manual, stow, calibrate

# Home Assistant entity integration (optional)
ha_wind_speed_entity: ""          # e.g., "sensor.ws90_wind_speed"
ha_wind_gust_entity: ""           # e.g., "sensor.ws90_gust_speed"
ha_wind_direction_entity: ""      # e.g., "sensor.ws90_wind_direction"
ha_temperature_entity: ""         # e.g., "sensor.ws90_outdoor_temperature"
ha_humidity_entity: ""            # e.g., "sensor.ws90_humidity"
use_wind_gust_for_stow: false     # Use gust instead of average for stow threshold
```

**Note**: If you have an existing weather station (like Ecowitt WS90) integrated in Home Assistant, you can use its wind data instead of a separate sensor on the ESP32. See [ECOWITT_SETUP.md](ECOWITT_SETUP.md) for details.

### 4. Start Add-on

Click "Start" and monitor the logs for successful startup.

## Web Interface

Access the control interface at:

```
http://homeassistant.local:8099
```

Or use your Home Assistant IP:

```
http://192.168.1.100:8099
```

### Dashboard Features

- **Real-time Position Display**: Current azimuth/elevation with compass visualization
- **Sun Position**: Calculated sun azimuth/elevation
- **Tracking Errors**: Shows how far off-target the tracker is
- **Mode Control**: Switch between Auto/Manual/Stow/Calibrate modes
- **Manual Control**: Slider controls for manual positioning
- **Emergency Stop**: Immediate motor shutdown button
- **Status Indicators**: Moving indicator, wind speed, stow status

## MQTT Topics

The add-on publishes and subscribes to these MQTT topics:

### Published (Status)

```
solar_tracker/status              # Full status JSON
solar_tracker/azimuth             # Current azimuth (degrees)
solar_tracker/elevation           # Current elevation (degrees)
solar_tracker/sun_azimuth         # Sun azimuth (degrees)
solar_tracker/sun_elevation       # Sun elevation (degrees)
solar_tracker/wind_speed          # Wind speed (mph)
```

### Subscribed (Commands)

```
solar_tracker/command/mode        # Set mode: "auto", "manual", "stow", "calibrate"
solar_tracker/command/move        # Manual move: {"azimuth": 180, "elevation": 45}
solar_tracker/command/stow        # Trigger stow
```

## Home Assistant Integration

The add-on automatically creates Home Assistant entities via MQTT discovery:

- `sensor.solar_tracker_azimuth`
- `sensor.solar_tracker_elevation`
- `sensor.solar_tracker_sun_azimuth`
- `sensor.solar_tracker_sun_elevation`
- `sensor.solar_tracker_wind_speed`

### Example Automation

```yaml
automation:
  - alias: "Solar Tracker - Stow on High Wind"
    trigger:
      platform: numeric_state
      entity_id: sensor.solar_tracker_wind_speed
      above: 45
    action:
      service: mqtt.publish
      data:
        topic: "solar_tracker/command/mode"
        payload: "stow"
```

### Example Dashboard Card

```yaml
type: entities
title: Solar Tracker
entities:
  - entity: sensor.solar_tracker_azimuth
    name: Current Azimuth
  - entity: sensor.solar_tracker_elevation
    name: Current Elevation
  - entity: sensor.solar_tracker_sun_azimuth
    name: Sun Azimuth
  - entity: sensor.solar_tracker_sun_elevation
    name: Sun Elevation
  - entity: sensor.solar_tracker_wind_speed
    name: Wind Speed
```

## ESP32 Firmware Requirements

Your ESP32 must provide the following HTTP API endpoints:

### GET `/api/sensors`
Returns JSON with sensor data:
```json
{
  "accelerometer": {"x": 0.1, "y": 0.0, "z": 9.8},
  "gyroscope": {"x": 0.0, "y": 0.0, "z": 0.0},
  "magnetometer": {"x": 25.0, "y": 5.0, "z": -15.0},
  "wind_speed": 12.5
}
```

### POST `/api/motor`
Set motor speed:
```json
{
  "motor": "azimuth",  // or "elevation"
  "speed": 50          // -100 to 100
}
```

### POST `/api/stop`
Stop all motors immediately.

### POST `/api/stow`
Move to stow position.

### GET `/api/limits`
Get limit switch states:
```json
{
  "azimuth_cw_limit": false,
  "azimuth_ccw_limit": false,
  "elevation_up_limit": false,
  "elevation_down_limit": false
}
```

## Calibration Procedures

### Gyroscope Calibration

1. Set mode to "calibrate"
2. Keep tracker completely stationary
3. Wait 30 seconds for gyro bias calculation
4. Calibration values saved automatically

### Magnetometer Calibration

1. Set mode to "calibrate"
2. Slowly rotate tracker through full 360° in azimuth
3. Also tilt through full elevation range
4. Collects min/max values for hard/soft iron correction
5. Calibration values saved automatically

### Accelerometer Calibration

1. Position tracker perfectly level (use level tool)
2. Set mode to "calibrate"
3. Calibration values saved automatically

## Tuning PID Controllers

If tracking is:

- **Oscillating**: Reduce Kp, reduce Kd
- **Too slow**: Increase Kp
- **Has steady-state error**: Increase Ki
- **Overshooting**: Reduce Kp, increase Kd

Start with provided defaults and adjust incrementally.

## Troubleshooting

### Add-on won't start
- Check logs for error messages
- Verify ESP32 IP is correct and reachable
- Ensure MQTT broker is running

### No sensor data
- Verify ESP32 firmware is running
- Check ESP32 HTTP API endpoints
- Test ESP32 connection manually with curl

### Tracking inaccurate
- Run calibration procedures
- Verify location coordinates are correct
- Check timezone setting
- Tune PID parameters

### High wind not triggering stow
- Verify wind sensor is connected and working
- Check `max_wind_speed` configuration
- Monitor `sensor.solar_tracker_wind_speed`

### Motors not moving
- Check limit switches
- Verify motor driver connections
- Test motors in manual mode
- Check ESP32 motor control endpoints

## Architecture

```
┌─────────────────────────────────────────┐
│        Home Assistant Add-on            │
│                                         │
│  ┌──────────────────────────────────┐  │
│  │   Solar Tracker Controller       │  │
│  │   - Sun Position Calculation     │  │
│  │   - Sensor Fusion                │  │
│  │   - PID Control                  │  │
│  │   - Safety Logic                 │  │
│  └──────────────────────────────────┘  │
│           ▲                  ▲          │
│           │                  │          │
│    ┌──────┴──────┐    ┌─────┴──────┐   │
│    │ ESP32 HTTP  │    │    MQTT    │   │
│    │  Interface  │    │ Interface  │   │
│    └──────┬──────┘    └─────┬──────┘   │
└───────────┼─────────────────┼──────────┘
            │                 │
            ▼                 ▼
    ┌───────────────┐  ┌────────────┐
    │  ESP32        │  │ Home       │
    │  Hardware     │  │ Assistant  │
    │               │  │ Core       │
    │  - IMU        │  │            │
    │  - Motors     │  │ - Entities │
    │  - RS485      │  │ - Auto     │
    │  - Sensors    │  │ - Dash     │
    └───────────────┘  └────────────┘
```

## Support

For issues, questions, or contributions:
- GitHub Issues: https://github.com/yourusername/solar-tracker-addon/issues
- Home Assistant Community: https://community.home-assistant.io/

## License

MIT License - See LICENSE file for details
