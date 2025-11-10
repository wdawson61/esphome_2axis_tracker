# Solar Tracker ESPHome Firmware

ESPHome-based firmware for a 2-axis solar tracker using ESP32-C6, WIT-Motion HWT905 9-axis IMU (via RS485), and H-bridge motor controllers.

## Features

- **Dual-axis tracking**: Elevation (tilt) and Azimuth (rotation) control
- **High-precision sensing**: WIT-Motion HWT905 IMU via RS485 communication
- **Closed-loop control**: Position feedback using pitch/yaw angles
- **Safety features**: Timeouts, emergency stop, angle limits
- **Home Assistant integration**: Full API exposure for automation
- **Sensor calibration**: Built-in calibration service for IMU
- **Real-time monitoring**: Live angle and acceleration data

## Hardware Requirements

### Components
- **ESP32-C6 DevKit** (or compatible)
- **WIT-Motion HWT905** 9-axis IMU sensor
- **SeeedStudio RS485 Breakout Board** (or compatible MAX485/SP3485 module)
- **2x BTS7960 H-bridge motor drivers** (43A high-current motor driver modules)
- **Linear actuator** with 24V DC motor (for elevation control)
- **Slewing drive motor** 24V DC (for azimuth control)
- **24V power supply** (for motors - minimum 10A recommended)
- **5V power supply** (for ESP32 and sensors - 2A minimum)

### Pin Assignments

```
ESP32-C6 Pin    | Function
----------------|---------------------------
GPIO4           | UART TX (to RS485 module)
GPIO5           | UART RX (from RS485 module)
GPIO6           | Elevation Forward
GPIO7           | Elevation Backward
GPIO8           | Azimuth CW (Clockwise)
GPIO9           | Azimuth CCW (Counter-clockwise)
GPIO10          | Azimuth Home Limit Switch
GPIO2           | Status LED
```

## Wiring Diagrams

### RS485 Connection (HWT905)

```
HWT905          SeeedStudio RS485       ESP32-C6
------          -----------------       --------
VCC (5V) -----> VCC (5V)
GND      -----> GND
A        -----> A
B        -----> B
                RO (Receiver)    -----> GPIO5 (RX)
                DI (Driver)      <----- GPIO4 (TX)
                VCC              -----> 5V
                GND              -----> GND

Note: SeeedStudio RS485 board has auto-direction control,
      so no need to manually control DE/RE pins.
      If using a different RS485 module with DE/RE pins,
      tie both DE and RE together to a GPIO or:
      - DE to 3.3V (always transmit enable)
      - RE to GND (always receive enable)
```

### H-Bridge Motor Connections

The BTS7960 is a high-current (43A) dual H-bridge driver. Each driver needs two control signals (RPWM and LPWM) for direction control.

#### Elevation Motor (Linear Actuator)
```
BTS7960 Module  ESP32-C6       Linear Actuator    Power
--------------  --------       ----------------   ------
RPWM (Right)    GPIO6          Motor Wire +       VCC → 24V+
LPWM (Left)     GPIO7          Motor Wire -       GND → 24V-
R_EN            3.3V (enable)                     B+ → 24V+
L_EN            3.3V (enable)                     B- → Motor+
R_IS            Not connected                     Motor connections:
L_IS            Not connected                     - M+ → Motor Wire +
VCC             5V                                - M- → Motor Wire -
GND             GND (Common)

Control Logic:
- Forward:  RPWM=HIGH, LPWM=LOW  (GPIO6=HIGH, GPIO7=LOW)
- Backward: RPWM=LOW,  LPWM=HIGH (GPIO6=LOW,  GPIO7=HIGH)
- Stop:     RPWM=LOW,  LPWM=LOW  (both LOW)
```

#### Azimuth Motor (Slewing Drive)
```
BTS7960 Module  ESP32-C6       Slewing Motor      Power
--------------  --------       --------------     ------
RPWM (Right)    GPIO8          Motor Wire +       VCC → 24V+
LPWM (Left)     GPIO9          Motor Wire -       GND → 24V-
R_EN            3.3V (enable)                     B+ → 24V+
L_EN            3.3V (enable)                     B- → Motor+
R_IS            Not connected                     Motor connections:
L_IS            Not connected                     - M+ → Motor Wire +
VCC             5V                                - M- → Motor Wire -
GND             GND (Common)

Control Logic:
- CW:  RPWM=HIGH, LPWM=LOW  (GPIO8=HIGH, GPIO9=LOW)
- CCW: RPWM=LOW,  LPWM=HIGH (GPIO8=LOW,  GPIO9=HIGH)
- Stop: RPWM=LOW, LPWM=LOW  (both LOW)
```

**Important BTS7960 Notes:**
1. **Enable Pins**: Tie R_EN and L_EN to 3.3V to permanently enable the driver
2. **Power Supply**: Use adequate 24V supply (10A+ recommended for dual motors)
3. **Heat Dissipation**: BTS7960 modules have heat sinks - ensure good ventilation
4. **Current Sensing**: R_IS and L_IS provide current feedback (optional, not used here)
5. **PWM Control**: For variable speed, use PWM on RPWM/LPWM (currently digital on/off)
6. **Protection**: Built-in over-current, over-temperature, and short-circuit protection

### Home Limit Switch Connection

```
Limit Switch    ESP32-C6
------------    --------
COM      -----> GPIO10 (with internal pullup)
NO       -----> GND

Note: Switch is normally open, closes to ground when triggered
The switch should be positioned so it triggers when the azimuth
is at the desired home/zero position (typically facing North or South)
```

## Installation

### 1. Install ESPHome

```bash
pip install esphome
```

### 2. Configure Secrets

Edit `secrets.yaml` with your credentials:

```yaml
wifi_ssid: "YourWiFiSSID"
wifi_password: "YourWiFiPassword"
api_encryption_key: "32-character-key-here"
ota_password: "YourOTAPassword"
ap_password: "FallbackPassword"
```

Generate an API encryption key:
```bash
esphome wizard solar_tracker.yaml
```

### 3. Compile and Upload

```bash
# First time upload via USB
esphome run solar_tracker.yaml

# Subsequent OTA updates
esphome run solar_tracker.yaml --device solar-tracker.local
```

### 4. Add to Home Assistant

After successful upload, the device should be auto-discovered in Home Assistant.
Go to **Settings** > **Devices & Services** > **ESPHome** and add the device.

## Usage

### Services

#### Set Elevation Angle
```yaml
service: esphome.solar_tracker_set_elevation
data:
  angle: 45.5  # degrees (0-90)
```

#### Set Azimuth/Heading
```yaml
service: esphome.solar_tracker_set_azimuth
data:
  angle: 180.0  # degrees (0-360)
```

**Note:** The azimuth axis must be homed first using the `home_azimuth` service.

#### Home Azimuth Axis
Establishes a zero reference position using the limit switch:

```yaml
service: esphome.solar_tracker_home_azimuth
```

**Homing Sequence:**
1. If on the limit switch, backs off clockwise
2. Rotates counter-clockwise to find the limit switch
3. Backs off and slowly approaches the switch
4. Sets the switch position as 0° (home position)
5. All subsequent azimuth commands are relative to this home position

**Important:** Run this service:
- After initial installation
- After power loss
- If position accuracy degrades
- Before any azimuth movements

#### Calibrate Sensor
Perform IMU calibration (required for accurate readings):

```yaml
service: esphome.solar_tracker_calibrate_sensor
```

**Calibration Steps:**
1. Place tracker on level surface
2. Call calibration service
3. Keep device stable for 5 seconds (accelerometer calibration)
4. Slowly rotate device in figure-8 pattern for 15 seconds (magnetometer calibration)
5. Calibration data is automatically saved to sensor

#### Stop Motors
```yaml
service: esphome.solar_tracker_stop_motors
```

#### Emergency Stop
```yaml
service: esphome.solar_tracker_emergency_stop
```

### Sensors

The firmware exposes the following sensors:

- **Elevation Angle** (°): Current tilt angle from HWT905
- **Heading Angle** (°): Current compass heading (0-360°)
- **Acceleration X** (m/s²): X-axis acceleration
- **Acceleration Y** (m/s²): Y-axis acceleration
- **Acceleration Z** (m/s²): Z-axis acceleration

### Automation Examples

#### Sun Tracking (requires sun position calculation)

```yaml
automation:
  - alias: "Track Sun Position"
    trigger:
      - platform: time_pattern
        minutes: "/5"  # Every 5 minutes
    action:
      - service: esphome.solar_tracker_set_azimuth
        data:
          angle: "{{ state_attr('sun.sun', 'azimuth') }}"
      - service: esphome.solar_tracker_set_elevation
        data:
          angle: "{{ state_attr('sun.sun', 'elevation') }}"
```

#### Night Stow Position

```yaml
automation:
  - alias: "Stow at Night"
    trigger:
      - platform: sun
        event: sunset
    action:
      - service: esphome.solar_tracker_set_elevation
        data:
          angle: 0
      - service: esphome.solar_tracker_set_azimuth
        data:
          angle: 180
```

#### Wind Protection

```yaml
automation:
  - alias: "Stow in High Wind"
    trigger:
      - platform: numeric_state
        entity_id: sensor.wind_speed
        above: 40  # km/h
    action:
      - service: esphome.solar_tracker_set_elevation
        data:
          angle: 0  # Horizontal position
```

## Configuration Options

### Tuning Parameters

You can adjust these constants in `solar_tracker.h`:

```cpp
// Angle tolerance (degrees)
const float ELEVATION_TOLERANCE = 0.5;  // Elevation precision
const float AZIMUTH_TOLERANCE = 2.0;    // Azimuth precision

// Timing
const unsigned long MOTOR_TIMEOUT = 120000;        // 2 min max runtime
const unsigned long AZIMUTH_READ_INTERVAL = 500;   // Heading check interval
const unsigned long AZIMUTH_BURST_TIME = 300;      // Motor pulse duration
```

### Angle Limits

Elevation is constrained to 0-90° by default. Modify in `set_elevation()`:

```cpp
target_elevation_ = constrain(target_angle, 0.0f, 90.0f);
```

## Troubleshooting

### No Sensor Data

1. **Check RS485 wiring**: Verify A/B connections and polarity on SeeedStudio board
2. **Check UART pins**: Ensure GPIO4 (TX) and GPIO5 (RX) are correct
3. **Check baud rate**: HWT905 default is 115200
4. **Enable debug logging**: Set logger level to `DEBUG` or `VERBOSE`
5. **Test RS485 board**: SeeedStudio board should have power LED lit

### Motor Not Moving

1. **Check BTS7960 connections**: Verify RPWM/LPWM pins and enable pins
2. **Check motor power**: Ensure 24V supply is connected and adequate (10A+)
3. **Check enable pins**: R_EN and L_EN should be tied to 3.3V
4. **Test individual motors**: Use GPIO test to verify BTS7960 operation
5. **Check heat sinks**: BTS7960 may thermal shutdown if overheating

### Inaccurate Angles

1. **Calibrate sensor**: Run calibration service
2. **Check mounting**: Ensure HWT905 is rigidly mounted to tracker
3. **Check for interference**: Keep away from magnetic sources
4. **Verify orientation**: Ensure sensor axes match tracker axes

### Motors Timeout

1. **Check mechanical resistance**: Ensure free movement
2. **Adjust tolerance**: Increase `ELEVATION_TOLERANCE` or `AZIMUTH_TOLERANCE`
3. **Check sensor alignment**: Verify pitch/yaw correspond to physical movement
4. **Increase timeout**: Adjust `MOTOR_TIMEOUT` if needed

## Protocol Details

### HWT905 Communication

The HWT905 uses a custom serial protocol at 115200 baud:

**Packet Format:**
```
Byte 0: 0x55 (Header)
Byte 1: Packet Type
Byte 2-9: Data (LSB first)
Byte 10: Checksum (sum of bytes 0-9)
```

**Packet Types:**
- `0x51`: Acceleration data
- `0x53`: Angle data (roll, pitch, yaw)
- `0x52`: Gyroscope data
- `0x54`: Magnetometer data

**Calibration Commands:**
```
Unlock: 0xFF 0xAA 0x69 0xB5 0x88
Accel Cal: 0xFF 0xAA 0x01 0x01 0x00
Mag Cal: 0xFF 0xAA 0x01 0x07 0x00
Exit Cal: 0xFF 0xAA 0x01 0x00 0x00
Save: 0xFF 0xAA 0x00 0x00 0x00
```

## Safety Features

1. **Motor Timeout**: Motors automatically stop after 2 minutes
2. **Emergency Stop**: Immediately halts all motor activity
3. **Angle Limits**: Software limits prevent over-extension
4. **Closed-Loop Control**: Constant feedback prevents runaway
5. **Burst Mode**: Azimuth motor runs in pulses to prevent overshooting

## Advanced Configuration

### PWM Motor Control

For variable speed control, modify the motor functions to use PWM:

```cpp
// In setup():
ledcSetup(0, 5000, 8);  // 5kHz, 8-bit resolution
ledcAttachPin(elevation_forward_pin_, 0);

// In run_elevation_forward():
ledcWrite(0, 200);  // 78% duty cycle
```

### Custom Sensor Fusion

Add additional sensors (GPS, compass) for improved accuracy:

```cpp
float fused_heading = (hwt905_heading * 0.7) + (gps_heading * 0.3);
```

### Remote Monitoring

Add MQTT support for cloud monitoring:

```yaml
mqtt:
  broker: your.mqtt.broker
  username: !secret mqtt_username
  password: !secret mqtt_password
```

## License

This firmware is provided as-is for solar tracker applications.
Modify and distribute freely for non-commercial use.

## Support

For issues and questions:
- Check ESPHome documentation: https://esphome.io
- HWT905 datasheet: [WIT-Motion website]
- Create an issue in your project repository

## Version History

- **v1.0** (2024-11): Initial release
  - Dual-axis control
  - HWT905 integration
  - Closed-loop position control
  - Home Assistant services
