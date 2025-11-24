# Home Assistant Entity Integration Update

## What Changed

The Solar Tracker add-on now supports reading wind and weather data directly from Home Assistant entities instead of requiring a separate wind sensor on the ESP32.

## Key Benefits

✅ **Use existing weather station** - No need for separate wind sensor
✅ **More accurate data** - Professional weather stations like Ecowitt WS90
✅ **Additional sensors** - Temperature, humidity, wind direction available
✅ **Flexible configuration** - Choose average or gust for stow decisions
✅ **Centralized monitoring** - All weather data in Home Assistant

## New Configuration Options

### Wind Entities
```yaml
ha_wind_speed_entity: "sensor.ws90_wind_speed"        # Average wind speed
ha_wind_gust_entity: "sensor.ws90_gust_speed"         # Peak gust speed
ha_wind_direction_entity: "sensor.ws90_wind_direction" # Wind direction
```

### Weather Entities (Optional)
```yaml
ha_temperature_entity: "sensor.ws90_outdoor_temperature"
ha_humidity_entity: "sensor.ws90_humidity"
```

### Stow Behavior
```yaml
use_wind_gust_for_stow: true  # Use gust (more conservative)
                               # or false to use average
```

## How It Works

### Wind Data Priority
1. **If HA entities configured** → Use Home Assistant data
2. **If no HA entities** → Fall back to ESP32 wind sensor
3. **Both available** → HA entities take priority

### Stow Decision Logic

**With `use_wind_gust_for_stow: true`:**
```python
if wind_gust > max_wind_speed:
    stow_tracker()
```
More conservative - stows on peak gusts

**With `use_wind_gust_for_stow: false`:**
```python
if wind_speed_average > max_wind_speed:
    stow_tracker()
```
Less conservative - uses sustained average

## Example Configurations

### Ecowitt WS90 (Recommended)
```yaml
esp32_ip: "192.168.1.100"
latitude: 40.7128
longitude: -74.0060
timezone: "America/New_York"

# Use Ecowitt WS90 for wind data
ha_wind_speed_entity: "sensor.ws90_wind_speed"
ha_wind_gust_entity: "sensor.ws90_gust_speed"
ha_wind_direction_entity: "sensor.ws90_wind_direction"
ha_temperature_entity: "sensor.ws90_outdoor_temperature"
ha_humidity_entity: "sensor.ws90_humidity"

max_wind_speed: 50
use_wind_gust_for_stow: true  # Stow on gusts > 50mph
```

### Generic Weather Integration
```yaml
# Works with any weather integration
ha_wind_speed_entity: "sensor.weather_station_wind_speed"
ha_wind_gust_entity: "sensor.weather_station_gust_speed"
max_wind_speed: 45
use_wind_gust_for_stow: true
```

### ESP32 Wind Sensor (Backward Compatible)
```yaml
# Leave HA entities empty to use ESP32 sensor
ha_wind_speed_entity: ""
ha_wind_gust_entity: ""
max_wind_speed: 50
```

## What's Displayed

### Web Interface
- **Wind Speed**: Shows the value used for monitoring
- **Wind Gust**: Displayed if gust entity configured
- **Wind Direction**: Displayed if direction entity configured

### MQTT Topics (unchanged)
```
solar_tracker/wind_speed     # Published for HA
```

## New Files

### ha_interface.py
New Python module that:
- Connects to Home Assistant API
- Reads entity states
- Handles entity unavailability
- Caches recent values
- Provides wind data to controller

### ECOWITT_SETUP.md
Complete guide covering:
- Finding entity IDs
- Configuration examples
- Troubleshooting
- Tuning wind thresholds
- Testing procedures

## Migration Guide

### If you already have the add-on installed:

1. **Update configuration** - Add HA entity IDs
2. **Restart add-on** - Changes take effect
3. **Check logs** - Verify entity connection
4. **Test wind display** - Check web interface
5. **Verify stow behavior** - Lower threshold temporarily

### Example Migration

**Before:**
```yaml
esp32_ip: "192.168.1.100"
max_wind_speed: 50
# Wind comes from ESP32 sensor
```

**After:**
```yaml
esp32_ip: "192.168.1.100"
max_wind_speed: 50

# Now use Ecowitt data
ha_wind_speed_entity: "sensor.ws90_wind_speed"
ha_wind_gust_entity: "sensor.ws90_gust_speed"
use_wind_gust_for_stow: true
```

## Compatibility

### Tested Weather Integrations
- ✅ Ecowitt (WS90, WS80, etc.)
- ✅ Ambient Weather
- ✅ WeatherFlow Tempest
- ✅ Davis Vantage
- ✅ Generic weather stations

### Requirements
- Entity must report numeric state (not "unknown" or "unavailable")
- Units should be mph (or configurable in future update)
- Entity should update regularly (< 60 seconds recommended)

## Troubleshooting

### Wind shows 0
- Check entity ID is correct
- Verify entity has valid state
- Check add-on logs for errors

### Stow not working
- Verify wind data is being read (check web UI)
- Check `use_wind_gust_for_stow` setting
- Ensure `max_wind_speed` is set correctly
- Test with lower threshold

### Entity not found
- Go to Developer Tools → States
- Search for entity
- Copy exact entity ID
- Verify weather integration is working

## Performance

### API Calls
- One API call per configured entity per control loop
- Default: 1 second update interval
- Minimal impact on Home Assistant

### Caching
- Entity states are cached
- Reduces API load
- Falls back to cache on timeout

## Future Enhancements

Planned features using weather data:
- Wind direction-based positioning
- Temperature compensation
- Humidity-based cleaning decisions
- Weather forecast integration
- Predictive stowing

## Documentation

Full details in:
- [ECOWITT_SETUP.md](ECOWITT_SETUP.md) - Complete setup guide
- [README.md](README.md) - Updated with HA entity info
- [QUICKSTART.md](QUICKSTART.md) - Quick configuration reference

## Support

Questions about:
- **Entity IDs** → See ECOWITT_SETUP.md
- **Configuration** → See README.md
- **Troubleshooting** → Check add-on logs first
- **Issues** → Create GitHub issue with logs

---

## Summary

This update makes the add-on more flexible and eliminates the need for a separate wind sensor when you already have a weather station integrated with Home Assistant. It's fully backward compatible - if you don't configure HA entities, it works exactly as before with the ESP32 sensor.

**Recommended setup**: Use Ecowitt WS90 entities with `use_wind_gust_for_stow: true` for the most conservative and safe wind monitoring.
