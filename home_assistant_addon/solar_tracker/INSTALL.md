# Installation Guide

This guide walks you through installing and configuring the Solar Tracker Controller add-on.

## Prerequisites

### Home Assistant Requirements
- Home Assistant OS or Home Assistant Supervised
- Version 2023.1 or newer
- MQTT Broker (Mosquitto add-on recommended)
- Network access to ESP32 device

### Hardware Requirements
- ESP32 device (C3, C6, or WROOM-32) with firmware
- 9-axis IMU sensor connected to ESP32
- Motor controllers connected to ESP32
- RS485 connection for Modbus devices (optional)
- Wind sensor (optional but recommended)

## Step-by-Step Installation

### 1. Install MQTT Broker (if not already installed)

1. Go to **Settings** → **Add-ons** → **Add-on Store**
2. Search for "Mosquitto broker"
3. Click **Install**
4. Go to **Configuration** tab
5. Set username and password (optional)
6. Click **Save**
7. Go to **Info** tab and click **Start**
8. Enable **Start on boot**

### 2. Prepare ESP32 Firmware

Your ESP32 must have firmware that provides the required HTTP API endpoints. See the README for endpoint specifications.

Basic ESP32 requirements:
- Web server on port 80
- GET `/api/sensors` - Returns IMU and wind data
- POST `/api/motor` - Controls motors
- POST `/api/stop` - Emergency stop
- POST `/api/stow` - Stow command
- GET `/api/limits` - Limit switch states

### 3. Add the Add-on Repository

1. Go to **Settings** → **Add-ons** → **Add-on Store**
2. Click the **⋮** menu (three dots) in the top right
3. Select **Repositories**
4. Add this URL:
   ```
   https://github.com/yourusername/solar-tracker-addon
   ```
5. Click **Add**
6. Close the dialog

### 4. Install Solar Tracker Controller Add-on

1. Refresh the Add-on Store page
2. Find **Solar Tracker Controller** in the list
3. Click on it
4. Click **Install**
5. Wait for installation to complete (may take several minutes)

### 5. Configure the Add-on

1. Go to the **Configuration** tab
2. Edit the configuration YAML:

```yaml
esp32_ip: "192.168.1.100"        # Replace with your ESP32's IP
esp32_port: 80
mqtt_broker: "core-mosquitto"
mqtt_port: 1883
mqtt_user: ""                     # If you set one in Mosquitto
mqtt_password: ""                 # If you set one in Mosquitto
latitude: 40.7128                 # Your latitude
longitude: -74.0060               # Your longitude
timezone: "America/New_York"      # Your timezone
update_interval: 1.0
pid_kp: 1.0
pid_ki: 0.1
pid_kd: 0.05
max_wind_speed: 50
stow_position_azimuth: 0
stow_position_elevation: 0
tracking_mode: "auto"
```

3. Click **Save**

### 6. Find Your Location Coordinates

To get accurate sun tracking, you need precise coordinates:

**Option 1: Google Maps**
1. Go to Google Maps
2. Right-click your location
3. Click the coordinates that appear
4. Copy latitude and longitude

**Option 2: GPS Device**
Use your phone or GPS device to get coordinates.

**Option 3: Online Tool**
Use https://www.latlong.net/

### 7. Set Your Timezone

Use standard timezone names from: https://en.wikipedia.org/wiki/List_of_tz_database_time_zones

Examples:
- `America/New_York`
- `America/Los_Angeles`
- `Europe/London`
- `Australia/Sydney`

### 8. Start the Add-on

1. Go to the **Info** tab
2. Click **Start**
3. Monitor the **Log** tab for startup messages
4. Look for: "Solar Tracker Controller started successfully"

### 9. Enable Auto-start (Optional but Recommended)

1. In the **Info** tab
2. Enable **Start on boot**
3. Enable **Watchdog** (auto-restart on crash)

### 10. Access Web Interface

Open a browser and go to:
```
http://homeassistant.local:8099
```

Or using your Home Assistant IP:
```
http://192.168.1.50:8099
```

You should see the Solar Tracker Control dashboard.

## Verification

### Check Add-on Logs

Look for these messages in the logs:

```
Solar Tracker Controller
Initializing subsystems...
Connected to ESP32
Connected to MQTT broker
Published MQTT discovery configuration
Starting main control loop
Web server started on http://0.0.0.0:8099
Solar Tracker Controller started successfully
```

### Check Home Assistant Entities

Go to **Developer Tools** → **States** and look for:
- `sensor.solar_tracker_azimuth`
- `sensor.solar_tracker_elevation`
- `sensor.solar_tracker_sun_azimuth`
- `sensor.solar_tracker_sun_elevation`
- `sensor.solar_tracker_wind_speed`

These should appear automatically via MQTT discovery.

### Test Web Interface

1. Open web interface at port 8099
2. Verify you see:
   - Current position display
   - Sun position
   - Mode controls
   - Manual controls

3. Try switching modes (Manual → Auto → Stow)
4. Check that status updates in real-time

### Test MQTT Communication

Use **Developer Tools** → **MQTT** to test:

**Subscribe to status:**
```
solar_tracker/status
```

**Send a command:**
Topic: `solar_tracker/command/mode`
Payload: `manual`

## Initial Calibration

After installation, calibrate your sensors:

### 1. Gyroscope Calibration

1. Ensure tracker is completely stationary
2. Set mode to "Calibrate" in web interface
3. Wait 30 seconds
4. Gyro offsets calculated and saved

### 2. Magnetometer Calibration

1. Set mode to "Calibrate"
2. Slowly rotate tracker 360° in azimuth
3. Also move through full elevation range
4. Takes about 2 minutes
5. Hard/soft iron correction calculated

### 3. Verify Calibration

1. Set mode to "Manual"
2. Point tracker to known direction (use compass)
3. Check azimuth reading matches
4. Tilt tracker to known angle (use level)
5. Check elevation reading matches

## Troubleshooting Installation

### Add-on Won't Start

**Check Logs:**
Look for specific error messages in the Log tab.

**Common Issues:**

1. **ESP32 Not Reachable**
   - Verify ESP32 IP address
   - Ping ESP32: `ping 192.168.1.100`
   - Check ESP32 is on same network
   - Try accessing ESP32 web interface directly

2. **MQTT Connection Failed**
   - Verify Mosquitto is running
   - Check MQTT credentials
   - Test MQTT: Developer Tools → MQTT

3. **Invalid Configuration**
   - Check YAML syntax
   - Verify timezone string is valid
   - Ensure latitude/longitude are numbers

### Can't Access Web Interface

1. Verify add-on is running (Info tab shows "Started")
2. Check port 8099 is not blocked by firewall
3. Try accessing from Home Assistant device itself
4. Check logs for web server startup message

### No Sensor Data

1. Test ESP32 API manually:
   ```bash
   curl http://192.168.1.100/api/sensors
   ```

2. Should return JSON with sensor data
3. If not, check ESP32 firmware
4. Verify sensors connected to ESP32

### MQTT Entities Not Appearing

1. Check MQTT integration is enabled in HA
2. Verify Mosquitto is running
3. Check MQTT discovery is enabled:
   - Settings → Devices & Services → MQTT
   - Configure → Enable discovery

4. Manually trigger discovery:
   - Restart the add-on
   - Check logs for "Published MQTT discovery configuration"

## Next Steps

After successful installation:

1. **Test tracking**: Set mode to Auto and verify it tracks the sun
2. **Tune PID**: Adjust PID parameters if tracking is not smooth
3. **Set up automations**: Create HA automations for special conditions
4. **Add dashboard cards**: Display tracker status in Lovelace UI
5. **Monitor performance**: Watch tracking accuracy over several days

## Getting Help

If you encounter issues:

1. Check the logs first
2. Review the README troubleshooting section
3. Search existing GitHub issues
4. Create a new issue with:
   - Add-on version
   - Home Assistant version
   - Complete error logs
   - Configuration (remove passwords)
   - Description of problem

## Advanced Configuration

### Custom PID Tuning

Start with defaults, then adjust based on behavior:

- **Oscillating**: Lower Kp to 0.5, Kd to 0.02
- **Slow response**: Raise Kp to 2.0
- **Steady-state error**: Raise Ki to 0.3
- **Overshoot**: Lower Kp, raise Kd

Change one parameter at a time and test.

### Update Rate

Default is 1.0 second (1 Hz). You can adjust:

- **Faster** (0.5s): More responsive but higher CPU usage
- **Slower** (2.0s): Lower CPU usage but slower response

Most systems work well at 1 Hz.

### Wind Stow Threshold

Set based on your location and structure:

- **50 mph**: Good default for most trackers
- **30 mph**: More conservative (lighter structures)
- **70 mph**: Less conservative (heavy-duty structures)

Consider sustained winds vs gusts in your area.
