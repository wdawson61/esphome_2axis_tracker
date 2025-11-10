# Solar Tracker Quick Start Guide

## 🚀 5-Minute Setup

### Step 1: Hardware Assembly (15 minutes)

1. **Mount HWT905 sensor** rigidly to tracker frame
2. **Connect SeeedStudio RS485 board:**
   ```
   HWT905  →  RS485 Board  →  ESP32-C6
   A       →  A
   B       →  B
              RO          →  GPIO5 (RX)
              DI          ←  GPIO4 (TX)
              VCC         →  5V
              GND         →  GND
   ```
3. **Connect BTS7960 motor drivers:**
   - Elevation: RPWM→GPIO6, LPWM→GPIO7, R_EN/L_EN→3.3V
   - Azimuth: RPWM→GPIO8, LPWM→GPIO9, R_EN/L_EN→3.3V
4. **Connect home limit switch:**
   - Switch COM → GPIO10
   - Switch NO → GND
   - Position switch at desired home position (0°)
5. **Power connections:**
   - 24V (10A+) to both BTS7960 VCC/B+ terminals
   - 5V to BTS7960 logic VCC and HWT905
   - Common GND for all components
   - USB-C to ESP32-C6 (for programming)

### Step 2: Software Setup (5 minutes)

```bash
# Install ESPHome
pip3 install esphome

# Edit secrets.yaml
nano secrets.yaml
# Add your WiFi credentials

# Compile and upload (first time via USB)
esphome run solar_tracker.yaml
```

### Step 3: First Boot (2 minutes)

1. **Watch logs** for successful boot
2. **Check WiFi** connection (IP address shown in logs)
3. **Add to Home Assistant:**
   - Settings → Devices & Services → ESPHome
   - Device should auto-discover

### Step 4: Calibration (1 minute)

```yaml
# In Home Assistant Developer Tools → Services
service: esphome.solar_tracker_calibrate_sensor
```

**During calibration:**
1. Keep tracker level and stable (5 seconds)
2. Rotate tracker in figure-8 pattern (15 seconds)
3. Done! Calibration saved automatically.

### Step 5: Home the Azimuth (2 minutes)

**IMPORTANT:** You must home the azimuth axis before any azimuth movements!

```yaml
# In Home Assistant Developer Tools → Services
service: esphome.solar_tracker_home_azimuth
```

**What happens:**
1. Motor rotates counter-clockwise searching for limit switch
2. When found, backs off and slowly approaches
3. Sets that position as 0° (home)
4. Takes 1-3 minutes depending on starting position

**Watch the logs** to see homing progress.

### Step 6: Test Movement (2 minutes)

```yaml
# Test elevation
service: esphome.solar_tracker_set_elevation
data:
  angle: 45

# Wait for movement to complete, then test azimuth
service: esphome.solar_tracker_set_azimuth
data:
  angle: 180
```

---

## ✅ Verification Checklist

After setup, verify:

- [ ] **Sensors showing data** (not "unavailable")
  - Elevation Angle: ~0-90°
  - Heading Angle: 0-360°
  - Acceleration X/Y/Z: values present
  - Azimuth Home Switch: on/off state

- [ ] **Homing successful**
  - home_azimuth service completes
  - Motor stops at limit switch
  - No timeout errors

- [ ] **Motors respond to commands**
  - Elevation increases when set higher
  - Azimuth rotates in correct direction
  - Motors stop when target reached

- [ ] **Tracking works**
  - Set elevation to sun's elevation
  - Set azimuth to sun's azimuth
  - Tracker should point at sun

---

## 🎯 Common First-Time Issues

### No Sensor Data
→ **Swap RS485 A and B on SeeedStudio board** (most common issue!)

### Motors Don't Move
→ Check BTS7960 enable pins tied to 3.3V and 24V power supply connected

### Wrong Direction
→ Swap motor GPIO pins (6↔7 for elevation, 8↔9 for azimuth)

### Can't Connect to WiFi
→ Check secrets.yaml WiFi credentials

---

## 📊 Expected Readings

After successful setup:

| Sensor | Typical Range | Notes |
|--------|--------------|-------|
| Elevation | 0-90° | Horizontal = 0°, Vertical = 90° |
| Heading | 0-360° | North = 0°, East = 90°, South = 180°, West = 270° |
| Accel X | ±16 m/s² | Should be near 0 when level |
| Accel Y | ±16 m/s² | Should be near 0 when level |
| Accel Z | ~9.8 m/s² | Should be near gravity when level |

---

## 🔧 Essential Services

### Position Control
```yaml
# FIRST: Home the azimuth axis
service: esphome.solar_tracker_home_azimuth

# Then set specific angles
service: esphome.solar_tracker_set_elevation
data:
  angle: 45.0

service: esphome.solar_tracker_set_azimuth
data:
  angle: 180.0
```

### Safety
```yaml
# Stop everything
service: esphome.solar_tracker_stop_motors

# Emergency stop (requires manual reset)
service: esphome.solar_tracker_emergency_stop
```

### Maintenance
```yaml
# Recalibrate sensors
service: esphome.solar_tracker_calibrate_sensor
```

---

## 🌤️ Enable Sun Tracking

Add to Home Assistant `configuration.yaml`:

```yaml
automation:
  - alias: "Track Sun Every 5 Minutes"
    trigger:
      - platform: time_pattern
        minutes: "/5"
    condition:
      - condition: sun
        after: sunrise
        before: sunset
    action:
      - service: esphome.solar_tracker_set_azimuth
        data:
          angle: "{{ state_attr('sun.sun', 'azimuth') }}"
      - delay: 2
      - service: esphome.solar_tracker_set_elevation
        data:
          angle: "{{ state_attr('sun.sun', 'elevation') }}"
```

Restart Home Assistant, and tracking begins automatically!

---

## 📱 Dashboard Card

Add to Lovelace dashboard:

```yaml
type: entities
title: Solar Tracker
entities:
  - entity: sensor.elevation_angle
    name: Current Elevation
  - entity: sensor.heading_angle
    name: Current Heading
  - entity: number.target_elevation
    name: Target Elevation
  - entity: number.target_azimuth
    name: Target Azimuth
  - entity: binary_sensor.elevation_motor_active
    name: Elevation Motor
  - entity: binary_sensor.azimuth_motor_active
    name: Azimuth Motor
```

---

## 🆘 Emergency Procedures

### Runaway Motors
1. Press **Emergency Stop** in Home Assistant
2. Or disconnect **12V power** to motors
3. Or press **physical reset** on ESP32

### Sensor Failure
1. Motors will timeout after 2 minutes
2. Check sensor LED (should be lit)
3. Reconnect RS485 wiring
4. Restart ESP32 if needed

### Network Loss
1. Tracker continues last command until complete
2. Automatic reconnection will occur
3. Or power cycle ESP32 to reset

---

## 🎓 Next Steps

Once basic tracking works:

1. **Add weather protection** (wind, snow automations)
2. **Fine-tune tolerances** for your hardware
3. **Enable night stow** position
4. **Set up notifications** for alerts
5. **Log performance data** for analysis

See `README.md` for complete documentation.
See `DEBUGGING.md` for troubleshooting.
See `home_assistant_automations.yaml` for advanced automations.

---

## 📞 Need Help?

1. Check `DEBUGGING.md` - comprehensive troubleshooting guide
2. Review logs: `esphome logs solar_tracker.yaml`
3. Test hardware: `python3 test_firmware.py`
4. Join ESPHome Discord or Home Assistant forums

---

## ⚠️ Safety Reminders

- Ensure **clear movement path** before testing
- Set **angle limits** appropriate for your hardware
- Test **emergency stop** before leaving unattended
- Install **physical limit switches** if possible
- Never exceed **safe wind speeds** for your structure

---

**You're ready! Happy tracking! ☀️**
