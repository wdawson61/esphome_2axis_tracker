"""
Web Server Module
Provides REST API and web interface for solar tracker control
"""

import asyncio
import logging
from pathlib import Path
from typing import Optional

from aiohttp import web
import aiofiles
import json

logger = logging.getLogger(__name__)


class WebServer:
    """Web server for solar tracker interface"""
    
    def __init__(self, controller):
        """
        Initialize web server
        
        Args:
            controller: SolarTrackerController instance
        """
        self.controller = controller
        self.app = web.Application()
        self.runner: Optional[web.AppRunner] = None
        
        # Setup routes
        self._setup_routes()
        
        logger.info("Web server initialized")
    
    def _setup_routes(self):
        """Setup web server routes"""
        
        # API routes
        self.app.router.add_get('/api/status', self.handle_status)
        self.app.router.add_post('/api/mode', self.handle_set_mode)
        self.app.router.add_post('/api/target', self.handle_set_target)
        self.app.router.add_post('/api/stow', self.handle_stow)
        self.app.router.add_post('/api/calibrate', self.handle_calibrate)
        
        # Static files and UI
        self.app.router.add_get('/', self.handle_index)
        self.app.router.add_static('/static', '/app/web/static')
    
    async def handle_index(self, request):
        """Serve main UI page"""
        html_path = Path('/app/web/index.html')
        if html_path.exists():
            async with aiofiles.open(html_path, 'r') as f:
                content = await f.read()
            return web.Response(text=content, content_type='text/html')
        else:
            return web.Response(text="UI not found", status=404)
    
    async def handle_status(self, request):
        """Get current status"""
        status = self.controller.get_status()
        return web.json_response(status)
    
    async def handle_set_mode(self, request):
        """Set tracking mode"""
        try:
            data = await request.json()
            mode = data.get('mode')
            
            if mode not in ['auto', 'manual', 'stow', 'calibrate']:
                return web.json_response(
                    {'success': False, 'error': 'Invalid mode'},
                    status=400
                )
            
            await self.controller.set_mode(mode)
            
            return web.json_response({'success': True, 'mode': mode})
            
        except Exception as e:
            logger.error(f"Error setting mode: {e}")
            return web.json_response(
                {'success': False, 'error': str(e)},
                status=500
            )
    
    async def handle_set_target(self, request):
        """Set manual target position"""
        try:
            data = await request.json()
            azimuth = float(data.get('azimuth', 0))
            elevation = float(data.get('elevation', 0))
            
            # Validate ranges
            if not (-180 <= azimuth <= 360):
                return web.json_response(
                    {'success': False, 'error': 'Invalid azimuth'},
                    status=400
                )
            
            if not (-90 <= elevation <= 90):
                return web.json_response(
                    {'success': False, 'error': 'Invalid elevation'},
                    status=400
                )
            
            await self.controller.set_target(azimuth, elevation)
            
            return web.json_response({
                'success': True,
                'azimuth': azimuth,
                'elevation': elevation
            })
            
        except Exception as e:
            logger.error(f"Error setting target: {e}")
            return web.json_response(
                {'success': False, 'error': str(e)},
                status=500
            )
    
    async def handle_stow(self, request):
        """Stow the tracker"""
        try:
            await self.controller.stow()
            return web.json_response({'success': True})
        except Exception as e:
            logger.error(f"Error stowing: {e}")
            return web.json_response(
                {'success': False, 'error': str(e)},
                status=500
            )
    
    async def handle_calibrate(self, request):
        """Start calibration procedure"""
        try:
            data = await request.json()
            sensor_type = data.get('sensor', 'gyro')
            
            # TODO: Implement calibration logic
            logger.info(f"Calibration requested for {sensor_type}")
            
            return web.json_response({
                'success': True,
                'message': f'Calibration started for {sensor_type}'
            })
            
        except Exception as e:
            logger.error(f"Error starting calibration: {e}")
            return web.json_response(
                {'success': False, 'error': str(e)},
                status=500
            )
    
    async def start(self):
        """Start web server"""
        try:
            self.runner = web.AppRunner(self.app)
            await self.runner.setup()
            
            site = web.TCPSite(self.runner, '0.0.0.0', 8099)
            await site.start()
            
            logger.info("Web server started on http://0.0.0.0:8099")
            
            # Keep running
            while True:
                await asyncio.sleep(3600)
                
        except Exception as e:
            logger.error(f"Error starting web server: {e}")
    
    async def stop(self):
        """Stop web server"""
        if self.runner:
            await self.runner.cleanup()
            logger.info("Web server stopped")
