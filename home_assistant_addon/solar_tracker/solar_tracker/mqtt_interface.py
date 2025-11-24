"""
MQTT Interface Module
Handles MQTT communication with Home Assistant
"""

import asyncio
import logging
from typing import Callable, Dict, Optional
import json

import paho.mqtt.client as mqtt

logger = logging.getLogger(__name__)


class MQTTInterface:
    """MQTT client for Home Assistant integration"""
    
    def __init__(self, broker: str, port: int = 1883, 
                 username: str = "", password: str = ""):
        """
        Initialize MQTT interface
        
        Args:
            broker: MQTT broker address
            port: MQTT broker port
            username: MQTT username (optional)
            password: MQTT password (optional)
        """
        self.broker = broker
        self.port = port
        self.username = username
        self.password = password
        
        self.client = mqtt.Client()
        self.client.on_connect = self._on_connect
        self.client.on_message = self._on_message
        self.client.on_disconnect = self._on_disconnect
        
        self.connected = False
        self.subscriptions: Dict[str, Callable] = {}
        
        logger.info(f"MQTT interface initialized: {broker}:{port}")
    
    async def connect(self):
        """Connect to MQTT broker"""
        try:
            if self.username:
                self.client.username_pw_set(self.username, self.password)
            
            self.client.connect_async(self.broker, self.port, 60)
            self.client.loop_start()
            
            # Wait for connection
            for _ in range(50):  # 5 second timeout
                if self.connected:
                    break
                await asyncio.sleep(0.1)
            
            if self.connected:
                logger.info("Connected to MQTT broker")
                await self._publish_discovery()
                return True
            else:
                logger.error("Failed to connect to MQTT broker")
                return False
                
        except Exception as e:
            logger.error(f"Error connecting to MQTT: {e}")
            return False
    
    async def disconnect(self):
        """Disconnect from MQTT broker"""
        self.client.loop_stop()
        self.client.disconnect()
        self.connected = False
        logger.info("Disconnected from MQTT broker")
    
    def _on_connect(self, client, userdata, flags, rc):
        """MQTT connection callback"""
        if rc == 0:
            self.connected = True
            logger.info("MQTT connected successfully")
            
            # Resubscribe to topics
            for topic in self.subscriptions.keys():
                client.subscribe(topic)
                logger.info(f"Subscribed to {topic}")
        else:
            logger.error(f"MQTT connection failed with code {rc}")
    
    def _on_disconnect(self, client, userdata, rc):
        """MQTT disconnection callback"""
        self.connected = False
        if rc != 0:
            logger.warning(f"Unexpected MQTT disconnect: {rc}")
    
    def _on_message(self, client, userdata, msg):
        """MQTT message callback"""
        topic = msg.topic
        payload = msg.payload.decode('utf-8')
        
        logger.debug(f"MQTT message received: {topic} = {payload}")
        
        # Call registered handler
        if topic in self.subscriptions:
            handler = self.subscriptions[topic]
            asyncio.create_task(handler(payload))
    
    async def subscribe(self, topic: str, handler: Callable):
        """
        Subscribe to MQTT topic
        
        Args:
            topic: MQTT topic to subscribe
            handler: Async callback function for messages
        """
        self.subscriptions[topic] = handler
        if self.connected:
            self.client.subscribe(topic)
            logger.info(f"Subscribed to {topic}")
    
    async def publish(self, topic: str, payload, retain: bool = False):
        """
        Publish to MQTT topic
        
        Args:
            topic: MQTT topic
            payload: Message payload (str, int, float, or dict)
            retain: Retain flag
        """
        if not self.connected:
            return
        
        try:
            if isinstance(payload, (dict, list)):
                payload = json.dumps(payload)
            elif not isinstance(payload, str):
                payload = str(payload)
            
            self.client.publish(topic, payload, retain=retain)
            
        except Exception as e:
            logger.error(f"Error publishing to MQTT: {e}")
    
    async def _publish_discovery(self):
        """Publish Home Assistant MQTT discovery messages"""
        
        # Define sensors for auto-discovery
        sensors = [
            {
                'name': 'Solar Tracker Azimuth',
                'state_topic': 'solar_tracker/azimuth',
                'unit_of_measurement': '°',
                'icon': 'mdi:compass',
                'unique_id': 'solar_tracker_azimuth'
            },
            {
                'name': 'Solar Tracker Elevation',
                'state_topic': 'solar_tracker/elevation',
                'unit_of_measurement': '°',
                'icon': 'mdi:angle-acute',
                'unique_id': 'solar_tracker_elevation'
            },
            {
                'name': 'Solar Tracker Sun Azimuth',
                'state_topic': 'solar_tracker/sun_azimuth',
                'unit_of_measurement': '°',
                'icon': 'mdi:sun-compass',
                'unique_id': 'solar_tracker_sun_azimuth'
            },
            {
                'name': 'Solar Tracker Sun Elevation',
                'state_topic': 'solar_tracker/sun_elevation',
                'unit_of_measurement': '°',
                'icon': 'mdi:sun-angle',
                'unique_id': 'solar_tracker_sun_elevation'
            },
            {
                'name': 'Solar Tracker Wind Speed',
                'state_topic': 'solar_tracker/wind_speed',
                'unit_of_measurement': 'mph',
                'icon': 'mdi:weather-windy',
                'unique_id': 'solar_tracker_wind_speed'
            }
        ]
        
        # Publish discovery messages
        for sensor in sensors:
            discovery_topic = f"homeassistant/sensor/solar_tracker/{sensor['unique_id']}/config"
            config = {
                'name': sensor['name'],
                'state_topic': sensor['state_topic'],
                'unit_of_measurement': sensor.get('unit_of_measurement'),
                'icon': sensor.get('icon'),
                'unique_id': sensor['unique_id'],
                'device': {
                    'identifiers': ['solar_tracker'],
                    'name': 'Solar Tracker',
                    'model': 'Custom Controller',
                    'manufacturer': 'DIY'
                }
            }
            await self.publish(discovery_topic, config, retain=True)
        
        logger.info("Published MQTT discovery configuration")
