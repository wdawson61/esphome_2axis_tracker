"""
Home Assistant API Interface Module
Reads entity states from Home Assistant for weather data and other sensors
"""

import asyncio
import logging
from typing import Dict, Optional
import aiohttp
import os

logger = logging.getLogger(__name__)


class HomeAssistantInterface:
    """Interface for reading Home Assistant entity states"""
    
    def __init__(self):
        """Initialize Home Assistant API interface"""
        # Get HA Supervisor API token
        self.token = os.getenv('SUPERVISOR_TOKEN', '')
        self.base_url = "http://supervisor/core/api"
        
        # Configure entity IDs from environment
        self.wind_speed_entity = os.getenv('HA_WIND_SPEED_ENTITY', '')
        self.wind_gust_entity = os.getenv('HA_WIND_GUST_ENTITY', '')
        self.wind_direction_entity = os.getenv('HA_WIND_DIRECTION_ENTITY', '')
        self.temperature_entity = os.getenv('HA_TEMPERATURE_ENTITY', '')
        self.humidity_entity = os.getenv('HA_HUMIDITY_ENTITY', '')
        self.use_wind_gust = os.getenv('USE_WIND_GUST_FOR_STOW', 'false').lower() == 'true'
        
        self.session: Optional[aiohttp.ClientSession] = None
        self.connected = False
        
        # Cache for entity states
        self.entity_cache: Dict[str, Dict] = {}
        
        logger.info("Home Assistant interface initialized")
        if self.wind_speed_entity:
            logger.info(f"Wind speed entity: {self.wind_speed_entity}")
        if self.wind_gust_entity:
            logger.info(f"Wind gust entity: {self.wind_gust_entity}")
        if self.wind_direction_entity:
            logger.info(f"Wind direction entity: {self.wind_direction_entity}")
    
    async def connect(self):
        """Initialize connection to Home Assistant API"""
        try:
            # Create session with auth header
            headers = {
                'Authorization': f'Bearer {self.token}',
                'Content-Type': 'application/json'
            }
            
            self.session = aiohttp.ClientSession(
                headers=headers,
                timeout=aiohttp.ClientTimeout(total=5)
            )
            
            # Test connection
            async with self.session.get(f"{self.base_url}/states") as response:
                if response.status == 200:
                    self.connected = True
                    logger.info("Connected to Home Assistant API")
                    return True
                else:
                    logger.error(f"Failed to connect to HA API: {response.status}")
                    
        except Exception as e:
            logger.error(f"Error connecting to Home Assistant API: {e}")
            self.connected = False
        
        return False
    
    async def disconnect(self):
        """Close connection to Home Assistant API"""
        if self.session:
            await self.session.close()
            self.connected = False
            logger.info("Disconnected from Home Assistant API")
    
    async def get_entity_state(self, entity_id: str) -> Optional[Dict]:
        """
        Get the state of a Home Assistant entity
        
        Args:
            entity_id: Entity ID (e.g., 'sensor.wind_speed')
        
        Returns:
            Dictionary with state and attributes, or None if unavailable
        """
        if not self.connected or not entity_id:
            return None
        
        try:
            url = f"{self.base_url}/states/{entity_id}"
            
            async with self.session.get(url) as response:
                if response.status == 200:
                    data = await response.json()
                    
                    # Cache the result
                    self.entity_cache[entity_id] = data
                    
                    return data
                else:
                    logger.warning(f"Entity {entity_id} returned status {response.status}")
                    
        except asyncio.TimeoutError:
            logger.warning(f"Timeout getting state for {entity_id}")
        except Exception as e:
            logger.error(f"Error getting state for {entity_id}: {e}")
        
        return None
    
    async def get_wind_data(self) -> Dict[str, float]:
        """
        Get wind data from configured Home Assistant entities
        
        Returns:
            Dictionary with wind_speed, wind_gust, wind_direction
        """
        wind_data = {
            'wind_speed': 0.0,
            'wind_gust': 0.0,
            'wind_direction': 0.0,
            'wind_speed_available': False,
            'wind_gust_available': False
        }
        
        # Get wind speed
        if self.wind_speed_entity:
            state = await self.get_entity_state(self.wind_speed_entity)
            if state and state.get('state') not in ['unknown', 'unavailable', None]:
                try:
                    wind_data['wind_speed'] = float(state['state'])
                    wind_data['wind_speed_available'] = True
                except (ValueError, TypeError):
                    logger.warning(f"Invalid wind speed value: {state.get('state')}")
        
        # Get wind gust
        if self.wind_gust_entity:
            state = await self.get_entity_state(self.wind_gust_entity)
            if state and state.get('state') not in ['unknown', 'unavailable', None]:
                try:
                    wind_data['wind_gust'] = float(state['state'])
                    wind_data['wind_gust_available'] = True
                except (ValueError, TypeError):
                    logger.warning(f"Invalid wind gust value: {state.get('state')}")
        
        # Get wind direction
        if self.wind_direction_entity:
            state = await self.get_entity_state(self.wind_direction_entity)
            if state and state.get('state') not in ['unknown', 'unavailable', None]:
                try:
                    wind_data['wind_direction'] = float(state['state'])
                except (ValueError, TypeError):
                    logger.warning(f"Invalid wind direction value: {state.get('state')}")
        
        return wind_data
    
    async def get_weather_data(self) -> Dict[str, Optional[float]]:
        """
        Get additional weather data from configured entities
        
        Returns:
            Dictionary with temperature, humidity
        """
        weather_data = {
            'temperature': None,
            'humidity': None
        }
        
        # Get temperature
        if self.temperature_entity:
            state = await self.get_entity_state(self.temperature_entity)
            if state and state.get('state') not in ['unknown', 'unavailable', None]:
                try:
                    weather_data['temperature'] = float(state['state'])
                except (ValueError, TypeError):
                    pass
        
        # Get humidity
        if self.humidity_entity:
            state = await self.get_entity_state(self.humidity_entity)
            if state and state.get('state') not in ['unknown', 'unavailable', None]:
                try:
                    weather_data['humidity'] = float(state['state'])
                except (ValueError, TypeError):
                    pass
        
        return weather_data
    
    def get_wind_speed_for_stow_check(self, wind_data: Dict[str, float]) -> float:
        """
        Get the wind speed value to use for stow threshold checking
        
        Args:
            wind_data: Wind data dictionary from get_wind_data()
        
        Returns:
            Wind speed value to compare against threshold
        """
        # Use gust if configured and available, otherwise use average
        if self.use_wind_gust and wind_data.get('wind_gust_available'):
            return wind_data['wind_gust']
        elif wind_data.get('wind_speed_available'):
            return wind_data['wind_speed']
        else:
            return 0.0
    
    def has_wind_entities(self) -> bool:
        """Check if any wind entities are configured"""
        return bool(self.wind_speed_entity or self.wind_gust_entity)
    
    async def get_all_entity_states(self, entity_ids: list) -> Dict[str, Optional[Dict]]:
        """
        Get states for multiple entities at once
        
        Args:
            entity_ids: List of entity IDs
        
        Returns:
            Dictionary mapping entity_id to state data
        """
        results = {}
        
        # Fetch all entities concurrently
        tasks = [self.get_entity_state(entity_id) for entity_id in entity_ids]
        states = await asyncio.gather(*tasks, return_exceptions=True)
        
        for entity_id, state in zip(entity_ids, states):
            if isinstance(state, Exception):
                logger.error(f"Error fetching {entity_id}: {state}")
                results[entity_id] = None
            else:
                results[entity_id] = state
        
        return results
