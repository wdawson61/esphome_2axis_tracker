# Solar Tracker Debugging and Diagnostics Guide

## Quick Diagnostics Checklist

### Power & Connectivity
- [ ] ESP32-C6 powered and LED blinking
- [ ] HWT905 sensor powered (LED indicator on sensor)
- [ ] Motor power supply connected (12V)
- [ ] WiFi connected (check logs for IP address)
- [ ] Home Assistant API connected

### Hardware Connections
- [ ] RS485 A+/B- properly connected (not reversed)
- [ ] UART TX/RX pins correct (GPIO4/GPIO5)
- [ ] Motor H-bridge inputs connected to correct GPIOs
- [ ] All ground connections common
- [ ] No loose connections

### Firmware
- [ ] ESPHome compilation successful
- [ ] Upload completed without errors
- [ ] Device appears in Home Assistant
- [ ] All sensors showing data (not "unavailable")

---

## Detailed Troubleshooting

### Issue: No Sensor Data from HWT905

**Symptoms:**
- Elevation and heading sensors show "unavailable"
- No acceleration data
- UART errors in logs

**Diagnostics:**

1. **Check UART Communication**
```
# In ESPHome logs, look for:
[D][uart_debug:114]: >>> FF AA 01 07 00  (commands being sent)
[D][uart_debug:114]: <<< 55 53 ...       (data being received)
```

2. **Verify RS485 Connections**
```
HWT905 Pin → SeeedStudio RS485 Board
A          → A (or B, some boards labeled differently)
B          → B (or A, depending on labeling)

RS485 Board → ESP32-C6
RO         → GPIO5 (RX)
DI         → GPIO4 (TX)
VCC        → 5V
GND        → GND

Common mistake: A/B reversed - swap them if no data
Note: SeeedStudio board has auto-direction control,
      no need to control DE/RE pins manually
```

3. **Check Baud Rate**
```
# HWT905 default is 115200
# Verify in solar_tracker.yaml:
uart:
  baud_rate: 115200
```

4. **Test with USB-TTL Adapter**
```
# Connect HWT905 directly to computer via USB-TTL
# Use serial terminal (115200 baud) to verify sensor output
# Should see binary packets starting with 0x55
```

5. **Enable Verbose Logging**
```yaml
# Add to solar_tracker.yaml:
logger:
  level: VERBOSE
  logs:
    uart: VERBOSE
```

**Solutions:**
- Swap A+ and B- if wiring was incorrect
- Check for damaged RS485 module
- Verify HWT905 is configured for UART mode (not I2C)
- Add pull-up/pull-down resistors if signal integrity issues
- Try lower baud rate (9600) for testing

---

### Issue: Sensor Data Inaccurate or Drifting

**Symptoms:**
- Angles don't match physical position
- Values drift over time
- Heading jumps erratically

**Diagnostics:**

1. **Check Calibration Status**
```
# Run calibration service:
service: esphome.solar_tracker_calibrate_sensor

# Monitor logs for calibration sequence
[I][HWT905:XXX]: Starting calibration sequence...
[I][HWT905:XXX]: Calibrating accelerometer...
[I][HWT905:XXX]: Calibrating magnetometer...
[I][HWT905:XXX]: Calibration complete!
```

2. **Verify Sensor Mounting**
- Sensor must be rigidly mounted to tracker frame
- Any flex or vibration causes errors
- Check that sensor axes align with tracker axes

3. **Check for Magnetic Interference**
- Keep HWT905 away from motors (>10cm)
- Avoid metal objects near sensor
- Test heading in different locations

4. **Monitor Raw Data**
```yaml
# Add to solar_tracker.yaml for debugging:
text_sensor:
  - platform: template
    name: "HWT905 Raw Debug"
    lambda: |-
      return str_sprintf("Roll:%.1f Pitch:%.1f Yaw:%.1f", 
        id(elevation_angle).state, ...);
```

**Solutions:**
- Perform calibration in final mounted position
- Use vibration-damping mounting
- Shield sensor from EMI with aluminum case
- Update HWT905 firmware if old version
- Replace sensor if hardware fault detected

---

### Issue: Motors Not Moving

**Symptoms:**
- Service calls accepted but no movement
- Logs show commands sent but no motor activity
- Motors hum but don't turn

**Diagnostics:**

1. **Check Motor Power**
```
# Measure voltage at BTS7960 motor terminals (M+/M-)
# Should be ~24V when motor command active
# Check B+ terminal has 24V supply
```

2. **Test GPIOs Directly**
```yaml
# Add temporary switches for manual control:
switch:
  - platform: gpio
    pin: GPIO6
    name: "Test Elev Forward (RPWM)"
  - platform: gpio
    pin: GPIO7
    name: "Test Elev Backward (LPWM)"
```

3. **Check BTS7960 Enable Pins**
```
# BTS7960 requires R_EN and L_EN to be HIGH (3.3V)
# Verify both enable pins are tied to 3.3V
# Check with multimeter: should read ~3.3V
```

4. **Check BTS7960 Power LED**
```
# Most BTS7960 modules have power indicator LED
# Should be lit when 24V supply connected
# If not lit, check 24V supply and connections
```

5. **Monitor Motor Commands in Logs**
```
[D][MotorController:XXX]: Setting elevation to 45.00°
[D][MotorController:XXX]: Running elevation forward
[D][MotorController:XXX]: Elevation: Current=10.00°, Target=45.00°
```

6. **Test Motor Directly**
```
# Bypass ESP32 - connect motor directly to 24V supply
# Verify motor and BTS7960 are functional
# Check motor current draw doesn't exceed supply rating
```

**Solutions:**
- Verify BTS7960 RPWM/LPWM connections match schematic
- Check that motor power supply is adequate (10A+ recommended)
- Ensure R_EN and L_EN pins are tied to 3.3V (not floating)
- Check for burnt BTS7960 components (rare, has protection)
- Verify GPIO pins are not conflicting with other peripherals
- Ensure adequate cooling for BTS7960 heat sinks
- Check motor current draw is within BTS7960 rating (43A max)

---

### Issue: Motors Run But Don't Stop

**Symptoms:**
- Motors reach target but keep running
- Timeout occurs before stopping
- Position never stabilizes

**Diagnostics:**

1. **Check Feedback Loop**
```
# Enable verbose motor logging:
logger:
  logs:
    component: VERBOSE

# Look for position updates:
[V][MotorController:XXX]: Elevation: Current=44.5°, Target=45.0°, Error=0.5°
[V][MotorController:XXX]: Elevation: Current=45.2°, Target=45.0°, Error=-0.2°
```

2. **Verify Tolerance Settings**
```cpp
// In solar_tracker.h:
const float ELEVATION_TOLERANCE = 0.5;  // Too tight?
const float AZIMUTH_TOLERANCE = 2.0;    // Try increasing
```

3. **Check Sensor Update Rate**
```
# Sensor should update every 100ms (PollingComponent interval)
# If updates are slow, increase timeout or reduce tolerance
```

**Solutions:**
- Increase tolerance values for less precise tracking
- Reduce motor speed if overshooting target
- Add damping to mechanical system
- Implement PID control for smoother approach
- Check for mechanical binding or resistance

---

### Issue: Azimuth Movement Incorrect Direction

**Symptoms:**
- Tracker rotates opposite of intended direction
- Heading increases when should decrease
- Overshoots target consistently

**Diagnostics:**

1. **Test Direction Mapping**
```yaml
# Call services manually and observe:
service: esphome.solar_tracker_set_azimuth
data:
  angle: 90  # Should rotate to East
```

2. **Check Yaw Convention**
```
# HWT905 yaw angles:
# 0° = initial orientation
# Positive rotation = clockwise (typically)
# Negative rotation = counter-clockwise

# Verify sensor convention matches code expectations
```

3. **Test Motor Directions**
```
# Manually test CW/CCW pins:
# CW should rotate clockwise when viewed from above
# CCW should rotate counter-clockwise
```

**Solutions:**
- Swap GPIO pins 8 and 9 if directions reversed
- Modify azimuth error calculation sign
- Remount sensor with correct orientation
- Update code to match your mechanical convention

---

### Issue: Homing Fails or Times Out

**Symptoms:**
- Homing sequence times out after 3 minutes
- Motor doesn't stop at limit switch
- Error: "Failed to find home switch"
- Azimuth movements not allowed

**Diagnostics:**

1. **Test Limit Switch**
```
# Check switch state in Home Assistant
sensor.azimuth_home_switch

# Should show:
# - "off" when switch is open (not triggered)
# - "on" when switch is closed (triggered)
```

2. **Manual Switch Test**
```
# Manually press the switch and observe state change
# If no change, check wiring
```

3. **Check Switch Wiring**
```
Limit Switch    ESP32-C6
------------    --------
COM      -----> GPIO10
NO       -----> GND

# Switch should be normally open (NO)
# Verify with multimeter: open circuit when not pressed
```

4. **Verify Pullup Resistor**
```
# GPIO10 should have internal pullup enabled
# Voltage should be ~3.3V when switch is open
```

5. **Test Motor Direction**
```
# During homing, motor should move CCW
# If moving CW, homing will never find switch
# Verify switch is positioned in CCW direction
```

**Solutions:**
- Verify switch wiring (COM to GPIO10, NO to GND)
- Check switch is mechanically functional
- Ensure switch is positioned in CCW direction
- Replace faulty switch
- Increase HOMING_TIMEOUT if needed

---

### Issue: Azimuth Commands Rejected

**Symptoms:**
- Error: "Azimuth not homed - please run home_azimuth() first"
- set_azimuth service does nothing

**Solution:**
Run homing first: `service: esphome.solar_tracker_home_azimuth`

---

### Issue: Compilation Errors

**Common Errors:**

1. **"Component not found"**
```
# Solution: Ensure solar_tracker.h is in same directory
# Or add full path in includes:
esphome:
  includes:
    - /path/to/solar_tracker.h
```

2. **"UART component not found"**
```
# Solution: Ensure UART is defined before custom component:
uart:
  id: hwt905_uart
  # ... config
```

3. **"GPIO conflict"**
```
# Solution: Each GPIO can only be used once
# Check for duplicate pin assignments
```

4. **ESP32-C6 specific errors**
```
# Solution: Ensure platformio options are correct:
platformio_options:
  board_build.mcu: esp32c6
  board_build.variant: esp32c6
```

---

## Performance Optimization

### Reduce Update Latency

```cpp
// In solar_tracker.h, HWT905Sensor constructor:
HWT905Sensor(uart::UARTComponent *parent) 
  : PollingComponent(50), uart::UARTDevice(parent)  // 50ms instead of 100ms
```

### Improve Azimuth Accuracy

```cpp
// Reduce burst time for finer control:
const unsigned long AZIMUTH_BURST_TIME = 200;  // 200ms instead of 300ms
```

### Add Backlash Compensation

```cpp
// In update_azimuth_movement():
if (error > 0 && last_direction_ < 0) {
  // Reverse direction - add extra pulse for backlash
  run_azimuth_burst_cw();
  delay(100);  // Backlash compensation
}
```

---

## Advanced Debugging

### Enable UART Debug Output

```yaml
uart:
  id: hwt905_uart
  tx_pin: GPIO4
  rx_pin: GPIO5
  baud_rate: 115200
  debug:
    direction: BOTH
    dummy_receiver: false
    after:
      delimiter: "\n"
    sequence:
      - lambda: UARTDebug::log_hex(direction, bytes, ':');
```

### Add Position History Logging

```cpp
// In solar_tracker.h, add member variables:
std::vector<float> elevation_history_;
std::vector<float> azimuth_history_;

// In update() methods:
elevation_history_.push_back(current_elevation);
if (elevation_history_.size() > 100) {
  elevation_history_.erase(elevation_history_.begin());
}

// Log for analysis:
ESP_LOGD("History", "Elevation trend: %.2f -> %.2f", 
  elevation_history_.front(), elevation_history_.back());
```

### Dump Configuration

```yaml
# In Home Assistant Developer Tools > Services:
service: esphome.solar_tracker_dump_config
```

---

## Log Analysis

### Normal Operation Logs

```
[I][app:102]: ESPHome version 2024.10.0
[C][wifi:558]: WiFi connected
[C][api:139]: API Server running on 192.168.1.XXX
[I][HWT905:45]: Setting up HWT905 sensor...
[D][HWT905:78]: Angles: Roll=0.12°, Pitch(Elev)=30.45°, Yaw(Head)=182.30°
[I][MotorController:23]: Setting elevation to 45.00°
[D][MotorController:145]: Elevation: Current=30.45°, Target=45.00°, Error=14.55°
[D][MotorController:145]: Elevation: Current=38.20°, Target=45.00°, Error=6.80°
[D][MotorController:145]: Elevation: Current=44.85°, Target=45.00°, Error=0.15°
[I][MotorController:155]: Elevation target reached: 44.95°
```

### Error Indicators

```
[W][HWT905:89]: Checksum failed  
# → RS485 signal integrity issue

[W][MotorController:201]: Elevation motor timeout - stopping
# → Mechanical binding or sensor failure

[E][uart:015]: Reading from UART timed out
# → No data from HWT905 - check wiring

[E][api:089]: Client disconnected
# → Home Assistant connection lost
```

---

## Safety Checks

### Pre-Operation Checklist

1. **Mechanical:**
   - [ ] All bolts tight
   - [ ] No obstructions in movement path
   - [ ] Linear actuator travel limits set
   - [ ] Slewing bearing lubricated

2. **Electrical:**
   - [ ] All connections secure
   - [ ] Motor power fused appropriately
   - [ ] Ground connections solid
   - [ ] No exposed wiring

3. **Software:**
   - [ ] Angle limits configured correctly
   - [ ] Emergency stop tested
   - [ ] Timeout values appropriate
   - [ ] Calibration completed

### Emergency Stop Procedure

```yaml
# Create dashboard button for emergency stop:
type: button
tap_action:
  action: call-service
  service: esphome.solar_tracker_emergency_stop
name: EMERGENCY STOP
icon: mdi:alert-octagon
```

---

## Performance Benchmarks

### Expected Response Times

- **Service call to motor start:** <100ms
- **Elevation movement (45°):** 10-30 seconds
- **Azimuth movement (90°):** 15-60 seconds
- **Sensor update rate:** 10 Hz (every 100ms)
- **Position tolerance:** ±0.5° elevation, ±2° azimuth

### Accuracy Expectations

- **Elevation accuracy:** ±0.5° (with calibration)
- **Azimuth accuracy:** ±2° (magnetometer dependent)
- **Repeatability:** ±1° (mechanical backlash dependent)
- **Drift:** <1°/hour after warmup

---

## Getting Help

If you've exhausted this guide:

1. **Collect diagnostic info:**
   - Full ESPHome logs (with VERBOSE level)
   - Photos of wiring
   - Video of issue occurring
   - Home Assistant device page screenshot

2. **Check resources:**
   - ESPHome Discord
   - Home Assistant forums
   - WIT-Motion support site

3. **Common gotchas:**
   - RS485 A/B reversed (most common)
   - Wrong baud rate
   - Insufficient motor power supply current
   - Magnetic interference on HWT905
   - GPIO conflicts with ESP32-C6 defaults
