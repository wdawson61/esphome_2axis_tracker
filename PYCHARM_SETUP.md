# PyCharm Development Environment Setup for ESPHome & Home Assistant

Complete guide for setting up a professional development environment using PyCharm for debugging ESPHome firmware and Home Assistant integrations.

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Installing PyCharm](#installing-pycharm)
3. [Setting Up ESPHome Development](#setting-up-esphome-development)
4. [Setting Up Home Assistant Development](#setting-up-home-assistant-development)
5. [Remote Debugging ESPHome Devices](#remote-debugging-esphome-devices)
6. [Debugging C++ Components](#debugging-c-components)
7. [Best Practices](#best-practices)
8. [Common Issues](#common-issues)

---

## Prerequisites

### System Requirements
- **OS**: Linux (Ubuntu 22.04+), macOS, or Windows 10/11
- **RAM**: 8GB minimum, 16GB recommended
- **Disk**: 10GB free space
- **Python**: 3.9+ (3.11 recommended)

### Software to Install

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install python3 python3-pip python3-venv git build-essential

# macOS (with Homebrew)
brew install python3 git

# Windows
# Download Python from python.org and Git from git-scm.com
```

---

## Installing PyCharm

### Option 1: PyCharm Professional (Recommended)

PyCharm Professional includes better support for remote development, Docker, and database tools.

**Free for:**
- Students (via GitHub Student Developer Pack)
- Open source projects
- Educational institutions

**Download:** https://www.jetbrains.com/pycharm/download/

```bash
# Linux - using JetBrains Toolbox (recommended)
# Download from: https://www.jetbrains.com/toolbox-app/

# Or via snap (Ubuntu)
sudo snap install pycharm-professional --classic

# macOS
brew install --cask pycharm

# Windows
# Download installer from JetBrains website
```

### Option 2: PyCharm Community Edition (Free)

Sufficient for basic ESPHome/HA development.

```bash
# Linux
sudo snap install pycharm-community --classic

# macOS
brew install --cask pycharm-ce

# Windows
# Download from JetBrains website
```

---

## Setting Up ESPHome Development

### 1. Clone ESPHome Repository

```bash
cd ~/projects
git clone https://github.com/esphome/esphome.git
cd esphome
```

### 2. Create Python Virtual Environment

```bash
# Create virtual environment
python3 -m venv venv

# Activate it
# Linux/macOS:
source venv/bin/activate
# Windows:
venv\Scripts\activate

# Install ESPHome in development mode
pip install -e .
pip install pylint black flake8 mypy
```

### 3. Open Project in PyCharm

1. **Launch PyCharm**
2. **Open** → Select `esphome` directory
3. **Configure Python Interpreter:**
   - File → Settings (Ctrl+Alt+S)
   - Project: esphome → Python Interpreter
   - Add Interpreter → Existing Environment
   - Select `venv/bin/python` (or `venv\Scripts\python.exe` on Windows)
   - Apply

### 4. Configure ESPHome Project Structure

```
esphome/
├── esphome/           # Core library
│   ├── components/    # Component implementations
│   ├── core/         # Core functionality
│   └── ...
├── script/           # Helper scripts
├── tests/            # Unit tests
└── your_configs/     # Your YAML configs (create this)
    └── solar_tracker.yaml
```

Create a directory for your configurations:

```bash
mkdir ~/projects/esphome/your_configs
cp /path/to/solar_tracker.yaml ~/projects/esphome/your_configs/
cp /path/to/solar_tracker.h ~/projects/esphome/your_configs/
```

### 5. Create Run Configuration for ESPHome

**Method 1: Using PyCharm Run Configurations**

1. Run → Edit Configurations
2. Click **+** → Python
3. Configure:
   - **Name**: `ESPHome Run`
   - **Script path**: Select `esphome/__main__.py`
   - **Parameters**: `run your_configs/solar_tracker.yaml`
   - **Working directory**: `~/projects/esphome`
   - **Python interpreter**: Select your venv

4. Click **+** → Python (for compilation only)
5. Configure:
   - **Name**: `ESPHome Compile`
   - **Script path**: Select `esphome/__main__.py`
   - **Parameters**: `compile your_configs/solar_tracker.yaml`

**Method 2: Using Terminal within PyCharm**

1. View → Tool Windows → Terminal
2. Run commands directly:
   ```bash
   esphome compile your_configs/solar_tracker.yaml
   esphome run your_configs/solar_tracker.yaml
   esphome logs your_configs/solar_tracker.yaml
   ```

### 6. Enable Code Assistance for C++

For editing custom C++ components (like `solar_tracker.h`):

1. Install **C/C++** plugin (Professional only) or use **CLion**
2. Settings → Plugins → Marketplace → Search "C/C++"
3. Install and restart

For Community Edition users:
- Use external editor for C++ (VS Code with C++ extension)
- Or switch to CLion for C++ development

### 7. Set Up Debugging with Logs

Create a debug configuration:

**File: `.run/ESPHome Debug.run.xml`**
```xml
<component name="ProjectRunConfigurationManager">
  <configuration default="false" name="ESPHome Debug" type="PythonConfigurationType">
    <module name="esphome" />
    <option name="INTERPRETER_OPTIONS" value="" />
    <option name="PARENT_ENVS" value="true" />
    <envs>
      <env name="PYTHONUNBUFFERED" value="1" />
      <env name="ESPHOME_LOG_LEVEL" value="DEBUG" />
    </envs>
    <option name="SDK_HOME" value="" />
    <option name="WORKING_DIRECTORY" value="$PROJECT_DIR$" />
    <option name="IS_MODULE_SDK" value="true" />
    <option name="ADD_CONTENT_ROOTS" value="true" />
    <option name="ADD_SOURCE_ROOTS" value="true" />
    <option name="SCRIPT_NAME" value="$PROJECT_DIR$/esphome/__main__.py" />
    <option name="PARAMETERS" value="logs your_configs/solar_tracker.yaml" />
  </configuration>
</component>
```

---

## Setting Up Home Assistant Development

### 1. Clone Home Assistant Core

```bash
cd ~/projects
git clone https://github.com/home-assistant/core.git ha-core
cd ha-core
```

### 2. Create Development Environment

```bash
# Create virtual environment
python3 -m venv venv
source venv/bin/activate  # Linux/macOS
# venv\Scripts\activate     # Windows

# Install Home Assistant in development mode
pip install -e .
pip install pre-commit pytest pylint mypy
```

### 3. Set Up Home Assistant Config Directory

```bash
mkdir -p ~/ha-config
cd ~/ha-config

# Create basic configuration.yaml
cat > configuration.yaml << 'EOF'
# Home Assistant Configuration
homeassistant:
  name: Development
  latitude: 40.7128
  longitude: -74.0060
  elevation: 10
  unit_system: metric
  time_zone: America/New_York

# Enable default integrations
default_config:

# Enable API
api:

# Enable frontend
frontend:

# Enable ESPHome
esphome:

# Text to speech
tts:
  - platform: google_translate

# Logger
logger:
  default: info
  logs:
    homeassistant.components.esphome: debug
EOF
```

### 4. Open Home Assistant in PyCharm

1. **Open Project** → Select `ha-core` directory
2. **Configure Interpreter** → Select `venv/bin/python`
3. **Mark Directories:**
   - Right-click `homeassistant` → Mark Directory as → Sources Root
   - Right-click `tests` → Mark Directory as → Test Sources Root

### 5. Create Run Configurations

**Run Configuration for Home Assistant:**

1. Run → Edit Configurations → **+** → Python
2. Configure:
   - **Name**: `Home Assistant`
   - **Script path**: `~/projects/ha-core/homeassistant/__main__.py`
   - **Parameters**: `--config ~/ha-config -v`
   - **Working directory**: `~/projects/ha-core`
   - **Environment variables**: `PYTHONPATH=~/projects/ha-core`

3. Apply and run with **Shift+F10**

**Debug Configuration:**

Same as above, but click **Debug** icon (Shift+F9) instead of Run.

### 6. Integrate Your Solar Tracker

Copy your automation files:

```bash
cp /path/to/home_assistant_automations.yaml ~/ha-config/automations.yaml
```

Add to `configuration.yaml`:
```yaml
automation: !include automations.yaml
```

### 7. Set Breakpoints

1. Open any Python file in Home Assistant
2. Click in the gutter (left of line numbers) to set breakpoint
3. Run in Debug mode
4. Code will pause at breakpoints
5. Inspect variables in **Debug** panel

---

## Remote Debugging ESPHome Devices

### Method 1: Serial Logging (Easiest)

**Direct USB Connection:**

```python
# Create run configuration
# Script: esphome/__main__.py
# Parameters: logs your_configs/solar_tracker.yaml --device /dev/ttyUSB0
```

**In PyCharm Terminal:**
```bash
esphome logs solar_tracker.yaml --device /dev/ttyUSB0
```

**Filtering Logs:**
```bash
# Show only motor controller logs
esphome logs solar_tracker.yaml | grep MotorController

# Show only errors and warnings
esphome logs solar_tracker.yaml | grep -E '\[W\]|\[E\]'
```

### Method 2: OTA Logging (Wireless)

After initial USB flash:

```python
# Parameters: logs your_configs/solar_tracker.yaml --device solar-tracker.local
```

Or:
```bash
esphome logs solar_tracker.yaml --device 192.168.1.XXX
```

### Method 3: MQTT Logging for Persistence

Add to your `solar_tracker.yaml`:

```yaml
logger:
  level: DEBUG
  logs:
    component: DEBUG

# Add MQTT logger
mqtt:
  broker: your-mqtt-broker.local
  username: !secret mqtt_username
  password: !secret mqtt_password
  log_topic: esphome/solar_tracker/log
```

**Subscribe in PyCharm Terminal:**
```bash
mosquitto_sub -h your-mqtt-broker.local -t "esphome/solar_tracker/log" -u username -P password
```

### Method 4: Web Server for Real-Time Logs

Add to YAML:
```yaml
web_server:
  port: 80
  log: true
```

Access logs at: `http://solar-tracker.local/logs`

---

## Debugging C++ Components

### Option 1: Serial Print Debugging (Easiest)

Add debug prints in your `solar_tracker.h`:

```cpp
void home_azimuth() {
    ESP_LOGD("DEBUG", "home_azimuth() called");
    ESP_LOGD("DEBUG", "Current homing_active_: %d", homing_active_);
    
    // Your code...
    
    ESP_LOGD("DEBUG", "Homing phase: %d", homing_phase_);
}
```

**Log Levels:**
- `ESP_LOGV` - Verbose (most detailed)
- `ESP_LOGD` - Debug
- `ESP_LOGI` - Info
- `ESP_LOGW` - Warning
- `ESP_LOGE` - Error

### Option 2: JTAG Debugging (Advanced)

**Hardware Required:**
- ESP-PROG or similar JTAG debugger
- Connection to ESP32-C6 JTAG pins

**Setup PlatformIO in PyCharm:**

1. Install **PlatformIO** plugin
2. Settings → Plugins → Marketplace → "PlatformIO"
3. Restart PyCharm

**Configure platformio.ini:**
```ini
[env:esp32-c6]
platform = espressif32
board = esp32-c6-devkitc-1
framework = arduino

debug_tool = esp-prog
debug_init_break = tbreak setup
debug_build_flags = -O0 -g -ggdb
```

**Debug Commands in PlatformIO:**
```bash
# Build with debug symbols
pio run -t debug

# Upload and start debug session
pio debug --interface=gdb -x .pioinit
```

### Option 3: Simulator (For Logic Testing)

Create a Python mock for testing logic:

**File: `test_solar_tracker_logic.py`**
```python
"""Test solar tracker logic without hardware"""

class MockHWT905:
    def __init__(self):
        self.heading = 237.0
        self.elevation = 0.0
    
    def get_heading(self):
        return self.heading
    
    def get_elevation(self):
        return self.elevation

class MockMotorController:
    def __init__(self):
        self.azimuth_homed = False
        self.home_offset = 0.0
        self.current_azimuth = 0.0
    
    def home_azimuth(self, hwt905):
        print("Starting homing...")
        # Simulate finding switch at current position
        self.home_offset = hwt905.get_heading()
        self.azimuth_homed = True
        print(f"Home offset set: {self.home_offset}°")
    
    def get_corrected_azimuth(self, hwt905):
        if not self.azimuth_homed:
            raise Exception("Not homed!")
        
        raw = hwt905.get_heading()
        corrected = raw - self.home_offset
        
        # Normalize
        while corrected < 0:
            corrected += 360
        while corrected >= 360:
            corrected -= 360
        
        return corrected

# Test
if __name__ == "__main__":
    sensor = MockHWT905()
    controller = MockMotorController()
    
    print(f"Initial heading: {sensor.get_heading()}°")
    
    controller.home_azimuth(sensor)
    print(f"Corrected position: {controller.get_corrected_azimuth(sensor)}°")
    
    # Simulate rotation
    sensor.heading = 327.0
    print(f"After rotation - Raw: {sensor.heading}°, Corrected: {controller.get_corrected_azimuth(sensor)}°")
```

**Run in PyCharm:**
- Right-click file → Run
- Set breakpoints to step through logic
- Inspect variables without flashing hardware

---

## Best Practices

### 1. Use Version Control

```bash
# Initialize git in your config directory
cd ~/projects/esphome/your_configs
git init
git add solar_tracker.yaml solar_tracker.h secrets.yaml.example
git commit -m "Initial solar tracker configuration"

# Add .gitignore
cat > .gitignore << 'EOF'
secrets.yaml
.esphome/
*.bin
EOF
```

**In PyCharm:**
- VCS → Enable Version Control Integration → Git
- Use built-in Git tools (Commit, Push, Pull)

### 2. Use Code Inspection

Enable PyCharm inspections:
- Settings → Editor → Inspections
- Enable:
  - Python → PEP 8 coding style violation
  - Python → Type checker
  - Spelling

### 3. Use TODO Tracking

Add TODO comments:
```python
# TODO: Add PWM speed control for motors
# FIXME: Azimuth sometimes overshoots by 2°
# NOTE: Homing takes 1-3 minutes depending on start position
```

View all TODOs:
- View → Tool Windows → TODO

### 4. Create Custom Live Templates

Settings → Editor → Live Templates → Python

**Example Template: `esphlog`**
```python
ESP_LOGD("$TAG$", "$MESSAGE$");
$END$
```

Usage: Type `esphlog` + Tab → fills template

### 5. Use ESPHome Validation

Before flashing:
```bash
# Validate YAML
esphome config solar_tracker.yaml

# Check for issues
esphome compile --only-generate solar_tracker.yaml
```

### 6. Set Up File Watchers

Automatically validate YAML on save:

1. Settings → Tools → File Watchers
2. **+** → Custom
3. Configure:
   - **Name**: ESPHome Validator
   - **File type**: YAML
   - **Scope**: Project Files
   - **Program**: `$ProjectFileDir$/venv/bin/esphome`
   - **Arguments**: `config $FilePath$`
   - **Output paths**: `$ProjectFileDir$/.esphome/build`

---

## Common Issues

### Issue 1: Import Errors

**Problem:**
```python
ImportError: No module named 'esphome'
```

**Solution:**
1. Verify virtual environment is activated
2. Reinstall ESPHome: `pip install -e .`
3. Invalidate caches: File → Invalidate Caches → Invalidate and Restart

### Issue 2: Can't Connect to Device

**Problem:**
```
ERROR Error connecting to solar-tracker.local
```

**Solution:**
```bash
# Find IP address
arp -a | grep esp32

# Or use mDNS
avahi-browse -rt _esphomelib._tcp

# Use IP directly
esphome logs solar_tracker.yaml --device 192.168.1.XXX
```

### Issue 3: C++ Syntax Errors in PyCharm

**Problem:**
C++ code shows red underlines but compiles fine

**Solution:**
- PyCharm Community doesn't fully support C++
- Install CLion (C++ IDE from JetBrains)
- Or use VS Code for C++ editing
- Or ignore highlighting (code works if ESPHome compiles)

### Issue 4: Slow Indexing

**Problem:**
PyCharm is slow when opening project

**Solution:**
1. Settings → Project Structure
2. Exclude directories:
   - `.esphome`
   - `venv`
   - `build`
3. Apply → OK

### Issue 5: Breakpoints Not Hit

**Problem:**
Debugger doesn't stop at breakpoints in HA

**Solution:**
1. Ensure running in Debug mode (Shift+F9)
2. Check breakpoint is on executable line (not comment)
3. Verify Python interpreter matches venv
4. Try: Run → Debug → Attach to Process

---

## Advanced: Remote Development

### SSH to Raspberry Pi Running HA

**PyCharm Professional Only**

1. Tools → Deployment → Configuration
2. **+** → SFTP
3. Configure:
   - **Host**: raspberry-pi.local
   - **Port**: 22
   - **Username**: pi
   - **Password**: (or use SSH key)
   - **Root path**: /home/pi/homeassistant

4. Tools → Deployment → Automatic Upload → Always

Now edits automatically sync to Pi!

### Docker Development

**PyCharm Professional Only**

1. Settings → Build, Execution, Deployment → Docker
2. **+** → Docker
3. Select connection type (Unix socket / TCP)

**Run HA in Docker:**
```yaml
# docker-compose.yml
version: '3'
services:
  homeassistant:
    image: homeassistant/home-assistant:latest
    volumes:
      - ./config:/config
    ports:
      - "8123:8123"
    environment:
      - TZ=America/New_York
```

Run from PyCharm:
- Right-click `docker-compose.yml` → Run

---

## Workflow Example: Making Changes

### Scenario: Adding PWM Speed Control

**Step 1: Edit C++ Component**

Open `solar_tracker.h` in PyCharm:

```cpp
// Add PWM setup in setup()
ledcSetup(0, 5000, 8);  // Channel 0, 5kHz, 8-bit
ledcAttachPin(elevation_forward_pin_, 0);

// Modify motor control
void run_elevation_forward(uint8_t speed = 255) {
    ESP_LOGD("MotorController", "Running forward at speed: %d", speed);
    ledcWrite(0, speed);
}
```

**Step 2: Validate YAML**

Terminal in PyCharm:
```bash
esphome config solar_tracker.yaml
```

**Step 3: Compile**

```bash
esphome compile solar_tracker.yaml
```

Watch for C++ compilation errors in output.

**Step 4: Flash Device**

```bash
esphome run solar_tracker.yaml --device /dev/ttyUSB0
```

**Step 5: Monitor Logs**

```bash
esphome logs solar_tracker.yaml
```

Look for your debug messages.

**Step 6: Test in Home Assistant**

1. Run HA in debug mode
2. Call service to test motor
3. Check logs for speed messages

**Step 7: Commit Changes**

```bash
git add solar_tracker.h
git commit -m "Add PWM speed control to elevation motor"
git push
```

---

## Useful PyCharm Shortcuts

| Action | Windows/Linux | macOS |
|--------|---------------|-------|
| Run | Shift+F10 | ⌃R |
| Debug | Shift+F9 | ⌃D |
| Find in Files | Ctrl+Shift+F | ⌘⇧F |
| Go to Definition | Ctrl+B | ⌘B |
| Find Usages | Alt+F7 | ⌥F7 |
| Refactor Rename | Shift+F6 | ⇧F6 |
| Format Code | Ctrl+Alt+L | ⌘⌥L |
| Terminal | Alt+F12 | ⌥F12 |
| Commit | Ctrl+K | ⌘K |
| Push | Ctrl+Shift+K | ⌘⇧K |

---

## Additional Resources

### Documentation
- **ESPHome**: https://esphome.io
- **Home Assistant**: https://developers.home-assistant.io
- **PyCharm**: https://www.jetbrains.com/help/pycharm/

### Communities
- **ESPHome Discord**: https://discord.gg/KhAMKrd
- **Home Assistant Discord**: https://discord.gg/home-assistant
- **Home Assistant Forums**: https://community.home-assistant.io

### Video Tutorials
- Everything Smart Home (YouTube)
- The Hook Up (YouTube)
- Home Assistant Official (YouTube)

---

## Summary

You now have a complete development environment:

✅ **PyCharm** configured for Python development  
✅ **ESPHome** repository for firmware development  
✅ **Home Assistant** core for integration testing  
✅ **Debugging tools** for C++ and Python  
✅ **Version control** with Git  
✅ **Remote logging** via OTA and MQTT  

**Next Steps:**
1. Clone ESPHome and HA repositories
2. Set up virtual environments
3. Configure run/debug configurations
4. Start making changes to your solar tracker!
5. Use breakpoints and logging for debugging

**Happy Developing! 🚀**
