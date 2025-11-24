#!/usr/bin/with-contenv bashio

bashio::log.info "Starting Solar Tracker Controller..."

# Get configuration
export ESP32_IP=$(bashio::config 'esp32_ip')
export ESP32_PORT=$(bashio::config 'esp32_port')
export MQTT_BROKER=$(bashio::config 'mqtt_broker')
export MQTT_PORT=$(bashio::config 'mqtt_port')
export MQTT_USER=$(bashio::config 'mqtt_user')
export MQTT_PASSWORD=$(bashio::config 'mqtt_password')
export LATITUDE=$(bashio::config 'latitude')
export LONGITUDE=$(bashio::config 'longitude')
export TIMEZONE=$(bashio::config 'timezone')
export UPDATE_INTERVAL=$(bashio::config 'update_interval')
export PID_KP=$(bashio::config 'pid_kp')
export PID_KI=$(bashio::config 'pid_ki')
export PID_KD=$(bashio::config 'pid_kd')
export MAX_WIND_SPEED=$(bashio::config 'max_wind_speed')
export STOW_AZIMUTH=$(bashio::config 'stow_position_azimuth')
export STOW_ELEVATION=$(bashio::config 'stow_position_elevation')
export TRACKING_MODE=$(bashio::config 'tracking_mode')
export HA_WIND_SPEED_ENTITY=$(bashio::config 'ha_wind_speed_entity')
export HA_WIND_GUST_ENTITY=$(bashio::config 'ha_wind_gust_entity')
export HA_WIND_DIRECTION_ENTITY=$(bashio::config 'ha_wind_direction_entity')
export HA_TEMPERATURE_ENTITY=$(bashio::config 'ha_temperature_entity')
export HA_HUMIDITY_ENTITY=$(bashio::config 'ha_humidity_entity')
export USE_WIND_GUST_FOR_STOW=$(bashio::config 'use_wind_gust_for_stow')

bashio::log.info "ESP32 IP: ${ESP32_IP}"
bashio::log.info "Location: ${LATITUDE}, ${LONGITUDE}"
bashio::log.info "Tracking Mode: ${TRACKING_MODE}"

if bashio::config.has_value 'ha_wind_speed_entity'; then
    bashio::log.info "Using HA wind entity: ${HA_WIND_SPEED_ENTITY}"
fi

# Start the Python application
cd /app
exec python3 -u solar_tracker/main.py
