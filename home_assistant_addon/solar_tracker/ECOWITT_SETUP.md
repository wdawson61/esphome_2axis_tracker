# Ecowitt WS90 Integration Guide

This guide shows how to configure the Solar Tracker add-on to use wind data from your existing Ecowitt WS90 weather station already integrated in Home Assistant.

## Benefits of Using HA Entities

✅ **No additional hardware** - Use your existing weather station
✅ **More accurate** - Professional weather station vs simple anemometer
✅ **Additional data** - Temperature, humidity, wind direction available
✅ **Centralized** - All weather data in one place
✅ **Reliable** - Proven integration with Home Assistant

## Finding Your Ecowitt Entity IDs

### Step 1: Identify Your Entities

1. Go to **Settings** → **Devices & Services**
2. Find your **Ecowitt** integration
3. Click on it to see all entities
4. Look for entities like:
   - `sensor.ws90_wind_speed`
   - `sensor.ws90_gust_speed`
   - `sensor.ws90_wind_direction`
   - `sensor.ws90_outdoor_temperature`
   - `sensor.ws90_humidity`

### Step 2: Check Entity States

1. Go to **Developer Tools** → **States**
2. Search for "ws90" or "ecowitt"
3. Note the exact entity IDs
4. Verify they show current values

## Common Ecowitt WS90 Entity Names

Your entities may be named like:

```
sensor.ws90_wind_speed          # Average wind speed
sensor.ws90_gust_speed          # Wind gust speed
sensor.ws90_wind_direction      # Wind direction (degrees)
sensor.ws90_outdoor_temperature # Temperature
sensor.ws90_humidity            # Relative humidity
```

Or they might have a different prefix based on your integration:

```
sensor.ecowitt_wind_speed
sensor.outdoor_wind_speed
sensor.weather_station_wind_speed
```

## Configuration

### Basic Configuration (Wind Speed Only)

```yaml
esp32_ip: "192.168.1.100"
esp32_port: 80
mqtt_broker: "core-mosquitto"
mqtt_port: 1883
latitude: 40.7128
longitude: -74.0060
timezone: "America/New_York"

# Wind configuration
ha_wind_speed_entity: "sensor.ws90_wind_speed"
max_wind_speed: 50
use_wind_gust_for_stow: false
```

### Recommended Configuration (Average + Gust)

```yaml
esp32_ip: "192.168.1.100"
esp32_port: 80
mqtt_broker: "core-mosquitto"
mqtt_port: 1883
latitude: 40.7128
longitude: -74.0060
timezone: "America/New_York"

# Wind configuration - Use BOTH average and gust
ha_wind_speed_entity: "sensor.ws90_wind_speed"
ha_wind_gust_entity: "sensor.ws90_gust_speed"
ha_wind_direction_entity: "sensor.ws90_wind_direction"

# Use gust for stow decisions (more conservative)
max_wind_speed: 50
use_wind_gust_for_stow: true
```

### Full Configuration (All Weather Data)

```yaml
esp32_ip: "192.168.1.100"
esp32_port: 80
mqtt_broker: "core-mosquitto"
mqtt_port: 1883
latitude: 40.7128
longitude: -74.0060
timezone: "America/New_York"

# Complete weather integration
ha_wind_speed_entity: "sensor.ws90_wind_speed"
ha_wind_gust_entity: "sensor.ws90_gust_speed"
ha_wind_direction_entity: "sensor.ws90_wind_direction"
ha_temperature_entity: "sensor.ws90_outdoor_temperature"
ha_humidity_entity: "sensor.ws90_humidity"

# Wind safety settings
max_wind_speed: 50
use_wind_gust_for_stow: true

# Other settings
update_interval: 1.0
pid_kp: 1.0
pid_ki: 0.1
pid_kd: 0.05
stow_position_azimuth: 0
stow_position_elevation: 0
tracking_mode: "auto"
```

## Configuration Options Explained

### Wind Speed Entity
```yaml
ha_wind_speed_entity: "sensor.ws90_wind_speed"
```
- **Purpose**: Primary wind speed reading (average)
- **Used for**: Monitoring, logging, display
- **Optional**: Leave empty to use ESP32 sensor instead

### Wind Gust Entity
```yaml
ha_wind_gust_entity: "sensor.ws90_gust_speed"
```
- **Purpose**: Peak wind speed (gust)
- **Used for**: Safety decisions if `use_wind_gust_for_stow: true`
- **Optional**: Leave empty if not available

### Wind Direction Entity
```yaml
ha_wind_direction_entity: "sensor.ws90_wind_direction"
```
- **Purpose**: Wind direction in degrees (0-360)
- **Used for**: Future features, logging
- **Optional**: Not currently used for control decisions

### Temperature Entity
```yaml
ha_temperature_entity: "sensor.ws90_outdoor_temperature"
```
- **Purpose**: Outdoor temperature
- **Used for**: Future features (thermal expansion compensation)
- **Optional**: For monitoring only currently

### Humidity Entity
```yaml
ha_humidity_entity: "sensor.ws90_humidity"
```
- **Purpose**: Relative humidity
- **Used for**: Future features
- **Optional**: For monitoring only currently

### Use Wind Gust for Stow
```yaml
use_wind_gust_for_stow: true
```
- **true**: Use gust speed for stow threshold comparison (more conservative)
- **false**: Use average speed for stow threshold comparison (less conservative)
- **Recommended**: `true` for safety

## How Wind Stow Logic Works

### When `use_wind_gust_for_stow: false` (Average)
```
if average_wind_speed > max_wind_speed:
    stow_tracker()
```

**Example**: Max = 50 mph
- Average wind: 48 mph → Continue tracking
- Average wind: 52 mph → Stow tracker

### When `use_wind_gust_for_stow: true` (Gust)
```
if gust_speed > max_wind_speed:
    stow_tracker()
```

**Example**: Max = 50 mph
- Average: 40 mph, Gust: 52 mph → Stow tracker (more conservative)
- Average: 45 mph, Gust: 48 mph → Continue tracking

**Recommendation**: Use gust for stow to protect against sudden wind spikes.

## Tuning Wind Thresholds

### Conservative (Safer)
```yaml
max_wind_speed: 35
use_wind_gust_for_stow: true
```
- Stows at lower wind speeds
- Better for lightweight trackers
- More frequent stowing

### Moderate (Recommended)
```yaml
max_wind_speed: 50
use_wind_gust_for_stow: true
```
- Balanced approach
- Good for most installations
- Uses gust for safety

### Aggressive (Less Stowing)
```yaml
max_wind_speed: 60
use_wind_gust_for_stow: false
```
- Allows higher winds
- For heavy-duty installations
- More time tracking

## Verifying Configuration

After configuring, check the add-on logs:

```
INFO: Home Assistant interface initialized
INFO: Wind speed entity: sensor.ws90_wind_speed
INFO: Wind gust entity: sensor.ws90_gust_speed
INFO: Wind direction entity: sensor.ws90_wind_direction
INFO: Connected to Home Assistant API
```

### Test Wind Reading

1. Go to web interface (port 8099)
2. Check wind speed display
3. Should match your WS90 reading in HA
4. Verify it updates in real-time

### Test Stow Logic

1. Temporarily lower threshold:
   ```yaml
   max_wind_speed: 5  # Very low for testing
   ```
2. Restart add-on
3. Should immediately stow due to "high wind"
4. Check logs for: "High wind detected: X mph, auto-stowing"
5. Restore normal threshold

## Troubleshooting

### Wind Speed Shows 0

**Check entity ID:**
```yaml
# Wrong (extra character or typo)
ha_wind_speed_entity: "sensor.ws90_windspeed"

# Correct
ha_wind_speed_entity: "sensor.ws90_wind_speed"
```

**Verify entity exists:**
- Go to Developer Tools → States
- Search for your entity
- Copy exact entity ID

### Entity Not Found Error

**Check logs:**
```
WARNING: Entity sensor.ws90_wind_speed returned status 404
```

**Solutions:**
1. Verify Ecowitt integration is working
2. Check entity ID spelling
3. Ensure entity is not disabled
4. Restart Ecowitt integration if needed

### Wind Data Not Updating

**Check entity state:**
- Go to Developer Tools → States
- Find your wind entity
- Check if state is "unknown" or "unavailable"
- If yes, fix Ecowitt integration first

**Check add-on logs:**
```
WARNING: Invalid wind speed value: unavailable
```

### Tracker Not Stowing in High Wind

**Verify wind reading:**
1. Check web interface shows correct wind speed
2. Check logs show wind value
3. Verify threshold is correct

**Check logic:**
```yaml
# If using average but gust is high:
ha_wind_speed_entity: "sensor.ws90_wind_speed"     # 40 mph
ha_wind_gust_entity: "sensor.ws90_gust_speed"      # 55 mph
max_wind_speed: 50
use_wind_gust_for_stow: false  # ← Should be true!
```

**Solution:** Set `use_wind_gust_for_stow: true`

## Example: Complete Working Configuration

```yaml
# Basic settings
esp32_ip: "192.168.1.150"
esp32_port: 80
latitude: 42.3601
longitude: -71.0589
timezone: "America/New_York"

# MQTT
mqtt_broker: "core-mosquitto"
mqtt_port: 1883
mqtt_user: "mqttuser"
mqtt_password: "mqttpassword"

# Ecowitt WS90 Integration
ha_wind_speed_entity: "sensor.ws90_wind_speed"
ha_wind_gust_entity: "sensor.ws90_gust_speed"
ha_wind_direction_entity: "sensor.ws90_wind_direction"
ha_temperature_entity: "sensor.ws90_outdoor_temperature"
ha_humidity_entity: "sensor.ws90_humidity"

# Wind safety
max_wind_speed: 45
use_wind_gust_for_stow: true

# PID control
pid_kp: 1.2
pid_ki: 0.15
pid_kd: 0.08
update_interval: 1.0

# Stow position
stow_position_azimuth: 0
stow_position_elevation: 10

# Initial mode
tracking_mode: "auto"
```

## Benefits of This Integration

### Accuracy
- Professional weather station
- Calibrated sensors
- Multiple data points

### Reliability
- Proven Ecowitt integration
- No additional wiring
- Centralized monitoring

### Flexibility
- Choose average or gust for stow
- Use additional weather data
- Easy to change thresholds

### Monitoring
- All data in Home Assistant
- Historical tracking
- Create custom automations

## Advanced: Using Wind Direction

Future features may include:

- **Directional stowing**: Stow only for winds from certain directions
- **Optimized positioning**: Angle tracker to reduce wind load
- **Wind-aware tracking**: Predict movement based on wind direction

Currently wind direction is logged but not used for control decisions.

## Support

If you encounter issues:
1. Check entity IDs are correct
2. Verify Ecowitt integration is working
3. Review add-on logs
4. Test with temporary low threshold
5. Create GitHub issue with logs and config
