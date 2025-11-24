#!/usr/bin/env python3
"""
Solar Tracker Controller Add-on
Main entry point for the application
"""

import asyncio
import logging
import signal
import sys
from pathlib import Path

from controller import SolarTrackerController
from web_server import WebServer

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    handlers=[
        logging.StreamHandler(sys.stdout)
    ]
)

logger = logging.getLogger(__name__)


class Application:
    """Main application class"""
    
    def __init__(self):
        self.controller = None
        self.web_server = None
        self.running = False
        
    async def start(self):
        """Start the application"""
        logger.info("Initializing Solar Tracker Controller...")
        
        try:
            # Initialize controller
            self.controller = SolarTrackerController()
            await self.controller.initialize()
            
            # Initialize web server
            self.web_server = WebServer(self.controller)
            
            # Start both services
            self.running = True
            
            # Create tasks
            controller_task = asyncio.create_task(self.controller.run())
            web_task = asyncio.create_task(self.web_server.start())
            
            logger.info("Solar Tracker Controller started successfully")
            logger.info("Web interface available at http://0.0.0.0:8099")
            
            # Wait for both tasks
            await asyncio.gather(controller_task, web_task)
            
        except Exception as e:
            logger.error(f"Failed to start application: {e}", exc_info=True)
            await self.stop()
            sys.exit(1)
    
    async def stop(self):
        """Stop the application gracefully"""
        logger.info("Shutting down Solar Tracker Controller...")
        self.running = False
        
        if self.controller:
            await self.controller.stop()
        
        if self.web_server:
            await self.web_server.stop()
        
        logger.info("Shutdown complete")


def signal_handler(app):
    """Handle shutdown signals"""
    def handler(signum, frame):
        logger.info(f"Received signal {signum}, initiating shutdown...")
        asyncio.create_task(app.stop())
    return handler


async def main():
    """Main entry point"""
    app = Application()
    
    # Setup signal handlers
    signal.signal(signal.SIGTERM, signal_handler(app))
    signal.signal(signal.SIGINT, signal_handler(app))
    
    try:
        await app.start()
    except KeyboardInterrupt:
        logger.info("Received keyboard interrupt")
        await app.stop()
    except Exception as e:
        logger.error(f"Unexpected error: {e}", exc_info=True)
        await app.stop()
        sys.exit(1)


if __name__ == "__main__":
    asyncio.run(main())
