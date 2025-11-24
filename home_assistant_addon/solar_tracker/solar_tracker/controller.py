"""
Solar Tracker Controller
Handles sun position calculation, sensor fusion, PID control, and ESP32 communication
"""

import asyncio
import logging
import os
from datetime import datetime, timezone
from typing import Dict, Optional, Tuple
import json

import pytz
from astral import LocationInfo
from astral.sun import sun, azimuth, elevation
from simple_pid import PID
import numpy as np

from esp32_interface import ESP32Interface
from mqtt_interface import MQTTInterface
from sensor_fusion import SensorFusion
from ha_interface import HomeAssistantInterface

logger = logging.getLogger(__name__)


class TrackingMode:
    """Tracking mode enumeration"""
    MANUAL = "manual"
    AUTO = "auto"
    STOW = "stow"
    CALIBRATE = "calibrate"


class SolarTrackerController:
    """Main controller for solar tracker system"""
    
    def __init__(self):
        # Configuration from environment
        self.latitude = float(os.getenv('LATITUDE', 40.7128))
        self.longitude = float(os.getenv('LONGITUDE', -74.0060))
        self.timezone_str = os.getenv('TIMEZONE', 'America/New_York')
        self.update_interval = float(os.getenv('UPDATE_INTERVAL', 1.0))
        
        # PID parameters
        self.pid_kp = float(os.getenv('PID_KP', 1.0))
        self.pid_ki = float(os.getenv('PID_KI', 0.1))
        self.pid_kd = float(os.getenv('PID_KD', 0.05))
        
        # Safety parameters
        self.max_wind_speed = float(os.getenv('MAX_WIND_SPEED', 50))
        self.stow_azimuth = float(os.getenv('STOW_AZIMUTH', 0))
        self.stow_elevation = float(os.getenv('STOW_ELEVATION', 0))
        
        # Initialize timezone
        self.timezone = pytz.timezone(self.timezone_str)
        self.location = LocationInfo(
            name="Tracker",
            region="",
            timezone=self.timezone_str,
            latitude=self.latitude,
            longitude=self.longitude
        )
        
        # Initialize PID controllers
        self.azimuth_pid = PID(
            self.pid_kp, self.pid_ki, self.pid_kd,
            setpoint=0,
            output_limits=(-100, 100)
        )
        self.elevation_pid = PID(
            self.pid_kp, self.pid_ki, self.pid_kd,
            setpoint=0,
            output_limits=(-100, 100)
        )
        
        # State variables
        self.tracking_mode = os.getenv('TRACKING_MODE', TrackingMode.AUTO)
        self.current_azimuth = 0.0
        self.current_elevation = 0.0
        self.target_azimuth = 0.0
        self.target_elevation = 0.0
        self.wind_speed = 0.0
        self.is_stowed = False
        self.is_moving = False
        self.last_update = None
        
        # Component interfaces
        self.esp32 = None
        self.mqtt = None
        self.ha_interface = None
        self.sensor_fusion = SensorFusion()
        
        # Status data
        self.status = {
            'mode': self.tracking_mode,
            'current_azimuth': 0.0,
            'current_elevation': 0.0,
            'target_azimuth': 0.0,
            'target_elevation': 0.0,
            'sun_azimuth': 0.0,
            'sun_elevation': 0.0,
            'wind_speed': 0.0,
            'is_stowed': False,
            'is_moving': False,
            'error_azimuth': 0.0,
            'error_elevation': 0.0,
            'last_update': None,
            'sensor_data': {}
        }
        
        logger.info(f"Controller initialized at {self.latitude}, {self.longitude}")
    
    async def initialize(self):
        """Initialize all subsystems"""
        logger.info("Initializing subsystems...")
        
        # Initialize ESP32 interface
        self.esp32 = ESP32Interface(
            os.getenv('ESP32_IP', '192.168.1.100'),
            int(os.getenv('ESP32_PORT', 80))
        )
        await self.esp32.connect()
        
        # Initialize Home Assistant interface
        self.ha_interface = HomeAssistantInterface()
        await self.ha_interface.connect()
        
        # Initialize MQTT interface
        self.mqtt = MQTTInterface(
            broker=os.getenv('MQTT_BROKER', 'core-mosquitto'),
            port=int(os.getenv('MQTT_PORT', 1883)),
            username=os.getenv('MQTT_USER', ''),
            password=os.getenv('MQTT_PASSWORD', '')
        )
        await self.mqtt.connect()
        
        # Subscribe to command topics
        await self.mqtt.subscribe('solar_tracker/command/mode', self.handle_mode_command)
        await self.mqtt.subscribe('solar_tracker/command/move', self.handle_move_command)
        await self.mqtt.subscribe('solar_tracker/command/stow', self.handle_stow_command)
        
        logger.info("Subsystems initialized")
    
    async def run(self):
        """Main control loop"""
        logger.info("Starting main control loop")
        
        while True:
            try:
                # Update sensor data
                await self.update_sensors()
                
                # Calculate sun position
                sun_position = self.calculate_sun_position()
                
                # Update targets based on mode
                await self.update_targets(sun_position)
                
                # Execute control logic
                await self.execute_control()
                
                # Publish status
                await self.publish_status()
                
                # Update timestamp
                self.last_update = datetime.now(timezone.utc).isoformat()
                
                # Wait for next update
                await asyncio.sleep(self.update_interval)
                
            except Exception as e:
                logger.error(f"Error in control loop: {e}", exc_info=True)
                await asyncio.sleep(self.update_interval)
    
    async def update_sensors(self):
        """Read and fuse sensor data from ESP32, get wind from Home Assistant"""
        try:
            # Get raw sensor data from ESP32
            sensor_data = await self.esp32.get_sensor_data()
            
            if sensor_data:
                # Extract IMU data
                accel = sensor_data.get('accelerometer', {})
                gyro = sensor_data.get('gyroscope', {})
                mag = sensor_data.get('magnetometer', {})
                
                # Fuse sensor data to get orientation
                orientation = self.sensor_fusion.update(
                    accel_x=accel.get('x', 0),
                    accel_y=accel.get('y', 0),
                    accel_z=accel.get('z', 0),
                    gyro_x=gyro.get('x', 0),
                    gyro_y=gyro.get('y', 0),
                    gyro_z=gyro.get('z', 0),
                    mag_x=mag.get('x', 0),
                    mag_y=mag.get('y', 0),
                    mag_z=mag.get('z', 0),
                    dt=self.update_interval
                )
                
                # Update current position
                self.current_azimuth = orientation['azimuth']
                self.current_elevation = orientation['elevation']
                
                # Update status
                self.status['sensor_data'] = sensor_data
                self.status['current_azimuth'] = self.current_azimuth
                self.status['current_elevation'] = self.current_elevation
            
            # Get wind data from Home Assistant if configured
            if self.ha_interface and self.ha_interface.has_wind_entities():
                wind_data = await self.ha_interface.get_wind_data()
                
                # Use the configured wind value (average or gust)
                self.wind_speed = self.ha_interface.get_wind_speed_for_stow_check(wind_data)
                
                # Store all wind data in status
                self.status['wind_speed'] = self.wind_speed
                self.status['wind_data'] = wind_data
                
                # Get additional weather data if configured
                weather_data = await self.ha_interface.get_weather_data()
                if weather_data.get('temperature') is not None:
                    self.status['temperature'] = weather_data['temperature']
                if weather_data.get('humidity') is not None:
                    self.status['humidity'] = weather_data['humidity']
            else:
                # Fallback to ESP32 wind sensor if available
                if sensor_data:
                    self.wind_speed = sensor_data.get('wind_speed', 0)
                    self.status['wind_speed'] = self.wind_speed
                
        except Exception as e:
            logger.error(f"Error updating sensors: {e}")
    
    def calculate_sun_position(self) -> Dict[str, float]:
        """Calculate current sun position"""
        try:
            now = datetime.now(self.timezone)
            
            # Calculate sun azimuth and elevation
            sun_az = azimuth(self.location.observer, now)
            sun_el = elevation(self.location.observer, now)
            
            # Get today's sun events
            s = sun(self.location.observer, date=now.date(), tzinfo=self.timezone)
            
            result = {
                'azimuth': sun_az,
                'elevation': sun_el,
                'sunrise': s['sunrise'].isoformat(),
                'sunset': s['sunset'].isoformat(),
                'is_daylight': sun_el > 0
            }
            
            self.status['sun_azimuth'] = sun_az
            self.status['sun_elevation'] = sun_el
            
            return result
            
        except Exception as e:
            logger.error(f"Error calculating sun position: {e}")
            return {'azimuth': 0, 'elevation': 0, 'is_daylight': False}
    
    async def update_targets(self, sun_position: Dict[str, float]):
        """Update target positions based on tracking mode"""
        
        # Check for high wind - auto-stow
        if self.wind_speed > self.max_wind_speed and not self.is_stowed:
            logger.warning(f"High wind detected: {self.wind_speed} mph, auto-stowing")
            await self.stow()
            return
        
        if self.tracking_mode == TrackingMode.AUTO:
            # Track the sun if it's above horizon
            if sun_position['is_daylight'] and sun_position['elevation'] > 0:
                self.target_azimuth = sun_position['azimuth']
                self.target_elevation = sun_position['elevation']
                self.is_stowed = False
            else:
                # Stow at night
                if not self.is_stowed:
                    await self.stow()
        
        elif self.tracking_mode == TrackingMode.STOW:
            if not self.is_stowed:
                await self.stow()
        
        elif self.tracking_mode == TrackingMode.MANUAL:
            # Targets set by user commands
            pass
        
        elif self.tracking_mode == TrackingMode.CALIBRATE:
            # Calibration mode - special handling
            pass
        
        self.status['target_azimuth'] = self.target_azimuth
        self.status['target_elevation'] = self.target_elevation
        self.status['mode'] = self.tracking_mode
    
    async def execute_control(self):
        """Execute PID control to move tracker to target"""
        
        if self.tracking_mode == TrackingMode.MANUAL or self.is_stowed:
            return
        
        try:
            # Calculate errors
            azimuth_error = self.target_azimuth - self.current_azimuth
            elevation_error = self.target_elevation - self.current_elevation
            
            # Normalize azimuth error to [-180, 180]
            if azimuth_error > 180:
                azimuth_error -= 360
            elif azimuth_error < -180:
                azimuth_error += 360
            
            self.status['error_azimuth'] = azimuth_error
            self.status['error_elevation'] = elevation_error
            
            # Check if we're close enough (deadband)
            deadband = 1.0  # degrees
            if abs(azimuth_error) < deadband and abs(elevation_error) < deadband:
                self.is_moving = False
                self.status['is_moving'] = False
                return
            
            # Calculate PID outputs
            azimuth_output = self.azimuth_pid(azimuth_error)
            elevation_output = self.elevation_pid(elevation_error)
            
            # Send commands to motors
            await self.esp32.set_motor_speed('azimuth', azimuth_output)
            await self.esp32.set_motor_speed('elevation', elevation_output)
            
            self.is_moving = True
            self.status['is_moving'] = True
            
        except Exception as e:
            logger.error(f"Error in control execution: {e}")
    
    async def stow(self):
        """Move tracker to stow position"""
        logger.info("Stowing tracker")
        self.target_azimuth = self.stow_azimuth
        self.target_elevation = self.stow_elevation
        self.is_stowed = True
        self.status['is_stowed'] = True
        
        # Send stow command to ESP32
        await self.esp32.stow()
    
    async def publish_status(self):
        """Publish current status to MQTT"""
        try:
            # Update status dict
            self.status['last_update'] = self.last_update
            
            # Publish to MQTT
            await self.mqtt.publish('solar_tracker/status', json.dumps(self.status))
            
            # Publish individual sensors for HA
            await self.mqtt.publish('solar_tracker/azimuth', self.current_azimuth)
            await self.mqtt.publish('solar_tracker/elevation', self.current_elevation)
            await self.mqtt.publish('solar_tracker/wind_speed', self.wind_speed)
            await self.mqtt.publish('solar_tracker/sun_azimuth', self.status['sun_azimuth'])
            await self.mqtt.publish('solar_tracker/sun_elevation', self.status['sun_elevation'])
            
        except Exception as e:
            logger.error(f"Error publishing status: {e}")
    
    async def handle_mode_command(self, payload: str):
        """Handle mode change command"""
        try:
            mode = payload.strip().lower()
            if mode in [TrackingMode.AUTO, TrackingMode.MANUAL, 
                       TrackingMode.STOW, TrackingMode.CALIBRATE]:
                logger.info(f"Mode changed to: {mode}")
                self.tracking_mode = mode
                
                if mode == TrackingMode.STOW:
                    await self.stow()
        except Exception as e:
            logger.error(f"Error handling mode command: {e}")
    
    async def handle_move_command(self, payload: str):
        """Handle manual move command"""
        try:
            data = json.loads(payload)
            if self.tracking_mode == TrackingMode.MANUAL:
                self.target_azimuth = float(data.get('azimuth', self.target_azimuth))
                self.target_elevation = float(data.get('elevation', self.target_elevation))
                logger.info(f"Manual target set: Az={self.target_azimuth}, El={self.target_elevation}")
        except Exception as e:
            logger.error(f"Error handling move command: {e}")
    
    async def handle_stow_command(self, payload: str):
        """Handle stow command"""
        await self.stow()
    
    async def stop(self):
        """Stop the controller gracefully"""
        logger.info("Stopping controller")
        
        # Stop motors
        if self.esp32:
            await self.esp32.stop_motors()
            await self.esp32.disconnect()
        
        # Disconnect Home Assistant
        if self.ha_interface:
            await self.ha_interface.disconnect()
        
        # Disconnect MQTT
        if self.mqtt:
            await self.mqtt.disconnect()
        
        logger.info("Controller stopped")
    
    def get_status(self) -> Dict:
        """Get current status for web interface"""
        return self.status.copy()
    
    async def set_mode(self, mode: str):
        """Set tracking mode"""
        await self.handle_mode_command(mode)
    
    async def set_target(self, azimuth: float, elevation: float):
        """Set manual target position"""
        if self.tracking_mode == TrackingMode.MANUAL:
            self.target_azimuth = azimuth
            self.target_elevation = elevation
