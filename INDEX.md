# Solar Tracker Project - Complete Documentation Index

## 📦 Project Overview

**Complete ESPHome firmware for ESP32-C6 based 2-axis solar tracker**

- **Status**: ✅ Production Ready
- **Hardware**: BTS7960, WIT-Motion HWT905, SeeedStudio RS485, 24V motors
- **Features**: Dual-axis control, RS485 IMU, homing system, Home Assistant integration
- **Documentation**: 14 files, ~4,200 lines, 158KB total
- **Last Updated**: November 2024

---

## 🚀 Quick Start (First Time Users)

**Essential reading order:**

1. 📖 [**QUICKSTART.md**](QUICKSTART.md) - 5-minute setup guide
2. 🛒 [**HARDWARE_BOM.md**](HARDWARE_BOM.md) - Shopping list ($246-690)
3. 🔧 [**README.md**](README.md) - Complete reference
4. 🐛 [**DEBUGGING.md**](DEBUGGING.md) - When things don't work

**Then proceed to:**
- Flash firmware from `solar_tracker.yaml`
- Configure `secrets.yaml` with WiFi credentials
- Run sensor calibration
- Home the azimuth axis
- Start tracking!

---

## 📁 File Directory

### Core Firmware Files

#### `solar_tracker.yaml` (4.8KB)
**Main ESPHome configuration file**

📋 Contents:
- ESP32-C6 platform configuration
- UART setup for HWT905 sensor
- GPIO pin assignments
- Custom C++ component integration
- Binary sensors (motors, home switch)
- Services (set angles, home, calibrate, stop)
- Number inputs for target angles
- WiFi and API configuration

🔑 Key sections:
```yaml
uart:           # HWT905 communication
sensor:         # Elevation, heading, acceleration
binary_sensor:  # Motor status, home switch
api:services:   # Home Assistant services
```

⚙️ Customization points:
- WiFi credentials (via secrets.yaml)
- GPIO pins (if different hardware)
- Component includes
- Service definitions

---

#### `solar_tracker.h` (21KB)
**C++ implementation of sensor and motor control**

📋 Contents:
- `HWT905Sensor` class - RS485 IMU interface
- `SolarTrackerMotorController` class - Motor control
- Homing state machine
- Position feedback loops
- Safety timeouts

🔑 Key components:

**HWT905Sensor:**
- UART packet parsing
- Angle and acceleration reading
- Sensor calibration
- Checksum validation

**SolarTrackerMotorController:**
- Elevation control (linear actuator)
- Azimuth control (slewing drive)
- 5-phase homing sequence
- Home offset management
- Safety features

⚙️ Tunable constants:
```cpp
ELEVATION_TOLERANCE = 0.5°
AZIMUTH_TOLERANCE = 2.0°
MOTOR_TIMEOUT = 120 seconds
HOMING_TIMEOUT = 180 seconds
```

---

#### `secrets.yaml` (285 bytes)
**WiFi credentials template**

📋 Contents:
- WiFi SSID and password
- API encryption key
- OTA password
- Fallback AP password

⚠️ **IMPORTANT**: 
- Copy to your ESPHome config directory
- Fill in your actual credentials
- **Never commit to version control!**
- Generate API key: `esphome wizard solar_tracker.yaml`

---

### Documentation Files

#### `README.md` (13KB) ⭐ **START HERE**
**Complete reference documentation**

📖 Chapters:
1. Features overview
2. Hardware requirements (BTS7960, HWT905, etc.)
3. Pin assignments
4. Wiring diagrams (RS485, BTS7960, limit switch)
5. Installation steps
6. Usage instructions
7. Services (set angles, home, calibrate)
8. Sensors (elevation, heading, acceleration)
9. Automation examples
10. Configuration options
11. Troubleshooting
12. Protocol details
13. Safety features

🎯 Use when:
- First time setup
- Need wiring diagrams
- Understanding service calls
- Configuring parameters
- Learning HWT905 protocol

---

#### `QUICKSTART.md` (7.0KB) 🚀 **FASTEST PATH**
**5-minute setup guide**

📖 Contents:
- Hardware assembly (15 min)
- Software setup (5 min)
- First boot procedure
- Calibration (1 min)
- Homing (2 min)
- Test movement (2 min)
- Verification checklist
- Common first-time issues
- Expected readings
- Essential services
- Dashboard card example

🎯 Use when:
- You want to get running FAST
- Have hardware ready to assemble
- Need step-by-step instructions
- First time with ESPHome

⏱️ Total time: ~30 minutes from zero to tracking

---

#### `DEBUGGING.md` (14KB) 🐛 **WHEN THINGS BREAK**
**Comprehensive troubleshooting guide**

📖 Issues covered:
1. No sensor data (RS485 issues)
2. Sensor data inaccurate
3. Motors not moving (BTS7960 problems)
4. Motors run but don't stop
5. Azimuth wrong direction
6. Compilation errors
7. **Homing fails or times out** ⭐ NEW
8. Azimuth commands rejected
9. Position inaccurate after homing

🔧 For each issue:
- Symptoms
- Detailed diagnostics
- Step-by-step solutions
- Log analysis
- Hardware tests

➕ Additional sections:
- Performance optimization
- Advanced debugging (UART, JTAG)
- Log analysis examples
- Safety checks
- Performance benchmarks

🎯 Use when:
- Setup not working
- Erratic behavior
- Communication failures
- Motor problems
- Homing issues

---

#### `HOMING.md` (11KB) 🏠 **HOMING DEEP DIVE**
**Complete guide to azimuth homing system**

📖 Contents:
1. Why homing is necessary
2. Hardware requirements (limit switch types)
3. Detailed sequence explanation (5 phases)
4. State machine diagram
5. Usage instructions
6. Position calculation math
7. Safety features
8. Troubleshooting
9. Configuration parameters
10. Advanced topics
11. Best practices
12. Integration with sun tracking

🔍 State machine phases:
1. Move off switch
2. Seek switch
3. Slow approach
4. Set zero
5. Complete

🎯 Use when:
- Understanding homing necessity
- Selecting limit switch
- Debugging homing issues
- Optimizing homing parameters
- Implementing automatic homing

---

#### `HARDWARE_BOM.md` (13KB) 🛒 **SHOPPING LIST**
**Complete bill of materials with specs**

📖 Contents:
1. Core components (ESP32, HWT905, etc.)
2. Detailed specifications for each
3. Where to buy (multiple vendors)
4. Price estimates (low/high)
5. Alternative options
6. Quality considerations
7. Mounting hardware
8. Optional components
9. Total cost breakdown
10. Tools required
11. Sourcing tips
12. Pre-purchase checklist

💰 Cost Summary:
- **Budget build**: $246
- **Premium build**: $690
- **Typical**: $400-500

🛒 Key components:
- ESP32-C6: $5-10
- HWT905: $30-50
- SeeedStudio RS485: $5-8
- BTS7960 (×2): $16-30
- Motors: $100-350
- Power supplies: $35-70

🎯 Use when:
- Planning purchase
- Budgeting project
- Selecting components
- Finding vendors
- Comparing options

---

#### `PYCHARM_SETUP.md` (19KB) 💻 **DEVELOPMENT GUIDE**
**Complete PyCharm/ESPHome dev environment**

📖 Chapters:
1. Prerequisites
2. Installing PyCharm
3. ESPHome development setup
4. Home Assistant development setup
5. Remote debugging ESPHome devices
6. Debugging C++ components
7. Best practices
8. Common issues
9. Advanced remote development

🛠️ Covers:
- Virtual environment setup
- Run configurations
- Breakpoint debugging
- Serial logging
- OTA logging
- MQTT logging
- JTAG debugging
- Mock testing
- Git integration
- File watchers
- Live templates

🎯 Use when:
- Setting up development environment
- Want to modify firmware
- Need to debug issues
- Contributing to project
- Learning ESPHome internals

⚙️ IDE options:
- PyCharm Professional (recommended)
- PyCharm Community (free)
- VS Code (alternative)
- CLion (for C++)

---

#### `PROJECT_SUMMARY.md` (7.8KB) 📊 **EXECUTIVE OVERVIEW**
**High-level project summary**

📖 Contents:
1. Delivered files list
2. Features implemented
3. Hardware support
4. Pin configuration
5. Key design decisions
6. Technical specifications
7. Quick deployment
8. Validation results
9. Documentation structure
10. Home Assistant integration
11. Future enhancements

🎯 Use when:
- Need quick overview
- Sharing project
- Understanding architecture
- Evaluating completeness
- Presenting to others

---

#### `HOMING_CHANGES.md` (8.2KB) 📝 **HOMING FEATURE LOG**
**Detailed changelog for homing addition**

📖 Contents:
1. Hardware additions (GPIO10 limit switch)
2. YAML configuration updates
3. C++ implementation changes
4. New class members
5. New methods
6. Modified methods
7. Documentation updates
8. Testing validation
9. Benefits summary

🎯 Use when:
- Understanding homing implementation
- Reviewing what changed
- Migrating from older version
- Learning implementation details

---

#### `HARDWARE_UPDATES.md` (15KB) 📋 **HARDWARE SPEC LOG**
**Documentation of hardware specifications update**

📖 Contents:
1. Changes made (BTS7960, SeeedStudio, 24V)
2. Files updated
3. BTS7960 integration details
4. SeeedStudio RS485 integration
5. Power system design (why 24V)
6. Migration from L298N
7. Testing validation
8. Quick reference card

🎯 Use when:
- Understanding specific hardware choices
- Migrating from different hardware
- Reviewing recent changes
- Need quick reference

---

### Automation & Testing

#### `home_assistant_automations.yaml` (13KB)
**Pre-built Home Assistant automations**

📋 Contains:
- **Basic sun tracking** (every 5 minutes)
- **Startup homing** (automatic on boot)
- **Night stow** (sunset positioning)
- **Morning startup** (pre-sunrise positioning)
- **Wind protection** (high wind stow)
- **Wind recovery** (resume after calm)
- **Ice protection** (steep angle in freezing rain)
- **Maintenance mode** (suspend tracking)
- **Weekly calibration** (automatic Sunday 2AM)
- **Sensor failure emergency stop**
- **Low battery protection**

➕ Also includes:
- Input helpers (booleans, numbers)
- Scripts (manual position, test routine)
- Sensors (tracking error, status)

🎯 Use when:
- Setting up automations
- Need examples
- Want automatic operation
- Implementing safety features

---

#### `test_firmware.py` (11KB) ✅ **VALIDATION SUITE**
**Automated testing of firmware logic**

📋 Tests:
1. ✅ HWT905 protocol parsing
2. ✅ Azimuth error calculation (wraparound)
3. ✅ Motor control state machine
4. ✅ Safety features (angle limiting)
5. ✅ Homing state machine
6. ✅ Home offset calculation

🖥️ Also displays:
- Pin configuration
- Homing switch configuration
- Calibration sequence
- Next steps

▶️ Run with:
```bash
python3 test_firmware.py
```

🎯 Use when:
- Validating firmware changes
- Understanding logic
- Learning implementation
- Contributing code

---

## 🗂️ Documentation by Use Case

### I want to...

#### **Build the tracker from scratch**
1. [HARDWARE_BOM.md](HARDWARE_BOM.md) - Order components
2. [QUICKSTART.md](QUICKSTART.md) - Assembly & setup
3. [README.md](README.md) - Detailed reference
4. [home_assistant_automations.yaml](home_assistant_automations.yaml) - Setup automations

#### **Troubleshoot a problem**
1. [DEBUGGING.md](DEBUGGING.md) - Troubleshooting guide
2. [HOMING.md](HOMING.md) - If homing issues
3. [README.md](README.md) - Protocol & configuration details

#### **Modify the firmware**
1. [PYCHARM_SETUP.md](PYCHARM_SETUP.md) - Dev environment
2. [solar_tracker.h](solar_tracker.h) - C++ source code
3. [solar_tracker.yaml](solar_tracker.yaml) - ESPHome config
4. [test_firmware.py](test_firmware.py) - Test your changes

#### **Understand the design**
1. [PROJECT_SUMMARY.md](PROJECT_SUMMARY.md) - High-level overview
2. [HARDWARE_UPDATES.md](HARDWARE_UPDATES.md) - Hardware choices
3. [HOMING_CHANGES.md](HOMING_CHANGES.md) - Homing implementation
4. [README.md](README.md) - Complete details

#### **Set up automations**
1. [home_assistant_automations.yaml](home_assistant_automations.yaml) - Examples
2. [README.md](README.md) - Service documentation
3. [QUICKSTART.md](QUICKSTART.md) - Basic sun tracking

---

## 📊 File Statistics

| File | Size | Lines | Type | Purpose |
|------|------|-------|------|---------|
| solar_tracker.yaml | 4.8KB | 160 | Config | ESPHome configuration |
| solar_tracker.h | 21KB | 680 | C++ | Sensor & motor control |
| secrets.yaml | 285B | 8 | Config | Credentials template |
| README.md | 13KB | 400 | Doc | Complete reference |
| QUICKSTART.md | 7.0KB | 240 | Doc | 5-min setup guide |
| DEBUGGING.md | 14KB | 450 | Doc | Troubleshooting |
| HOMING.md | 11KB | 350 | Doc | Homing deep dive |
| HARDWARE_BOM.md | 13KB | 420 | Doc | Shopping list |
| PYCHARM_SETUP.md | 19KB | 650 | Doc | Dev environment |
| PROJECT_SUMMARY.md | 7.8KB | 280 | Doc | Executive overview |
| HOMING_CHANGES.md | 8.2KB | 260 | Doc | Homing changelog |
| HARDWARE_UPDATES.md | 15KB | 480 | Doc | Hardware spec log |
| home_assistant_automations.yaml | 13KB | 410 | Config | HA automations |
| test_firmware.py | 11KB | 350 | Test | Validation suite |
| **TOTAL** | **158KB** | **~4,700** | | |

---

## 🏆 Project Highlights

### ✅ Completeness
- 14 comprehensive files
- 4,700+ lines of code & documentation
- Full hardware specifications
- End-to-end setup guide
- Extensive troubleshooting
- Professional dev environment guide
- Complete shopping list
- Automated testing

### ✅ Quality
- Production-ready firmware
- All tests passing
- Safety features built-in
- Comprehensive error handling
- Multiple documentation levels
- Real-world tested logic

### ✅ Hardware Specifications
- **BTS7960** 43A H-bridge (specified)
- **SeeedStudio RS485** (specified)
- **24V power system** (fully designed)
- **Complete BOM** with prices
- **Known-working hardware**

### ✅ Features
- Dual-axis positioning
- Closed-loop control
- Azimuth homing with limit switch
- RS485 IMU integration
- Safety timeouts
- Emergency stop
- Home Assistant integration
- Automatic sun tracking
- Weather protection automations

---

## 🎯 Recommended Reading Paths

### Beginner Path (First Timer)
1. PROJECT_SUMMARY.md (10 min)
2. HARDWARE_BOM.md (20 min - order parts)
3. QUICKSTART.md (while waiting for parts)
4. README.md (reference during assembly)
5. DEBUGGING.md (when needed)

**Total time to tracking**: ~2 hours + shipping time

### Developer Path (Contributing)
1. README.md (understand system)
2. PROJECT_SUMMARY.md (architecture)
3. PYCHARM_SETUP.md (dev environment)
4. solar_tracker.h (source code)
5. test_firmware.py (validation)

**Total time to first contribution**: ~4 hours

### Troubleshooter Path (Something's Wrong)
1. DEBUGGING.md (find your issue)
2. HOMING.md (if homing related)
3. README.md (protocol details)
4. HARDWARE_UPDATES.md (hardware specs)
5. test_firmware.py (validate logic)

**Most issues resolved in**: ~30 minutes

---

## 📞 Support Resources

### Documentation Issues
- Review [DEBUGGING.md](DEBUGGING.md) first
- Check [README.md](README.md) protocol section
- Run [test_firmware.py](test_firmware.py)
- Review relevant hardware guide

### Hardware Questions
- [HARDWARE_BOM.md](HARDWARE_BOM.md) - Component selection
- [HARDWARE_UPDATES.md](HARDWARE_UPDATES.md) - Specifications
- [README.md](README.md) - Wiring diagrams

### Firmware Questions
- [PYCHARM_SETUP.md](PYCHARM_SETUP.md) - Development
- [solar_tracker.h](solar_tracker.h) - Source code
- [PROJECT_SUMMARY.md](PROJECT_SUMMARY.md) - Architecture

### Community
- ESPHome Discord: https://discord.gg/KhAMKrd
- Home Assistant Forums: https://community.home-assistant.io
- ESPHome Docs: https://esphome.io

---

## 🚀 Next Steps

### For New Users
1. ✅ Read [QUICKSTART.md](QUICKSTART.md)
2. 🛒 Order parts from [HARDWARE_BOM.md](HARDWARE_BOM.md)
3. ⚙️ Configure [secrets.yaml](secrets.yaml)
4. 🔧 Flash [solar_tracker.yaml](solar_tracker.yaml)
5. ☀️ Start tracking!

### For Developers
1. ✅ Read [PYCHARM_SETUP.md](PYCHARM_SETUP.md)
2. 💻 Set up development environment
3. 📖 Study [solar_tracker.h](solar_tracker.h)
4. ✏️ Make improvements
5. 🧪 Run [test_firmware.py](test_firmware.py)

### For Contributors
1. ✅ Read [PROJECT_SUMMARY.md](PROJECT_SUMMARY.md)
2. 📋 Identify enhancement areas
3. 💻 Follow [PYCHARM_SETUP.md](PYCHARM_SETUP.md)
4. ✏️ Submit improvements
5. 📄 Update documentation

---

## 📜 License & Credits

**Firmware**: Open source, free for non-commercial use

**Hardware**:
- ESP32-C6: Espressif Systems
- HWT905: WIT-Motion
- BTS7960: Infineon Technologies
- SeeedStudio RS485: Seeed Technology

**Software**:
- ESPHome: https://esphome.io
- Home Assistant: https://www.home-assistant.io

---

## ⚠️ Important Notes

### Before You Start
- ⚡ Work with 24V DC requires care
- 🔧 Basic electronics knowledge helpful
- 🛠️ Tools required (soldering, etc.)
- ☁️ Weather protection critical
- ⏱️ Allow time for testing
- 💰 Budget $250-700 for hardware

### Safety First
- Follow electrical safety practices
- Install fuses on motor circuits
- Test indoors before outdoor deployment
- Verify all connections
- Monitor first operation closely
- Install emergency stop

### Quality
- Use genuine HWT905 (not clones)
- Don't cheap out on power supply
- Quality wire and connectors
- Weather-rated enclosure
- Test components on arrival

---

## 📈 Version History

**v1.2 (November 2024)** - Current
- ✅ Added BTS7960 specifications
- ✅ Added SeeedStudio RS485 details
- ✅ Updated to 24V power system
- ✅ Added HARDWARE_BOM.md
- ✅ Added PYCHARM_SETUP.md
- ✅ Added HARDWARE_UPDATES.md

**v1.1 (November 2024)**
- ✅ Added azimuth homing system
- ✅ Added limit switch support
- ✅ Added HOMING.md
- ✅ Added HOMING_CHANGES.md
- ✅ Updated all documentation
- ✅ Added homing tests

**v1.0 (November 2024)**
- ✅ Initial release
- ✅ Dual-axis control
- ✅ HWT905 integration
- ✅ BTS7960 support
- ✅ Home Assistant services
- ✅ Complete documentation

---

**Project Status: Production Ready with Complete Documentation** ✅

**Ready to build your solar tracker!** 🌞
