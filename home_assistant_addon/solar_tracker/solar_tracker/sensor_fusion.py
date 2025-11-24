"""
Sensor Fusion Module
Combines accelerometer, gyroscope, and magnetometer data using complementary filter
"""

import logging
import numpy as np
from typing import Dict

logger = logging.getLogger(__name__)


class SensorFusion:
    """
    9-axis sensor fusion using complementary filter
    Combines gyro, accelerometer, and magnetometer for accurate orientation
    """
    
    def __init__(self, alpha: float = 0.98):
        """
        Initialize sensor fusion
        
        Args:
            alpha: Complementary filter coefficient (0-1)
                  Higher = trust gyro more, Lower = trust accel/mag more
        """
        self.alpha = alpha
        
        # Current orientation estimate (Euler angles in degrees)
        self.roll = 0.0
        self.pitch = 0.0
        self.yaw = 0.0
        
        # Calibration offsets (to be determined during calibration)
        self.accel_offset = np.array([0.0, 0.0, 0.0])
        self.gyro_offset = np.array([0.0, 0.0, 0.0])
        self.mag_offset = np.array([0.0, 0.0, 0.0])
        self.mag_scale = np.array([1.0, 1.0, 1.0])
        
        # Low-pass filter for accelerometer
        self.accel_filtered = np.array([0.0, 0.0, 1.0])  # Assuming Z-up initially
        
        logger.info("Sensor fusion initialized")
    
    def update(self, accel_x: float, accel_y: float, accel_z: float,
               gyro_x: float, gyro_y: float, gyro_z: float,
               mag_x: float, mag_y: float, mag_z: float,
               dt: float) -> Dict[str, float]:
        """
        Update orientation estimate with new sensor data
        
        Args:
            accel_x, accel_y, accel_z: Accelerometer (m/s^2 or g)
            gyro_x, gyro_y, gyro_z: Gyroscope (deg/s or rad/s)
            mag_x, mag_y, mag_z: Magnetometer (uT or gauss)
            dt: Time delta since last update (seconds)
        
        Returns:
            Dictionary with azimuth, elevation, roll, pitch, yaw
        """
        
        # Apply calibration offsets
        accel = np.array([accel_x, accel_y, accel_z]) - self.accel_offset
        gyro = np.array([gyro_x, gyro_y, gyro_z]) - self.gyro_offset
        mag = (np.array([mag_x, mag_y, mag_z]) - self.mag_offset) * self.mag_scale
        
        # Normalize accelerometer
        accel_norm = np.linalg.norm(accel)
        if accel_norm > 0:
            accel = accel / accel_norm
        
        # Low-pass filter on accelerometer to reduce noise
        alpha_accel = 0.2
        self.accel_filtered = alpha_accel * accel + (1 - alpha_accel) * self.accel_filtered
        
        # Calculate roll and pitch from accelerometer
        accel_roll = np.degrees(np.arctan2(self.accel_filtered[1], self.accel_filtered[2]))
        accel_pitch = np.degrees(np.arctan2(-self.accel_filtered[0],
                                           np.sqrt(self.accel_filtered[1]**2 + self.accel_filtered[2]**2)))
        
        # Integrate gyroscope for orientation change
        gyro_roll_rate = gyro[0]
        gyro_pitch_rate = gyro[1]
        gyro_yaw_rate = gyro[2]
        
        # Complementary filter: combine gyro integration with accel
        self.roll = self.alpha * (self.roll + gyro_roll_rate * dt) + (1 - self.alpha) * accel_roll
        self.pitch = self.alpha * (self.pitch + gyro_pitch_rate * dt) + (1 - self.alpha) * accel_pitch
        
        # Calculate yaw from magnetometer (tilt-compensated)
        yaw_from_mag = self._calculate_tilt_compensated_heading(mag, self.roll, self.pitch)
        
        # Integrate gyro yaw with magnetometer correction
        gyro_yaw = self.yaw + gyro_yaw_rate * dt
        
        # Use lower alpha for yaw to trust magnetometer more (less drift)
        alpha_yaw = 0.95
        self.yaw = alpha_yaw * gyro_yaw + (1 - alpha_yaw) * yaw_from_mag
        
        # Normalize yaw to [0, 360]
        self.yaw = self.yaw % 360
        
        # Convert to solar tracker coordinates
        # Azimuth = yaw (0=North, 90=East, 180=South, 270=West)
        # Elevation = pitch (positive up from horizon)
        azimuth = self.yaw
        elevation = self.pitch
        
        return {
            'azimuth': azimuth,
            'elevation': elevation,
            'roll': self.roll,
            'pitch': self.pitch,
            'yaw': self.yaw
        }
    
    def _calculate_tilt_compensated_heading(self, mag: np.ndarray, 
                                           roll: float, pitch: float) -> float:
        """
        Calculate tilt-compensated magnetic heading
        
        Args:
            mag: Magnetometer vector [x, y, z]
            roll: Roll angle (degrees)
            pitch: Pitch angle (degrees)
        
        Returns:
            Heading in degrees (0=North, 90=East)
        """
        
        # Convert angles to radians
        roll_rad = np.radians(roll)
        pitch_rad = np.radians(pitch)
        
        # Tilt compensation matrix
        cos_pitch = np.cos(pitch_rad)
        sin_pitch = np.sin(pitch_rad)
        cos_roll = np.cos(roll_rad)
        sin_roll = np.sin(roll_rad)
        
        # Compensate magnetometer readings for tilt
        mag_x_comp = (mag[0] * cos_pitch + 
                     mag[1] * sin_roll * sin_pitch + 
                     mag[2] * cos_roll * sin_pitch)
        
        mag_y_comp = (mag[1] * cos_roll - 
                     mag[2] * sin_roll)
        
        # Calculate heading
        heading = np.degrees(np.arctan2(mag_y_comp, mag_x_comp))
        
        # Normalize to [0, 360]
        if heading < 0:
            heading += 360
        
        return heading
    
    def calibrate_gyro(self, samples: list):
        """
        Calibrate gyroscope by averaging samples while stationary
        
        Args:
            samples: List of (gyro_x, gyro_y, gyro_z) tuples
        """
        if len(samples) > 0:
            self.gyro_offset = np.mean(samples, axis=0)
            logger.info(f"Gyro calibrated: offset = {self.gyro_offset}")
    
    def calibrate_magnetometer(self, samples: list):
        """
        Calibrate magnetometer using min/max method
        
        Args:
            samples: List of (mag_x, mag_y, mag_z) tuples from rotating device
        """
        if len(samples) > 0:
            samples_array = np.array(samples)
            mag_min = np.min(samples_array, axis=0)
            mag_max = np.max(samples_array, axis=0)
            
            # Calculate offset (hard iron distortion)
            self.mag_offset = (mag_max + mag_min) / 2
            
            # Calculate scale (soft iron distortion)
            mag_range = mag_max - mag_min
            avg_range = np.mean(mag_range)
            self.mag_scale = avg_range / mag_range
            
            logger.info(f"Magnetometer calibrated: offset = {self.mag_offset}, scale = {self.mag_scale}")
    
    def reset(self):
        """Reset orientation to zero"""
        self.roll = 0.0
        self.pitch = 0.0
        self.yaw = 0.0
        self.accel_filtered = np.array([0.0, 0.0, 1.0])
        logger.info("Sensor fusion reset")
    
    def set_orientation(self, roll: float = None, pitch: float = None, yaw: float = None):
        """
        Manually set orientation (useful for calibration)
        
        Args:
            roll, pitch, yaw: Angles in degrees (None = don't change)
        """
        if roll is not None:
            self.roll = roll
        if pitch is not None:
            self.pitch = pitch
        if yaw is not None:
            self.yaw = yaw
