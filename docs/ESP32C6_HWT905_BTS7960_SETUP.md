# ESP32-C6 Dual-Axis Solar Tracker Setup
## XIAO ESP32-C6 + WitMotion HWT905 + BTS7960 Motor Drivers

Your hardware configuration:
- **Board**: Seeed XIAO ESP32-C6
- **IMU**: WitMotion HWT905 (RS485 Modbus)
- **Motors**: Dual-axis (slewing drive + linear actuator)
- **Drivers**: 2× BTS7960 43A Motor Drivers
- **Limit Switches**: 2 switches (East and West for azimuth)

---

## 🔌 Pin Assignments

| Function | XIAO Pin | GPIO | Connection |
|----------|----------|------|------------|
| **RS485 TX** | D4 | 22 | HWT905 DI |
| **RS485 RX** | D5 | 23 | HWT905 RO |
| **RS485 DE** | D2 | 2 | HWT905 DE/RE |
| **Az RPWM** | D8 | 8 | BTS7960 #1 RPWM (CW) |
| **Az LPWM** | D9 | 9 | BTS7960 #1 LPWM (CCW) |
| **El RPWM** | D10 | 10 | BTS7960 #2 RPWM (Up) |
| **El LPWM** | D1 | 1 | BTS7960 #2 LPWM (Down) |
| **East Limit** | D6 | 6 | East limit switch |
| **West Limit** | D7 | 7 | West limit switch |

---

## 🔧 BTS7960 Motor Driver Wiring

### Understanding BTS7960
The BTS7960 is a high-current (43A) H-bridge driver with **two PWM inputs**:
- **RPWM** (Right PWM) - Forward direction
- **LPWM** (Left PWM) - Reverse direction
- **R_EN** - Right enable (connect to 5V)
- **L_EN** - Left enable (connect to 5V)

**Key difference from L298N**: BTS7960 uses **two separate PWM signals** instead of DIR+PWM.

### BTS7960 #1 - Azimuth Motor (Slewing Drive)

```
XIAO ESP32-C6          BTS7960 Driver #1          Slewing Drive Motor
─────────────          ─────────────────          ───────────────────
D8 (GPIO8)  ──→  RPWM  │                          
D9 (GPIO9)  ──→  LPWM  │  M+ ────────────→  Motor Terminal 1
5V          ──→  R_EN  │  M- ────────────→  Motor Terminal 2
5V          ──→  L_EN  │
GND         ──→  GND   │
                       │
12-24V+     ──→  VCC   │  (Motor power supply)
12-24V GND  ──→  GND   │
```

### BTS7960 #2 - Elevation Motor (Linear Actuator)

```
XIAO ESP32-C6          BTS7960 Driver #2          Linear Actuator
─────────────          ─────────────────          ───────────────
D10 (GPIO10) ──→  RPWM │                          
D1 (GPIO1)   ──→  LPWM │  M+ ────────────→  Motor Wire 1
5V           ──→  R_EN │  M- ────────────→  Motor Wire 2
5V           ──→  L_EN │
GND          ──→  GND  │
                       │
12-24V+      ──→  VCC  │  (Motor power supply)
12-24V GND   ──→  GND  │
```

### Complete Wiring Diagram

```
┌────────────────────────────────────────────────────────┐
│                  Power Distribution                    │
│                                                        │
│  12-24V Power Supply                                   │
│      │                                                 │
│      ├─→ BTS7960 #1 VCC (Azimuth)                     │
│      ├─→ BTS7960 #2 VCC (Elevation)                   │
│      └─→ Buck Converter (12/24V → 5V)                 │
│             │                                          │
│             └─→ XIAO ESP32-C6 5V pin                  │
└────────────────────────────────────────────────────────┘

┌────────────────────────────────────────────────────────┐
│                  XIAO ESP32-C6                         │
│                                                        │
│  D4 (GPIO22) ─┐                                        │
│  D5 (GPIO23) ─┤ RS485 Board → HWT905 IMU             │
│  D2 (GPIO2)  ─┘                                        │
│                                                        │
│  D8 (GPIO8)  ──→ BTS7960 #1 RPWM (Az CW)             │
│  D9 (GPIO9)  ──→ BTS7960 #1 LPWM (Az CCW)            │
│                                                        │
│  D10 (GPIO10) ──→ BTS7960 #2 RPWM (El Up)            │
│  D1 (GPIO1)   ──→ BTS7960 #2 LPWM (El Down)          │
│                                                        │
│  D6 (GPIO6)  ←── East Limit Switch                    │
│  D7 (GPIO7)  ←── West Limit Switch                    │
│                                                        │
│  5V, GND     ──→ Power                                │
└────────────────────────────────────────────────────────┘
```

---

## 📦 Bill of Materials

### Amazon Links

**BTS7960 Motor Drivers** (you need 2):
https://www.amazon.com/dp/B0D732VYGZ

### Complete Parts List

1. **XIAO ESP32-C6** (~$7)
2. **SeeedStudio RS485 Expansion Board** or MAX485 module (~$5)
3. **WitMotion HWT905** 9-axis IMU with RS485 (~$40)
4. **BTS7960 43A Motor Driver** × 2 (~$10 each)
5. **Slewing Drive Motor** (12-24V DC, geared) (~$100-300)
6. **Linear Actuator** (12-24V, 6-12" stroke) (~$30-80)
7. **Limit Switches** × 2 (~$5)
8. **Power Supply** 12V or 24V, 10-20A (~$30)
9. **Buck Converter** 12/24V → 5V, 3A (~$5)
10. Wires, connectors, heat shrink, enclosure

**Total**: ~$300-500 depending on motor choices

---

## 🚀 Firmware Upload

### 1. Install Arduino IDE
- Download Arduino IDE 2.x
- Add ESP32 board support:
  - File → Preferences
  - Add: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
  - Tools → Board Manager → Install "esp32"

### 2. Install Libraries
Library Manager (Ctrl+Shift+I):
- **ArduinoJson** (v6.x)

### 3. Configure Firmware
Open `Solar_Tracker_ESP32C6_HWT905.ino`:

```cpp
// Change WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// HWT905 defaults (usually don't need to change)
#define HWT905_ADDR 0x50      // Modbus address
#define HWT905_BAUD 115200    // Baud rate
```

### 4. Upload
1. Connect XIAO ESP32-C6 via USB-C
2. Tools → Board → "XIAO_ESP32C6"
3. Tools → Port → Select your COM port
4. Click **Upload** ⬆️
5. Open Serial Monitor (115200 baud)
6. **Note the IP address**

### 5. Test
```bash
# Browser
http://ESP32_IP/

# API Test
curl http://ESP32_IP/api/sensors

# Should return JSON with IMU data
```

---

## ⚙️ BTS7960 Configuration

### Important Notes

1. **Enable Pins Must Be HIGH**
   - Connect R_EN to 5V
   - Connect L_EN to 5V
   - Or connect both to ESP32 GPIO if you want software enable

2. **Never Drive Both PWMs High Simultaneously**
   - The firmware ensures only RPWM OR LPWM is active
   - Both LOW = stopped
   - RPWM high = forward
   - LPWM high = reverse

3. **Power Supply Sizing**
   - BTS7960 can handle 43A peak
   - Typical draw: 5-15A under load
   - Size supply for: (Azimuth current + Elevation current) × 1.5

4. **Heat Sinking**
   - BTS7960 can get hot under sustained load
   - Mount on metal plate or add heatsink
   - Ensure good airflow

5. **PWM Frequency**
   - Firmware uses 1kHz (works well for BTS7960)
   - Higher frequencies = smoother but hotter
   - Lower frequencies = less heat but more audible noise

---

## 🏠 Home Assistant Configuration

```yaml
# Basic Settings
esp32_ip: "192.168.1.XXX"  # Your ESP32 IP
esp32_port: 80
latitude: 40.7128
longitude: -74.0060
timezone: "America/New_York"

# MQTT
mqtt_broker: "core-mosquitto"
mqtt_port: 1883

# Ecowitt WS90 Wind
ha_wind_speed_entity: "sensor.ws90_wind_speed"
ha_wind_gust_entity: "sensor.ws90_gust_speed"
use_wind_gust_for_stow: true
max_wind_speed: 50

# PID Tuning
pid_kp: 1.0
pid_ki: 0.1
pid_kd: 0.05
update_interval: 1.0

# Stow Position
stow_position_azimuth: 180    # South
stow_position_elevation: 0    # Horizontal

# Initial Mode
tracking_mode: "auto"
```

---

## 🧪 Testing Procedure

### 1. Test HWT905 IMU
```bash
# Serial Monitor should show:
# "HWT905: Read complete" every 100ms

# Test API:
curl http://ESP32_IP/api/sensors | jq

# Should show accel, gyro, mag, angles, temp
```

### 2. Test Azimuth Motor (Slewing Drive)
```bash
# Clockwise (East) - RPWM active
curl -X POST http://ESP32_IP/api/motor \
  -H "Content-Type: application/json" \
  -d '{"motor":"azimuth","speed":20}'

# Counter-clockwise (West) - LPWM active
curl -X POST http://ESP32_IP/api/motor \
  -d '{"motor":"azimuth","speed":-20}'

# Stop
curl -X POST http://ESP32_IP/api/motor \
  -d '{"motor":"azimuth","speed":0}'
```

### 3. Test Elevation Motor (Linear Actuator)
```bash
# Extend (Up) - RPWM active
curl -X POST http://ESP32_IP/api/motor \
  -d '{"motor":"elevation","speed":30}'

# Retract (Down) - LPWM active
curl -X POST http://ESP32_IP/api/motor \
  -d '{"motor":"elevation","speed":-30}'

# Stop
curl -X POST http://ESP32_IP/api/motor \
  -d '{"motor":"elevation","speed":0}'
```

### 4. Test Limit Switches
```bash
# Check status
curl http://ESP32_IP/api/limits

# Manually trigger each switch
# Serial Monitor should show "East limit hit!" or "West limit hit!"
# Motor should stop automatically
```

### 5. Test Emergency Stop
```bash
curl -X POST http://ESP32_IP/api/stop

# Both motors should stop immediately
```

---

## 🐛 Troubleshooting

### Motors Don't Move

**Check BTS7960 Power:**
```
- VCC connected to 12-24V?
- GND connected?
- Voltage correct (use multimeter)?
```

**Check Enable Pins:**
```
- R_EN connected to 5V?
- L_EN connected to 5V?
- Measure voltage on enable pins
```

**Check PWM Signals:**
```
- Use oscilloscope or LED
- D8, D9, D10, D1 should pulse
- Only one PWM active per driver
```

**Check Motor Connections:**
```
- M+ and M- wired to motor?
- Motor not mechanically stuck?
- Try swapping M+ and M- (reverses direction)
```

### Motor Runs Only One Direction

**Possible causes:**
- One PWM pin not connected
- One H-bridge side failed
- Check both RPWM and LPWM pins
- Test with multimeter

### BTS7960 Gets Hot

**Normal operation:**
- Warm to touch is OK
- Can add heatsink if concerned

**Too hot (>80°C):**
- Reduce PWM duty cycle
- Improve ventilation
- Check for motor stall
- Verify power supply voltage

### HWT905 No Data

**Check RS485 Wiring:**
```
- A to A, B to B (not crossed)
- Twisted pair cable
- Max 1200m cable length
- 120Ω termination if long cable
```

**Check Baud Rate:**
```
- Default: 115200
- Use WitMotion software to verify
```

**Check Address:**
```
- Default: 0x50 (80 decimal)
- Verify with WitMotion software
```

### Limit Switches Not Working

**Check Wiring:**
```
- COM to GPIO (D6 or D7)
- NO to 3.3V
- Normally open configuration
```

**Test Electrically:**
```
- Measure voltage on GPIO pin
- Should be HIGH when switch closed
- LOW when open (internal pull-down)
```

---

## 📐 Mechanical Setup

### Slewing Drive Mounting
1. Mount slewing drive horizontally
2. Attach solar panel frame to rotating platform
3. Ensure smooth 360° rotation
4. No cable twist issues (use slip ring if needed)

### Linear Actuator Mounting
1. One end to panel frame (pivot point)
2. Other end to slewing drive platform
3. Calculate stroke length needed:
   - 0° to 90° elevation typically needs 6-12" stroke
4. Mount points should allow full extension/retraction

### Limit Switch Placement
- **East**: Trigger at maximum clockwise rotation
- **West**: Trigger at maximum counter-clockwise
- Leave 5-10° safety margin
- Use weatherproof switches (IP65+)

---

## 🎯 Calibration

### 1. Mechanical Alignment
```
1. Point tracker due South (azimuth = 180°)
2. Set elevation to horizontal (0°)
3. Note HWT905 yaw reading
4. This is your zero reference
```

### 2. PID Tuning

**Start conservative:**
```yaml
pid_kp: 0.5
pid_ki: 0.05
pid_kd: 0.02
```

**If too slow:**
```yaml
pid_kp: 1.5    # Increase proportional
```

**If oscillates:**
```yaml
pid_kd: 0.1    # Increase derivative
pid_kp: 0.8    # Decrease proportional
```

**For smooth tracking:**
```yaml
pid_kp: 1.0
pid_ki: 0.1
pid_kd: 0.05
update_interval: 1.0  # 1 second updates
```

### 3. Test Auto Tracking
1. Set mode to "auto"
2. Watch for 1 hour
3. Verify smooth following
4. Adjust PID if needed
5. Test over full day

---

## ⚡ Power Management

### Current Draw Estimates
- ESP32-C6: 50-200mA
- HWT905: 50mA
- BTS7960 (idle): 10mA each
- BTS7960 (active): 2-10A each depending on motor load

### Recommended Power Supply
- **Voltage**: Match your motors (12V or 24V common)
- **Current**: (Max motor current × 2) + 2A for electronics
- **Example**: 10A motors → 22A supply minimum

### Wire Sizing
- **Motor wires**: 14-16 AWG
- **Power supply to BTS7960**: 12-14 AWG  
- **Signal wires**: 22-24 AWG

---

## 📊 Performance Expectations

### Tracking Accuracy
- **With good PID tuning**: ±0.5-1°
- **Without tuning**: ±2-5°
- **HWT905 accuracy**: ±0.1° (yaw)

### Speed
- **Azimuth**: Depends on slewing drive (typically 1-5°/sec)
- **Elevation**: Depends on actuator (typically 0.5-2°/sec)
- **Sun movement**: ~0.25°/min (15°/hour)

### Efficiency Gains
- **vs Fixed**: 25-35% more energy
- **vs Single-axis**: 5-10% more energy
- **Total improvement**: 30-45% over fixed

---

## 🔒 Safety Features

### Hardware Safeties
✅ Limit switches prevent over-rotation
✅ BTS7960 has overcurrent protection
✅ Thermal shutdown if overheated
✅ Reverse polarity protection

### Software Safeties
✅ Wind monitoring with auto-stow
✅ Emergency stop command
✅ Limit switch monitoring
✅ Watchdog timer
✅ Communication timeout

---

## 📚 Additional Resources

- **WitMotion HWT905 Manual**: [witmotion.com](https://www.wit-motion.com)
- **BTS7960 Datasheet**: Search "BTS7960 datasheet" 
- **Home Assistant Add-on Docs**: See INSTALL.md
- **Ecowitt Integration**: See ECOWITT_SETUP.md

---

## ✅ Pre-Flight Checklist

Before enabling auto mode:

- [ ] ESP32 has stable WiFi
- [ ] HWT905 reading correctly (check /api/sensors)
- [ ] Both motors respond to commands
- [ ] Azimuth CW and CCW work
- [ ] Elevation up and down work
- [ ] Limit switches tested and working
- [ ] Manual mode works in Home Assistant
- [ ] Wind monitoring configured
- [ ] Emergency stop tested
- [ ] PID parameters tuned
- [ ] Mechanical assembly secure
- [ ] All wiring secure and weatherproof
- [ ] Power supply adequate
- [ ] BTS7960 drivers cool under load

---

## 🎉 You're Ready!

Your dual-axis solar tracker with professional components is now ready for operation!

**Key advantages of your setup:**
✅ High-current BTS7960 drivers (43A rating)
✅ Industrial RS485 communication (noise-immune)
✅ Professional 9-axis IMU with Kalman filtering
✅ Dual-axis tracking for maximum efficiency
✅ Robust limit switch protection
✅ Integrated wind monitoring

Perfect for serious solar tracking applications! 🌞
