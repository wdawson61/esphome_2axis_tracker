# Solar Tracker Complete System
## Dual-Axis Solar Tracker with Home Assistant Integration

**Version**: 1.0.0  
**Date**: November 2024  
**Hardware**: ESP32-C6 + WitMotion HWT905 + BTS7960 Motor Drivers

---

## 📦 What's Included

This complete package contains everything you need to build a professional dual-axis solar tracker:

### 🏠 Home Assistant Add-on
Professional Python application for intelligent tracking control:
- Astronomical sun position calculations
- 9-axis sensor fusion algorithms
- Dual-axis PID control
- Wind monitoring with Ecowitt integration
- Web dashboard and MQTT interface
- **Location**: `home_assistant_addon/solar_tracker/`

### 🔧 ESP32-C6 Firmware
Hardware controller for motors and sensors:
- WitMotion HWT905 IMU via RS485/Modbus
- BTS7960 43A motor driver control (4 PWM outputs)
- Dual-axis motion control (azimuth + elevation)
- Limit switch protection
- HTTP API for Home Assistant
- **Location**: `firmware/Solar_Tracker_ESP32C6_HWT905.ino`

### 📚 Complete Documentation
Detailed guides for every aspect:
- Hardware setup and wiring
- Firmware configuration and upload
- Home Assistant installation
- PID tuning and calibration
- Troubleshooting and maintenance
- **Location**: `docs/`

### 🔬 Example Code
Additional firmware options and references:
- Generic ESP32 firmware (non-C6 boards)
- ESPHome configuration
- Modbus communication examples
- **Location**: `examples/`

---

## 🎯 Your Hardware Configuration

### Core Components
- **MCU**: Seeed XIAO ESP32-C6
- **IMU**: WitMotion HWT905 (9-axis, RS485)
- **Motor Drivers**: 2× BTS7960 43A H-Bridge
- **Motors**: 
  - Azimuth: Slewing drive
  - Elevation: Linear actuator
- **Sensors**: 2× limit switches (East/West)
- **Weather**: Ecowitt WS90 (integrated in Home Assistant)

### Pin Configuration Summary
```
RS485:  D4(TX), D5(RX), D2(DE)
Azimuth Motor:  D8(RPWM), D9(LPWM)
Elevation Motor: D10(RPWM), D1(LPWM)
Limits: D6(East), D7(West)
```

Full details in: `docs/ESP32C6_HWT905_BTS7960_SETUP.md`

---

## 🚀 Quick Start Guide

### 1. Upload ESP32 Firmware (30 minutes)
```bash
1. Install Arduino IDE 2.x
2. Add ESP32 board support
3. Install ArduinoJson library
4. Open: firmware/Solar_Tracker_ESP32C6_HWT905.ino
5. Configure WiFi credentials
6. Select Board: XIAO_ESP32C6
7. Upload
8. Note IP address from Serial Monitor
```

**Detailed guide**: `docs/ESP32C6_HWT905_BTS7960_SETUP.md`

### 2. Wire Hardware (2-4 hours)
```bash
1. Connect RS485 board to HWT905 IMU
2. Wire BTS7960 driver #1 to azimuth motor
3. Wire BTS7960 driver #2 to elevation motor
4. Install limit switches
5. Connect power supplies (12-24V motors, 5V ESP32)
```

**Wiring diagrams**: `docs/ESP32C6_HWT905_BTS7960_SETUP.md` and `docs/WIRING.md`

### 3. Install Home Assistant Add-on (20 minutes)
```bash
1. Install Mosquitto MQTT broker (if not already)
2. Add this repository to Home Assistant
3. Install "Solar Tracker Controller" add-on
4. Configure:
   - ESP32 IP address
   - Your location (lat/long)
   - Ecowitt wind entities
   - PID parameters
5. Start add-on
```

**Detailed guide**: `home_assistant_addon/solar_tracker/INSTALL.md`

### 4. Test and Calibrate (1-2 hours)
```bash
1. Test IMU readings
2. Test motor movements
3. Verify limit switches
4. Configure wind monitoring
5. Calibrate sensors
6. Tune PID parameters
7. Test in manual mode
8. Enable auto mode
```

**Testing procedures**: `docs/ESP32C6_HWT905_BTS7960_SETUP.md`

---

## 📂 Directory Structure

```
solar_tracker_complete/
│
├── 📄 README.md                          ⭐ This file - Start here!
│
├── 📁 firmware/                          🔧 ESP32-C6 Firmware
│   └── Solar_Tracker_ESP32C6_HWT905.ino    Main firmware for your hardware
│
├── 📁 home_assistant_addon/              🏠 Home Assistant Add-on
│   └── solar_tracker/                      Complete add-on directory
│       ├── solar_tracker/                  Python application
│       │   ├── main.py                     Entry point
│       │   ├── controller.py               Main control logic
│       │   ├── sensor_fusion.py            9-axis IMU fusion
│       │   ├── esp32_interface.py          ESP32 communication
│       │   ├── mqtt_interface.py           MQTT integration
│       │   ├── ha_interface.py             HA entity reading
│       │   └── web_server.py               Web dashboard
│       ├── web/                            Web interface
│       │   └── index.html                  Dashboard UI
│       ├── config.yaml                     Add-on configuration
│       ├── Dockerfile                      Container build
│       ├── requirements.txt                Dependencies
│       ├── run.sh                          Startup script
│       ├── README.md                       Complete documentation
│       ├── INSTALL.md                      Installation guide
│       ├── QUICKSTART.md                   Quick reference
│       ├── ECOWITT_SETUP.md                Ecowitt WS90 integration
│       ├── HA_ENTITY_INTEGRATION.md        Entity integration guide
│       └── CHANGELOG.md                    Version history
│
├── 📁 docs/                              📚 Documentation
│   ├── ESP32C6_HWT905_BTS7960_SETUP.md   ⭐ PRIMARY HARDWARE GUIDE
│   ├── INDEX.md                            Master file index
│   ├── PROJECT_SUMMARY.md                  Executive overview
│   ├── AUTOMATION_VS_ADDON.md              Comparison guide
│   ├── WIRING.md                           Detailed wiring info
│   └── HARDWARE_GUIDE.md                   General hardware info
│
└── 📁 examples/                          🔬 Reference Code
    ├── ESP32_Modbus_Web_Original.ino       Your original RS485 code
    ├── ESP32_Modbus_Web.ino                Enhanced Modbus interface
    ├── Solar_Tracker_ESP32_Hardware.ino    Generic ESP32 firmware
    └── solar_tracker_esphome.yaml          ESPHome alternative
```

---

## 📖 Documentation Guide

### 🚀 Getting Started
1. **Start Here**: `README.md` (this file)
2. **Hardware Setup**: `docs/ESP32C6_HWT905_BTS7960_SETUP.md`
3. **Add-on Installation**: `home_assistant_addon/solar_tracker/INSTALL.md`
4. **Daily Operation**: `home_assistant_addon/solar_tracker/QUICKSTART.md`

### 🔧 Configuration
- **Wind Integration**: `home_assistant_addon/solar_tracker/ECOWITT_SETUP.md`
- **PID Tuning**: `docs/ESP32C6_HWT905_BTS7960_SETUP.md` (Calibration section)
- **Wiring Details**: `docs/WIRING.md` and `docs/ESP32C6_HWT905_BTS7960_SETUP.md`

### 📊 Reference
- **Complete File Index**: `docs/INDEX.md`
- **Project Overview**: `docs/PROJECT_SUMMARY.md`
- **Design Decisions**: `docs/AUTOMATION_VS_ADDON.md`

---

## 🎯 Key Features

### Astronomical Tracking
✅ Real-time sun position calculation (< 0.01° accuracy)  
✅ Automatic timezone handling  
✅ Dawn-to-dusk operation  
✅ Configurable stow position  

### Sensor Fusion
✅ 9-axis IMU (accelerometer + gyroscope + magnetometer)  
✅ Complementary filter algorithm  
✅ Tilt-compensated compass heading  
✅ Temperature compensation  
✅ Drift-free orientation tracking  

### Motor Control
✅ Dual-axis PID control (azimuth + elevation)  
✅ Smooth motion with configurable parameters  
✅ Anti-windup and output limiting  
✅ BTS7960 43A H-bridge drivers  
✅ 4 separate PWM outputs  

### Safety Features
✅ Wind monitoring with auto-stow (Ecowitt WS90)  
✅ Configurable wind threshold (average or gust)  
✅ Hardware limit switches (East/West)  
✅ Emergency stop  
✅ Communication watchdog  
✅ Automatic night stowing  

### Integration
✅ Home Assistant MQTT auto-discovery  
✅ Real-time web dashboard  
✅ RESTful API  
✅ Ecowitt weather station integration  
✅ Cloud-free operation  

---

## ⚙️ System Requirements

### Home Assistant Server
- Home Assistant OS, Supervised, or Container
- 2GB+ RAM recommended
- Mosquitto MQTT broker add-on
- Network access to ESP32

### ESP32 Hardware
- XIAO ESP32-C6 (or compatible)
- 5V power supply (2A)
- WiFi 2.4GHz network
- Stable IP address recommended

### Motors and Drivers
- 12V or 24V DC motors
- BTS7960 motor drivers (2×)
- Power supply sized for motor current
- Proper heat sinking

---

## 🔍 Troubleshooting

### Common Issues

**ESP32 won't connect to WiFi**
- Check SSID/password in firmware
- Ensure 2.4GHz network (not 5GHz)
- Check Serial Monitor for errors

**No IMU data**
- Verify RS485 wiring (A-to-A, B-to-B)
- Check HWT905 power and baud rate
- Test with `curl http://ESP_IP/api/sensors`

**Motors don't move**
- Check BTS7960 power (12-24V)
- Verify enable pins (R_EN, L_EN to 5V)
- Test PWM outputs with multimeter/oscilloscope
- Check motor connections (M+, M-)

**Add-on can't connect to ESP32**
- Verify ESP32 IP address
- Test with: `curl http://ESP_IP/status`
- Check firewall rules
- Ensure ESP32 web server is running

**Full troubleshooting guide**: `docs/ESP32C6_HWT905_BTS7960_SETUP.md`

---

## 📊 Performance Specifications

### Tracking Accuracy
- **Positioning**: ±0.5° - 1° (with tuned PID)
- **IMU Resolution**: 0.01° (HWT905)
- **Update Rate**: 10Hz (sensor reading), 1Hz (control loop)

### Efficiency Gains
- **vs Fixed Panel**: +30-35% energy production
- **vs Single-Axis**: +5-10% additional gain
- **Total Improvement**: 35-45% over fixed installation

### Response Time
- **Sun Tracking**: 15°/hour average motion
- **Motor Speed**: Configurable via PID
- **Typical**: Full range in 2-5 minutes

---

## 🛠️ Maintenance

### Regular Checks (Monthly)
- [ ] Clean solar panels
- [ ] Inspect mechanical components
- [ ] Check wire connections
- [ ] Verify limit switches
- [ ] Test emergency stop
- [ ] Review tracking logs

### Seasonal Tasks
- [ ] Recalibrate IMU (if drift detected)
- [ ] Re-tune PID parameters (if needed)
- [ ] Lubricate moving parts
- [ ] Check weatherproofing
- [ ] Update firmware/add-on

---

## 🆘 Support

### Documentation
- All guides included in `docs/` directory
- Add-on documentation in `home_assistant_addon/solar_tracker/`
- Hardware-specific info in `docs/ESP32C6_HWT905_BTS7960_SETUP.md`

### Self-Help
1. Check relevant documentation first
2. Review troubleshooting sections
3. Examine add-on logs in Home Assistant
4. Check ESP32 Serial Monitor output
5. Test individual components

### Creating Issues
When reporting problems, include:
- Hardware configuration
- Firmware version
- Add-on configuration (remove passwords)
- Complete error logs
- Steps to reproduce
- Photos of wiring (if applicable)

---

## 🎓 Learning Path

### Beginner (Week 1)
1. Upload firmware and test basic operation
2. Get familiar with web interface
3. Test manual motor control
4. Install Home Assistant add-on

### Intermediate (Week 2-3)
1. Configure wind monitoring
2. Calibrate IMU sensors
3. Basic PID tuning
4. Enable auto tracking for short periods

### Advanced (Week 4+)
1. Fine-tune PID parameters
2. Optimize for your location
3. Set up automations
4. Monitor and analyze performance
5. Customize code if needed

---

## 📜 License

This project is provided as-is for educational and personal use.

**Components**:
- Custom code: MIT License (modify freely)
- Hardware designs: Open source
- Documentation: Creative Commons

**Third-party libraries**:
- ArduinoJson: MIT License
- Home Assistant: Apache 2.0 License
- See individual files for specific licenses

---

## 🙏 Acknowledgments

**Hardware**:
- Seeed Studio (XIAO ESP32-C6)
- WitMotion (HWT905 IMU)
- BTS7960 driver manufacturers

**Software**:
- Home Assistant community
- Arduino/ESP32 ecosystem
- Python libraries

**Inspiration**:
- DIY solar tracking community
- Open source hardware movement

---

## 📈 Future Enhancements

### Planned Features (v1.1+)
- [ ] Cloud cover detection
- [ ] Historical performance tracking
- [ ] Machine learning for predictive control
- [ ] Multi-tracker coordination
- [ ] Mobile app integration
- [ ] Weather forecast integration

### Community Contributions Welcome
- Additional hardware support
- Improved algorithms
- Better documentation
- Language translations

---

## 🎉 You're Ready to Build!

**What you have**:
✅ Professional dual-axis firmware  
✅ Intelligent Home Assistant add-on  
✅ Complete documentation  
✅ Hardware-specific setup guide  
✅ Troubleshooting resources  
✅ Example code and references  

**Time to build**: ~1-2 days for complete setup  
**Cost**: ~$300-500 for electronics + motors  
**Result**: Professional solar tracker with 35-45% efficiency gain!

---

## 📞 Quick Reference

| Component | Location | Purpose |
|-----------|----------|---------|
| **Firmware** | `firmware/Solar_Tracker_ESP32C6_HWT905.ino` | Upload to ESP32 |
| **Setup Guide** | `docs/ESP32C6_HWT905_BTS7960_SETUP.md` | Hardware instructions |
| **Add-on** | `home_assistant_addon/solar_tracker/` | Install in HA |
| **Install Guide** | `home_assistant_addon/solar_tracker/INSTALL.md` | HA setup |
| **Quick Ref** | `home_assistant_addon/solar_tracker/QUICKSTART.md` | Daily operation |
| **Wind Setup** | `home_assistant_addon/solar_tracker/ECOWITT_SETUP.md` | Ecowitt config |
| **Examples** | `examples/` | Reference code |

---

**Happy Tracking! 🌞**

For the most up-to-date information, always refer to the documentation in the `docs/` directory.
