# Changelog

All notable changes to this project will be documented in this file.

## [1.0.0] - 2024-11-24

### Added
- Initial release of Solar Tracker Controller add-on
- Automatic sun tracking using astronomical calculations
- 9-axis IMU sensor fusion (accelerometer, gyroscope, magnetometer)
- PID control for smooth motor movements
- Multiple tracking modes: Auto, Manual, Stow, Calibrate
- **Home Assistant entity integration for wind and weather data**
  - Support for existing weather stations (e.g., Ecowitt WS90)
  - Configurable wind speed, gust, and direction entities
  - Optional temperature and humidity integration
  - Choose between average or gust wind for stow decisions
- Wind speed monitoring with auto-stow
- Real-time web dashboard
- MQTT integration with Home Assistant
- Home Assistant MQTT discovery for automatic entity creation
- ESP32 HTTP API interface
- Comprehensive sensor calibration procedures
- Emergency stop functionality
- Configurable PID parameters
- Timezone and location-based sun calculations
- Limit switch support
- RS485/Modbus communication support

### Features
- **Control Loop**: 1Hz default update rate (configurable)
- **Web Interface**: Full-featured dashboard on port 8099
- **Safety**: Automatic stow on high wind or communication loss
- **Accuracy**: Complementary filter for drift-free orientation
- **Flexibility**: Support for multiple ESP32 variants

### Documentation
- Complete README with installation instructions
- Hardware requirements and wiring guides
- API documentation for ESP32 firmware
- Calibration procedures
- Troubleshooting guide
- PID tuning guide

## Future Enhancements

### Planned for 1.1.0
- [ ] Historical tracking data logging
- [ ] Performance analytics (tracking accuracy over time)
- [ ] Predictive wind-based positioning
- [ ] Cloud cover detection integration
- [ ] Multiple tracker support
- [ ] Advanced calibration wizard in web UI
- [ ] Custom tracking profiles (summer/winter optimization)

### Planned for 1.2.0
- [ ] Machine learning for optimal positioning
- [ ] Energy production correlation
- [ ] Weather forecast integration
- [ ] Mobile app support
- [ ] Voice control via Alexa/Google Home
- [ ] Advanced data visualization
