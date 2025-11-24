# Solar Tracker Add-on - Quick Reference

## 📁 Project Structure

```
solar_tracker_addon/
├── config.yaml              # Add-on configuration
├── Dockerfile              # Container build file
├── requirements.txt        # Python dependencies
├── run.sh                  # Startup script
├── README.md               # Full documentation
├── INSTALL.md              # Installation guide
├── CHANGELOG.md            # Version history
├── build.sh                # Build script
│
├── solar_tracker/          # Python application
│   ├── main.py            # Entry point
│   ├── controller.py      # Main tracking logic
│   ├── sensor_fusion.py   # 9-axis IMU fusion
│   ├── esp32_interface.py # ESP32 communication
│   ├── mqtt_interface.py  # MQTT/HA integration
│   └── web_server.py      # Web dashboard
│
└── web/                    # Web interface
    ├── index.html         # Dashboard UI
    └── static/            # Static assets
```

## 🚀 Quick Start

### Installation
1. Add repository to HA Add-on Store
2. Install "Solar Tracker Controller"
3. Configure (see below)
4. Start add-on
5. Access UI at http://homeassistant.local:8099

### Minimum Configuration
```yaml
esp32_ip: "192.168.1.100"      # Your ESP32 IP
latitude: 40.7128               # Your latitude
longitude: -74.0060             # Your longitude
timezone: "America/New_York"    # Your timezone
```

### With Ecowitt WS90 Wind Integration
```yaml
esp32_ip: "192.168.1.100"
latitude: 40.7128
longitude: -74.0060
timezone: "America/New_York"
ha_wind_speed_entity: "sensor.ws90_wind_speed"
ha_wind_gust_entity: "sensor.ws90_gust_speed"
max_wind_speed: 50
use_wind_gust_for_stow: true
```
See [ECOWITT_SETUP.md](ECOWITT_SETUP.md) for complete guide.

## 🎮 Tracking Modes

| Mode | Description | Use Case |
|------|-------------|----------|
| **Auto** | Follows sun automatically | Normal operation |
| **Manual** | User sets position | Testing, maintenance |
| **Stow** | Parks in safe position | High wind, night, storage |
| **Calibrate** | Sensor calibration | Initial setup, accuracy issues |

## 📊 Key Features

### Sun Tracking
- ✅ Astronomical calculations (Astral library)
- ✅ Accounts for latitude/longitude/timezone
- ✅ Auto-stow at night
- ✅ Sunrise/sunset aware

### Sensor Fusion
- ✅ 9-axis IMU (accel + gyro + mag)
- ✅ Complementary filter
- ✅ Drift-free orientation
- ✅ Tilt-compensated heading

### PID Control
- ✅ Separate azimuth/elevation loops
- ✅ Smooth, accurate movements
- ✅ Configurable gains
- ✅ Anti-windup protection

### Safety
- ✅ Wind speed monitoring
- ✅ Auto-stow on high wind
- ✅ Limit switch support
- ✅ Emergency stop
- ✅ Communication watchdog

## 🔌 MQTT Topics

### Status (Published)
```
solar_tracker/status          # Full JSON status
solar_tracker/azimuth         # Current azimuth
solar_tracker/elevation       # Current elevation
solar_tracker/sun_azimuth     # Target sun azimuth
solar_tracker/sun_elevation   # Target sun elevation
solar_tracker/wind_speed      # Wind speed
```

### Commands (Subscribed)
```
solar_tracker/command/mode    # "auto", "manual", "stow", "calibrate"
solar_tracker/command/move    # {"azimuth": 180, "elevation": 45}
solar_tracker/command/stow    # Trigger stow
```

## 🌐 Web API Endpoints

### Status
```
GET /api/status
Returns: Full status JSON
```

### Mode Control
```
POST /api/mode
Body: {"mode": "auto"}
Modes: auto, manual, stow, calibrate
```

### Manual Positioning
```
POST /api/target
Body: {"azimuth": 180.0, "elevation": 45.0}
```

### Emergency Stop
```
POST /api/stow
Immediately stows tracker
```

## 🔧 ESP32 API Requirements

Your ESP32 firmware must implement:

```
GET  /api/sensors     → IMU + wind data
POST /api/motor       → Motor control
POST /api/stop        → Emergency stop
POST /api/stow        → Stow position
GET  /api/limits      → Limit switches
```

### Example Sensor Response
```json
{
  "accelerometer": {"x": 0.1, "y": 0.0, "z": 9.8},
  "gyroscope": {"x": 0.0, "y": 0.0, "z": 0.0},
  "magnetometer": {"x": 25.0, "y": 5.0, "z": -15.0},
  "wind_speed": 12.5
}
```

## 📈 PID Tuning Guide

### Default Values
```yaml
pid_kp: 1.0    # Proportional gain
pid_ki: 0.1    # Integral gain
pid_kd: 0.05   # Derivative gain
```

### Tuning Tips

**Oscillating?**
- Lower Kp → 0.5
- Lower Kd → 0.02

**Too slow?**
- Raise Kp → 2.0

**Steady-state error?**
- Raise Ki → 0.3

**Overshooting?**
- Lower Kp
- Raise Kd

Change one parameter at a time!

## 🧭 Calibration Procedures

### Gyroscope
1. Keep tracker stationary
2. Set mode to "Calibrate"
3. Wait 30 seconds
4. Done automatically

### Magnetometer
1. Set mode to "Calibrate"
2. Rotate 360° in azimuth
3. Tilt through elevation range
4. Takes ~2 minutes
5. Done automatically

### Verification
1. Point at known direction (compass)
2. Check azimuth matches
3. Use level for elevation
4. Should be accurate to ~1°

## 🏠 Home Assistant Integration

### Auto-discovered Entities
- `sensor.solar_tracker_azimuth`
- `sensor.solar_tracker_elevation`
- `sensor.solar_tracker_sun_azimuth`
- `sensor.solar_tracker_sun_elevation`
- `sensor.solar_tracker_wind_speed`

### Example Automation
```yaml
automation:
  - alias: "High Wind Stow"
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

### Example Lovelace Card
```yaml
type: entities
title: Solar Tracker
entities:
  - sensor.solar_tracker_azimuth
  - sensor.solar_tracker_elevation
  - sensor.solar_tracker_sun_azimuth
  - sensor.solar_tracker_sun_elevation
  - sensor.solar_tracker_wind_speed
```

## 🐛 Common Issues

### Add-on won't start
- Check ESP32 IP is correct
- Verify ESP32 is reachable
- Check MQTT broker is running
- Review logs for errors

### No tracking
- Verify mode is "Auto"
- Check sun is above horizon
- Calibrate sensors
- Test in manual mode first

### Inaccurate tracking
- Run calibration procedures
- Verify location coordinates
- Check timezone setting
- Tune PID parameters

### High wind not triggering
- Check wind sensor connection
- Verify `max_wind_speed` setting
- Monitor wind speed entity
- Test with lower threshold

## 📝 Configuration Reference

```yaml
esp32_ip: "192.168.1.100"        # ESP32 IP address
esp32_port: 80                    # ESP32 port
mqtt_broker: "core-mosquitto"     # MQTT broker
mqtt_port: 1883                   # MQTT port
mqtt_user: ""                     # MQTT username (optional)
mqtt_password: ""                 # MQTT password (optional)
latitude: 40.7128                 # Location latitude
longitude: -74.0060               # Location longitude
timezone: "America/New_York"      # IANA timezone
update_interval: 1.0              # Loop rate (seconds)
pid_kp: 1.0                       # PID proportional
pid_ki: 0.1                       # PID integral
pid_kd: 0.05                      # PID derivative
max_wind_speed: 50                # Auto-stow threshold (mph)
stow_position_azimuth: 0          # Stow azimuth (degrees)
stow_position_elevation: 0        # Stow elevation (degrees)
tracking_mode: "auto"             # Initial mode
```

## 📚 Documentation Files

- **README.md** - Complete feature documentation
- **INSTALL.md** - Step-by-step installation
- **CHANGELOG.md** - Version history
- **This file** - Quick reference

## 🔗 Links

- Web UI: http://homeassistant.local:8099
- Add-on Store: Settings → Add-ons
- MQTT Tools: Developer Tools → MQTT
- Entities: Developer Tools → States
- Logs: Add-on Info → Log tab

## 💡 Pro Tips

1. **Start in Manual mode** to test hardware
2. **Calibrate sensors** before auto tracking
3. **Tune PID one parameter at a time**
4. **Monitor for a full day** before trusting
5. **Set conservative wind threshold** initially
6. **Use HA automations** for complex logic
7. **Check logs regularly** for issues
8. **Keep backup configuration** file

## 🆘 Support

- GitHub Issues: [Create issue]
- HA Community: [Post in forum]
- Logs: Check add-on logs first
- Testing: Use Manual mode to isolate issues
