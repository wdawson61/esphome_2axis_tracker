# Quick Start Guide

This guide will help you get your 2-axis solar tracker up and running quickly.

## Prerequisites

- ESP32-C6 development board
- WIT-Motion HWT905 IMU sensor
- RS485 transceiver module (if not built into HWT905)
- 2x H-bridge motor driver boards (e.g., L298N, DRV8833)
- Linear actuator for elevation control
- 5V power supply (sufficient for ESP32 and motors)
- USB-C cable for programming

## Hardware Setup

### 1. RS485 Connection (WIT-Motion HWT905)

Connect the RS485 transceiver to the ESP32-C6:

```
RS485 Module    ESP32-C6
TX/TXD    -->   GPIO16
RX/RXD    <--   GPIO17
VCC       -->   3.3V or 5V (check module specs)
GND       -->   GND
```

Connect the WIT-Motion HWT905 to the RS485 module according to its datasheet.

### 2. Motor Driver Connections

**Elevation Motor (H-Bridge 1):**
```
H-Bridge        ESP32-C6
IN1       <--   GPIO4 (Forward)
IN2       <--   GPIO5 (Backward)
VCC       -->   Motor Power Supply (e.g., 12V)
GND       -->   Common GND
OUT1/OUT2 -->   Linear Actuator Motor
```

**Azimuth Motor (H-Bridge 2) - Optional:**
```
H-Bridge        ESP32-C6
IN1       <--   GPIO6 (Forward)
IN2       <--   GPIO7 (Backward)
VCC       -->   Motor Power Supply
GND       -->   Common GND
OUT1/OUT2 -->   Azimuth Motor
```

### 3. Power Supply

- Use a 5V power supply for the ESP32-C6 (can use USB-C)
- Use an appropriate voltage for your motors (typically 12V for linear actuators)
- **Important:** Connect all ground pins together (common ground)
- Consider using separate power supplies for logic (ESP32) and motors for noise isolation

## Software Setup

### 1. Install ESPHome

```bash
pip install esphome
```

### 2. Configure WiFi and Credentials

```bash
cd esphome_2axis_tracker
cp secrets.yaml.template secrets.yaml
```

Edit `secrets.yaml` with your information:
```yaml
wifi_ssid: "YourWiFiSSID"
wifi_password: "YourWiFiPassword"
ap_password: "FallbackAPPassword"
api_encryption_key: "generate-with-esphome"
ota_password: "YourOTAPassword"
```

To generate a secure API encryption key:
```bash
python3 -c "import secrets; import base64; print(base64.b64encode(secrets.token_bytes(32)).decode())"
```

### 3. Customize Pin Assignments (if needed)

Edit `solar_tracker.yaml` if you need to change GPIO pins:

```yaml
# Find the uart section and modify:
uart:
  tx_pin: GPIO16  # Change if needed
  rx_pin: GPIO17  # Change if needed

# Find the output section and modify:
output:
  - platform: gpio
    pin: GPIO4    # Elevation forward pin
    id: elevation_motor_forward
  
  - platform: gpio
    pin: GPIO5    # Elevation backward pin
    id: elevation_motor_backward
```

### 4. Compile and Upload

Connect your ESP32-C6 via USB-C cable, then:

```bash
esphome run solar_tracker.yaml
```

ESPHome will:
1. Compile the firmware
2. Detect your connected ESP32-C6
3. Upload the firmware
4. Show you the logs

**First-time upload:** You'll need to use USB cable. After that, you can use OTA (Over-The-Air) updates via WiFi.

## Testing

### 1. Check Logs

After uploading, keep the USB cable connected and watch the logs:

```bash
esphome logs solar_tracker.yaml
```

You should see:
- WiFi connection status
- API connection confirmation
- Sensor data from WIT-Motion HWT905 (acceleration, heading, etc.)

### 2. Access Web Interface

Once connected to WiFi, open a web browser and navigate to:
```
http://solar-tracker.local
```

Or use the IP address shown in the logs.

### 3. Test Motor Control

In the web interface:
1. Find the "Target Elevation Angle" input
2. Enter a small value (e.g., 5 degrees)
3. The elevation motor should run briefly
4. Use the "Stop Elevation Motor" button if needed

### 4. Monitor Sensors

Check that you're receiving data from the WIT-Motion HWT905:
- Acceleration X, Y, Z
- Angular Velocity X, Y, Z
- Roll, Pitch, Heading
- Temperature

## Integration with Home Assistant

### 1. Add Device

In Home Assistant:
1. Go to Settings → Devices & Services
2. Click "+ ADD INTEGRATION"
3. Search for "ESPHome"
4. Enter the IP address or hostname: `solar-tracker.local`
5. Enter the API encryption key from your `secrets.yaml`

### 2. Use the Device

After adding, you'll have access to:
- All sensor entities (acceleration, heading, etc.)
- Number entity for setting elevation angle
- Button for stopping the motor
- Text sensor showing tracker status

### 3. Create Automations

Example automation to track the sun:
```yaml
automation:
  - alias: "Solar Tracker - Update Elevation"
    trigger:
      - platform: time_pattern
        minutes: '/5'  # Every 5 minutes
    action:
      - service: number.set_value
        target:
          entity_id: number.solar_tracker_target_elevation_angle
        data:
          value: "{{ state_attr('sun.sun', 'elevation') }}"
```

## Troubleshooting

### No WiFi Connection
- Check SSID and password in `secrets.yaml`
- Look for the fallback AP: "Solar-Tracker Fallback"
- Connect to it with the AP password from `secrets.yaml`

### No Sensor Data
- Check RS485 wiring
- Verify baud rate (115200)
- Enable verbose logging in `solar_tracker.yaml`:
  ```yaml
  logger:
    level: VERBOSE
  ```

### Motor Not Working
- Check H-bridge connections
- Verify motor power supply
- Check GPIO pin assignments
- Test with small angle changes first (1-5 degrees)

### Motor Runs Too Fast/Slow
Edit the timing calculation in `solar_tracker.yaml`:
```yaml
# In the set_elevation_angle script, find:
float run_time_ms = abs(angle_diff) * 1000.0;  # 1 degree per second

# Adjust for your actuator speed:
float run_time_ms = abs(angle_diff) * 500.0;   # 2 degrees per second
float run_time_ms = abs(angle_diff) * 2000.0;  # 0.5 degrees per second
```

## Calibration

1. **Zero Position:** Manually move the tracker to a known zero position (e.g., horizontal)
2. **Test Movement:** Set small angle increments (1-5 degrees) and verify movement
3. **Adjust Timing:** Fine-tune the motor run time calculation based on observed movement
4. **Limits:** Consider adding physical limit switches for safety

## Safety Notes

- **Never exceed motor voltage ratings**
- **Use appropriate wire gauge for motor currents**
- **Add limit switches to prevent mechanical damage**
- **Test in manual mode before automation**
- **Monitor initial runs carefully**
- **Ensure adequate ventilation for motor drivers**

## Next Steps

- Implement azimuth control (similar to elevation)
- Add limit switches for safety
- Integrate with sun position calculations
- Add weather-based positioning (retract during high winds)
- Implement automatic calibration routines

## Support

For issues or questions:
- Check the main README.md for detailed documentation
- Review ESPHome documentation: https://esphome.io/
- Open an issue on the GitHub repository
