"""
ESP32 Interface Module
Handles communication with ESP32 hardware via HTTP API
"""

import asyncio
import logging
from typing import Dict, Optional
import aiohttp
import json

logger = logging.getLogger(__name__)


class ESP32Interface:
    """Interface for communicating with ESP32 controller"""
    
    def __init__(self, ip: str, port: int = 80):
        """
        Initialize ESP32 interface
        
        Args:
            ip: ESP32 IP address
            port: ESP32 web server port
        """
        self.ip = ip
        self.port = port
        self.base_url = f"http://{ip}:{port}"
        self.session: Optional[aiohttp.ClientSession] = None
        self.connected = False
        
        logger.info(f"ESP32 interface initialized: {self.base_url}")
    
    async def connect(self):
        """Establish connection to ESP32"""
        try:
            self.session = aiohttp.ClientSession(timeout=aiohttp.ClientTimeout(total=5))
            
            # Test connection
            async with self.session.get(f"{self.base_url}/status") as response:
                if response.status == 200:
                    self.connected = True
                    logger.info("Connected to ESP32")
                    return True
        except Exception as e:
            logger.error(f"Failed to connect to ESP32: {e}")
            self.connected = False
        
        return False
    
    async def disconnect(self):
        """Close connection to ESP32"""
        if self.session:
            await self.session.close()
            self.connected = False
            logger.info("Disconnected from ESP32")
    
    async def get_sensor_data(self) -> Optional[Dict]:
        """
        Get all sensor data from ESP32
        
        Returns:
            Dictionary with sensor readings or None if failed
        """
        if not self.connected:
            return None
        
        try:
            async with self.session.get(f"{self.base_url}/api/sensors") as response:
                if response.status == 200:
                    data = await response.json()
                    return data
                else:
                    logger.warning(f"Failed to get sensor data: {response.status}")
        except asyncio.TimeoutError:
            logger.warning("Timeout getting sensor data")
        except Exception as e:
            logger.error(f"Error getting sensor data: {e}")
        
        return None
    
    async def set_motor_speed(self, motor: str, speed: float) -> bool:
        """
        Set motor speed
        
        Args:
            motor: Motor name ('azimuth' or 'elevation')
            speed: Speed value (-100 to 100)
        
        Returns:
            True if successful
        """
        if not self.connected:
            return False
        
        try:
            payload = {
                'motor': motor,
                'speed': max(-100, min(100, speed))
            }
            
            async with self.session.post(
                f"{self.base_url}/api/motor",
                json=payload
            ) as response:
                return response.status == 200
                
        except Exception as e:
            logger.error(f"Error setting motor speed: {e}")
            return False
    
    async def stop_motors(self) -> bool:
        """
        Stop all motors
        
        Returns:
            True if successful
        """
        try:
            async with self.session.post(f"{self.base_url}/api/stop") as response:
                return response.status == 200
        except Exception as e:
            logger.error(f"Error stopping motors: {e}")
            return False
    
    async def stow(self) -> bool:
        """
        Send stow command to ESP32
        
        Returns:
            True if successful
        """
        try:
            async with self.session.post(f"{self.base_url}/api/stow") as response:
                return response.status == 200
        except Exception as e:
            logger.error(f"Error sending stow command: {e}")
            return False
    
    async def get_limit_switches(self) -> Optional[Dict]:
        """
        Get limit switch states
        
        Returns:
            Dictionary with limit switch states
        """
        if not self.connected:
            return None
        
        try:
            async with self.session.get(f"{self.base_url}/api/limits") as response:
                if response.status == 200:
                    return await response.json()
        except Exception as e:
            logger.error(f"Error getting limit switches: {e}")
        
        return None
    
    async def send_modbus_command(self, slave_addr: int, hex_string: str) -> Optional[Dict]:
        """
        Send Modbus command via RS485
        
        Args:
            slave_addr: Modbus slave address
            hex_string: Hex command string
        
        Returns:
            Response dictionary or None
        """
        if not self.connected:
            return None
        
        try:
            payload = {
                'slave': slave_addr,
                'hex': hex_string
            }
            
            async with self.session.post(
                f"{self.base_url}/send",
                data=payload
            ) as response:
                if response.status == 200:
                    return await response.json()
        except Exception as e:
            logger.error(f"Error sending Modbus command: {e}")
        
        return None
    
    async def calibrate_sensor(self, sensor_type: str) -> bool:
        """
        Start sensor calibration
        
        Args:
            sensor_type: 'gyro', 'accel', or 'mag'
        
        Returns:
            True if calibration started
        """
        try:
            payload = {'sensor': sensor_type}
            async with self.session.post(
                f"{self.base_url}/api/calibrate",
                json=payload
            ) as response:
                return response.status == 200
        except Exception as e:
            logger.error(f"Error starting calibration: {e}")
            return False
