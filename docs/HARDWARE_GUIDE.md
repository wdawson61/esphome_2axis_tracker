# Solar Tracker Complete Hardware Guide

## 🏗️ System Architecture

The solar tracker system consists of **TWO separate components** that work together:

```
┌─────────────────────────────────────┐
│   HOME ASSISTANT SERVER             │
│   (Raspberry Pi, NUC, etc.)         │
│                                     │
│   ┌─────────────────────────────┐   │
│   │  Solar Tracker Add-on       │   │
│   │  (Python Application)       │   │
│   │                             │   │
│   │  • Sun position calc        │   │
│   │  • Sensor fusion            │   │
│   │  • PID control              │   │
│   │  • Web interface            │   │
│   │  • Wind from HA entities    │   │
│   └──────────┬──────────────────┘   │
│              │ HTTP REST API        │
└──────────────┼──────────────────────┘
               │
               ↓ WiFi
┌──────────────────────────────────────┐
│   ESP32 HARDWARE CONTROLLER          │
│   (On the solar tracker structure)  │
│                                      │
│   • Reads 9-axis IMU sensor          │
│   • Controls azimuth motor           │
│   • Controls elevation motor         │
│   • Monitors limit switches          │
│   • Optional: RS485/Modbus           │
│   • Provides HTTP API                │
└──────────────────────────────────────┘
```

## 📦 Two Codebases

### 1. Home Assistant Add-on (Python)
**Location**: `solar_tracker_addon/`
**Runs on**: Home Assistant server
**Purpose**: Intelligence and control algorithms
**Files**: 
- Python code (controller.py, sensor_fusion.py, etc.)
- Web dashboard
- Configuration

### 2. ESP32 Firmware (C++/Arduino or ESPHome)
**Location**: `Solar_Tracker_ESP32_Hardware.ino` or `solar_tracker_esphome.yaml`
**Runs on**: ESP32 microcontroller
**Purpose**: Hardware interface and motor control
**Files**:
- Arduino sketch (.ino) OR
- ESPHome YAML config

## 🔌 Hardware Requirements

### Required Components

#### 1. Home Assistant Server
- **Options**: Raspberry Pi 4, Intel NUC, VM, Home Assistant Blue/Yellow
- **RAM**: 2GB minimum, 4GB+ recommended
- **Storage**: 32GB+ SD card or SSD
- **Network**: Ethernet or WiFi

#### 2. ESP32 Development Board
**Choose ONE:**
- ✅ ESP32-WROOM-32 DevKit V1 (recommended for beginners)
- ✅ ESP32-C3 (lower cost, fewer pins)
- ✅ ESP32-C6 (newer, WiFi 6)
- ✅ Seeed XIAO ESP32-C6 (tiny, advanced users)

#### 3. 9-Axis IMU Sensor
**Choose ONE:**
- ✅ MPU9250 breakout board (recommended)
- ✅ MPU9255 (similar to MPU9250)
- ✅ ICM-20948 (newer, better performance)
- ✅ BMI088 + QMC5883L (separate accel/gyro + mag)

**Specifications:**
- I2C interface
- 3.3V or 5V compatible
- Accelerometer + Gyroscope + Magnetometer

#### 4. Motor Drivers
**Choose based on your motors:**

**For DC motors:**
- ✅ L298N H-Bridge (up to 2A per channel)
- ✅ BTS7960 (up to 43A, high power)
- ✅ Cytron MD10C (13A)

**For stepper motors:**
- ✅ A4988 driver
- ✅ DRV8825 driver
- ✅ TMC2208 (quieter)

**Quantity**: 2 drivers (one for azimuth, one for elevation)

#### 5. Motors
**Azimuth motor** (rotation):
- High torque (recommend 50+ Nm)
- Geared DC motor or stepper
- 12V or 24V

**Elevation motor** (tilt):
- Medium torque (recommend 20+ Nm)
- Linear actuator OR geared motor
- 12V or 24V

#### 6. Limit Switches
**Quantity**: 4 switches
- 2 for azimuth (CW and CCW limits)
- 2 for elevation (UP and DOWN limits)
- Type: Mechanical microswitches, normally-open
- Rating: 5A+ at 12V

#### 7. Power Supply
**12V or 24V DC power supply:**
- Current: Based on motor specs (typically 5-20A)
- Voltage: Match your motors
- Recommendation: 12V 10A for small tracker

**5V for ESP32:**
- Buck converter (12V/24V → 5V) OR
- Separate 5V power supply (2A)

### Optional Components

#### 8. RS485 Module (Optional)
**For Modbus devices:**
- MAX485 or SP485 module
- Twisted pair cable
- Needed only if using RS485 sensors

#### 9. Enclosures
- Weatherproof box for ESP32 and drivers
- IP65 rated recommended
- Heat management for motor drivers

#### 10. Additional Sensors (Optional)
- Wind sensor (if not using Ecowitt)
- Rain sensor
- Vibration sensor

## 🔧 Wiring Diagrams

### Complete System Wiring

```
┌──────────────────────────────────────────────────────────────┐
│                         POWER SUPPLY                         │
│                    12V/24V DC (10A+)                        │
└───┬──────────────────┬────────────────────┬─────────────────┘
    │                  │                    │
    │                  │                    │
    ↓ 12V/24V          ↓ 12V/24V           ↓ 12V → 5V
┌───────────┐     ┌───────────┐      ┌──────────────┐
│ Azimuth   │     │ Elevation │      │ Buck         │
│ Motor     │     │ Motor     │      │ Converter    │
│ Driver    │     │ Driver    │      │ (12V→5V)     │
└─────┬─────┘     └─────┬─────┘      └───┬──────────┘
      │                 │                  │ 5V
      │                 │                  ↓
      │ PWM+DIR         │ PWM+DIR    ┌─────────────────┐
      └────────┬────────┴────────────│   ESP32-WROOM   │
               │                     │                 │
               │                     │  GPIO21 ─┐      │
               │                     │  GPIO22 ─┤ I2C  │
               │                     │          │      │
               │                     │  GPIO25 ──→ Az DIR
               │                     │  GPIO26 ──→ Az PWM
               │                     │  GPIO27 ──→ El DIR
               │                     │  GPIO14 ──→ El PWM
               │                     │                 │
               │                     │  GPIO32 ← Az CW Limit
               │                     │  GPIO33 ← Az CCW Limit
               │                     │  GPIO34 ← El UP Limit
               │                     │  GPIO35 ← El DOWN Limit
               │                     │                 │
               │                     │  5V ─────────┐  │
               │                     │  GND ────────┤  │
               │                     └──────────────┼──┘
               │                                    │
               │                                    │
               └────────────────────────────────────┤
                                                    │
                                              ┌─────┴──────┐
                                              │  MPU9250   │
                                              │  IMU       │
                                              │            │
                                              │  SDA ← GPIO21
                                              │  SCL ← GPIO22
                                              │  VCC ← 3.3V
                                              │  GND ← GND
                                              └────────────┘
```

### Pin Assignments (ESP32-WROOM-32)

| Function | GPIO | Connection | Notes |
|----------|------|------------|-------|
| **I2C SDA** | 21 | MPU9250 SDA | IMU data |
| **I2C SCL** | 22 | MPU9250 SCL | IMU clock |
| **Azimuth DIR** | 25 | Motor driver DIR | Direction control |
| **Azimuth PWM** | 26 | Motor driver PWM | Speed control |
| **Elevation DIR** | 27 | Motor driver DIR | Direction control |
| **Elevation PWM** | 14 | Motor driver PWM | Speed control |
| **Az CW Limit** | 32 | Limit switch | Input, pull-up |
| **Az CCW Limit** | 33 | Limit switch | Input, pull-up |
| **El UP Limit** | 34 | Limit switch | Input, pull-up |
| **El DOWN Limit** | 35 | Limit switch | Input, pull-up |
| **Power** | 5V | Buck converter | ESP32 power |
| **Ground** | GND | Common ground | All grounds |

### Motor Driver Connections (L298N Example)

```
ESP32           L298N Driver        Motor
─────           ────────────        ─────
GPIO25 ──→ IN1  │
GPIO26 ──→ ENA  │  OUT1 ──────────→ Motor+
                │  OUT2 ──────────→ Motor-
GND ──────→ GND │
                │
12V ───────→ 12V (motor power)
5V ────────→ 5V  (logic power, or use onboard regulator)
```

### Limit Switch Wiring

```
ESP32           Limit Switch        Details
─────           ────────────        ───────
GPIO32 ────────→ COM (normally open)
3.3V ──────────→ NO contact
                 When closed: GPIO32 → HIGH
                 Internal pull-down in code
```

## 🎯 Two Setup Options

### Option A: Arduino Firmware (Recommended for Beginners)

**File**: `Solar_Tracker_ESP32_Hardware.ino`

**Pros:**
- ✅ Complete control
- ✅ Standalone operation
- ✅ Easy to customize
- ✅ No dependencies

**Setup:**
1. Install Arduino IDE
2. Install ESP32 board support
3. Install libraries: `ArduinoJson`, `Wire`
4. Open `.ino` file
5. Configure WiFi credentials
6. Adjust pin numbers if needed
7. Upload to ESP32

### Option B: ESPHome (Recommended for HA Integration)

**File**: `solar_tracker_esphome.yaml`

**Pros:**
- ✅ Native HA integration
- ✅ Easy updates via HA
- ✅ Automatic entity creation
- ✅ Web-based configuration

**Cons:**
- ❌ Requires ESPHome add-on
- ❌ Less customizable
- ❌ Magnetometer needs custom component

**Setup:**
1. Install ESPHome add-on in HA
2. Create new device
3. Copy YAML config
4. Configure secrets (WiFi, API key)
5. Compile and upload

## 🚀 Complete Installation Process

### Step 1: Prepare Home Assistant
1. Install Home Assistant (if not already)
2. Install Mosquitto MQTT broker add-on
3. Configure MQTT integration

### Step 2: Install Solar Tracker Add-on
1. Add repository to add-on store
2. Install Solar Tracker Controller add-on
3. Configure (ESP32 IP, location, etc.)
4. Don't start yet - need ESP32 first

### Step 3: Set Up ESP32 Hardware
1. Wire ESP32 according to diagram
2. Connect IMU sensor to I2C pins
3. Connect motor drivers to PWM pins
4. Wire limit switches to input pins
5. Connect power supply

### Step 4: Upload ESP32 Firmware

**Arduino Method:**
```bash
1. Open Solar_Tracker_ESP32_Hardware.ino
2. Change WiFi credentials
3. Verify pin assignments match wiring
4. Select Board: ESP32 Dev Module
5. Select Port
6. Click Upload
7. Open Serial Monitor
8. Note IP address
```

**ESPHome Method:**
```bash
1. ESPHome Dashboard → New Device
2. Copy solar_tracker_esphome.yaml
3. Edit secrets.yaml with WiFi
4. Compile
5. Upload via USB first time
6. OTA updates after that
```

### Step 5: Test ESP32
1. Open browser to ESP32 IP
2. Test API: `http://ESP_IP/api/sensors`
3. Should see JSON with IMU data
4. Test: `http://ESP_IP/api/limits`
5. Verify limit switch states

### Step 6: Configure Add-on
```yaml
esp32_ip: "192.168.1.XXX"  # Use IP from Step 4
esp32_port: 80
latitude: YOUR_LAT
longitude: YOUR_LONG
timezone: "America/New_York"
```

### Step 7: Start Add-on
1. Start Solar Tracker Controller add-on
2. Check logs for successful connection
3. Open web interface: `http://HA_IP:8099`
4. Verify sensor data displays

### Step 8: Calibrate
1. Set mode to "Calibrate"
2. Keep tracker stationary for gyro cal
3. Rotate 360° for magnetometer cal
4. Test in manual mode
5. Enable auto mode

## 🔍 Troubleshooting

### ESP32 Won't Connect to WiFi
- Check SSID/password
- Verify 2.4GHz network (not 5GHz)
- Check router settings
- Try hotspot mode

### No IMU Data
- Check I2C wiring (SDA/SCL)
- Verify 3.3V power to IMU
- Check I2C address (0x68 or 0x69)
- Run I2C scanner

### Motors Not Moving
- Check motor driver connections
- Verify PWM signals (oscilloscope or LED)
- Check motor power supply
- Test motor driver independently

### Add-on Can't Connect to ESP32
- Ping ESP32 IP
- Check firewall rules
- Verify ESP32 web server running
- Test with curl: `curl http://ESP_IP/status`

### Limit Switches Not Working
- Check wiring (COM to GPIO, NO to 3.3V)
- Test with multimeter
- Verify pull-up/pull-down configuration
- Check switch polarity

## 📋 Pre-Flight Checklist

Before enabling auto mode:

- [ ] ESP32 has stable WiFi
- [ ] IMU reading correctly
- [ ] Motors respond to commands
- [ ] Limit switches tested
- [ ] Manual mode works
- [ ] PID parameters tuned
- [ ] Wind monitoring configured
- [ ] Emergency stop tested
- [ ] Stow position set
- [ ] Calibration complete

## 📖 Next Steps

After hardware setup:
1. Read [ECOWITT_SETUP.md](ECOWITT_SETUP.md) for wind integration
2. Read [INSTALL.md](INSTALL.md) for add-on details
3. Read [QUICKSTART.md](QUICKSTART.md) for operation
4. Tune PID parameters in README.md

## 🆘 Getting Help

**Hardware issues:**
- Check wiring diagram
- Test components individually
- Verify power supply
- Check serial output

**Software issues:**
- Check add-on logs
- Verify configuration
- Test API manually
- Review documentation

**Questions:**
- Create GitHub issue
- Include logs and config
- Describe hardware setup
- Upload photos if helpful
