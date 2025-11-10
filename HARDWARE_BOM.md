# Solar Tracker Hardware Shopping List

Complete bill of materials (BOM) with specifications, sources, and estimated costs.

## Core Components

### Microcontroller

| Item | Specification | Quantity | Est. Cost | Notes |
|------|--------------|----------|-----------|-------|
| **ESP32-C6 DevKit** | RISC-V, WiFi 6, BLE 5 | 1 | $5-10 | Espressif official or clone |

**Where to Buy:**
- AliExpress: Search "ESP32-C6 DevKit"
- Amazon: ESP32-C6-DevKitC-1
- Mouser/Digikey: Espressif part #ESP32-C6-DevKitC-1

**Alternatives:**
- ESP32-S3 (dual-core, more RAM)
- ESP32 (original, proven)

---

### IMU Sensor

| Item | Specification | Quantity | Est. Cost | Notes |
|------|--------------|----------|-----------|-------|
| **WIT-Motion HWT905** | 9-axis IMU, RS485 | 1 | $30-50 | Includes accelerometer, gyro, magnetometer |

**Specifications:**
- Axes: 3-axis accelerometer, 3-axis gyro, 3-axis magnetometer
- Interface: RS485, TTL UART
- Voltage: 5V DC
- Baud Rate: 115200 (default)
- Accuracy: ±0.05° (static), ±1° (dynamic)
- Update Rate: 200Hz

**Where to Buy:**
- Official: wit-motion.com
- Amazon: Search "WIT Motion HWT905"
- AliExpress: Cheaper but verify seller

**Critical:** Order with **RS485 output**, not I2C or TTL only!

---

### RS485 Interface

| Item | Specification | Quantity | Est. Cost | Notes |
|------|--------------|----------|-----------|-------|
| **SeeedStudio RS485 Breakout** | MAX485 or SP3485 | 1 | $5-8 | Auto-direction preferred |

**Specifications:**
- Chip: MAX485 or SP3485
- Voltage: 3.3V or 5V compatible
- Features: Auto-direction control (DE/RE tied together)
- Connector: Screw terminals for A/B

**Where to Buy:**
- Seeed Studio: grove.seeedstudio.com
- Adafruit: Similar modules available
- Amazon: Search "RS485 module MAX485"

**Alternatives:**
- Generic MAX485 module (ensure has DE/RE control or tie together)
- DSD TECH RS485 module
- HiLetgo RS485 module

**Wiring Note:** If module doesn't have auto-direction:
- Tie DE and RE together to GPIO or
- DE → 3.3V (transmit enable)
- RE → GND (receive enable)

---

### Motor Controllers

| Item | Specification | Quantity | Est. Cost | Notes |
|------|--------------|----------|-----------|-------|
| **BTS7960 Motor Driver** | 43A dual H-bridge | 2 | $8-15 each | One for elevation, one for azimuth |

**Specifications:**
- Current: 43A max continuous
- Voltage: 5.5V - 27V (24V recommended)
- Logic: 3.3V or 5V compatible
- Protection: Over-current, over-temp, short-circuit
- Control: RPWM/LPWM for direction
- Enable: R_EN/L_EN pins
- PWM Frequency: Up to 25kHz
- Heat Sink: Included (ensure adequate cooling)

**Pin Configuration:**
```
RPWM - Right PWM (forward/CW)
LPWM - Left PWM (backward/CCW)
R_EN - Right Enable (tie to 3.3V)
L_EN - Left Enable (tie to 3.3V)
R_IS - Right Current Sense (optional)
L_IS - Left Current Sense (optional)
VCC - Logic power (5V)
GND - Common ground
B+ - Motor power input (24V)
B- - Motor power ground
M+ - Motor positive output
M- - Motor negative output
```

**Where to Buy:**
- Amazon: Search "BTS7960 motor driver"
- AliExpress: "BTS7960 43A H-bridge"
- eBay: "BTS7960 IBT-2"

**Common Names:**
- BTS7960B Motor Driver
- IBT-2 H-Bridge
- 43A Motor Driver Module

**Alternatives (if BTS7960 unavailable):**
- VNH2SP30 (30A, similar interface)
- Monster Moto Shield (30A)
- Cytron MD30C (30A with different interface)

**NOT Recommended:**
- L298N (only 2A, insufficient for most actuators)
- L293D (only 600mA, toy motors only)

---

### Motors

| Item | Specification | Quantity | Est. Cost | Notes |
|------|--------------|----------|-----------|-------|
| **Linear Actuator** | 24V DC, 50-500mm stroke | 1 | $40-150 | For elevation (tilt) |
| **Slewing Drive Motor** | 24V DC, with gearbox | 1 | $60-200 | For azimuth (rotation) |

**Linear Actuator Specifications:**
- Voltage: 24V DC
- Stroke: Choose based on tilt range needed
  - 100mm = ~4" travel
  - 200mm = ~8" travel  
  - 300mm = ~12" travel
- Force: 200-1000N (depends on panel size)
- Speed: 5-20mm/s
- Mounting: Clevis ends
- IP Rating: IP65+ for outdoor use

**Where to Buy:**
- Amazon: Search "24V linear actuator"
- Progressive Automations (progressiveautomations.com)
- Firgelli Automations (firgelli.com)
- AliExpress: Cheaper options available

**Slewing Drive Specifications:**
- Type: Worm gear slewing drive
- Voltage: 24V DC motor
- Torque: Based on panel weight and wind load
- Gear Ratio: 60:1 to 200:1 (slower = more torque)
- Rotation: 360° continuous or limited
- IP Rating: IP65+ for outdoor use

**Where to Buy:**
- Custom/Industrial: Kaydon, Rotek (expensive)
- Satellite Dish Actuators: Search "24V satellite dish motor"
- AliExpress: "24V slewing drive" or "turntable motor"
- Modify: Car window motors with gearbox

**Alternative Azimuth Solutions:**
- Large DC gear motor with lazy susan bearing
- Stepper motor with high gear ratio
- Satellite dish linear actuator (repurposed)

---

### Limit Switch

| Item | Specification | Quantity | Est. Cost | Notes |
|------|--------------|----------|-----------|-------|
| **Mechanical Limit Switch** | SPDT, NO contact | 1 | $2-5 | For azimuth homing |

**Specifications:**
- Type: Micro switch or roller switch
- Rating: 1A @ 30V DC minimum
- Contacts: SPDT (use NO terminal)
- Mounting: Adjustable position
- IP Rating: IP65+ if exposed to weather

**Where to Buy:**
- Amazon: Search "limit switch micro"
- McMaster-Carr: Precision switches
- AliExpress: "microswitch roller"

**Alternatives:**
- Magnetic reed switch (non-contact)
- Hall effect sensor + magnet
- Optical sensor (photoelectric)
- Inductive proximity sensor

---

### Power Supplies

| Item | Specification | Quantity | Est. Cost | Notes |
|------|--------------|----------|-----------|-------|
| **24V DC Power Supply** | 10A minimum | 1 | $30-60 | For motors |
| **5V DC Power Supply** | 2A minimum | 1 | $5-10 | For ESP32 and sensors |

**24V Supply Specifications:**
- Voltage: 24V DC (regulated)
- Current: 10A minimum, 15A recommended
- Type: Switching power supply
- Protection: Over-current, short-circuit
- Mounting: DIN rail or enclosed
- Use: Powers both BTS7960 modules

**Sizing Guidelines:**
- Linear Actuator: 2-5A typical
- Slewing Motor: 3-8A typical
- Total Peak: ~10A
- Continuous: ~5A
- **Recommendation: 15A supply for safety margin**

**Where to Buy:**
- Amazon: Search "24V 10A power supply"
- Mean Well: RS-360-24 (15A, high quality)
- Mouser/Digikey: Industrial PSUs

**5V Supply:**
- USB power adapter (2A)
- Buck converter from 24V
- Mean Well RS-25-5 (5A, overkill but reliable)

---

## Mounting Hardware

| Item | Quantity | Est. Cost | Notes |
|------|----------|-----------|-------|
| **Enclosure for Electronics** | 1 | $15-30 | IP65+ rated, waterproof |
| **Cable Glands** | 5-10 | $1-2 each | M12 or M16, waterproof |
| **DIN Rail** | 1m | $5-10 | For power supply mounting |
| **Crimps/Connectors** | Set | $10-20 | XT60, spade terminals, etc. |
| **Wire** | Per meter | $1-3/m | 18-22 AWG for signal, 14-16 AWG for power |

---

## Optional Components

| Item | Specification | Quantity | Est. Cost | Notes |
|------|--------------|----------|-----------|-------|
| **Current Sensors** | ACS712 20A | 2 | $3-5 each | Monitor motor current |
| **Voltage Sensor** | 0-25V divider | 1 | $2-3 | Monitor 24V supply |
| **Temperature Sensor** | DS18B20 | 1-2 | $2-3 each | BTS7960 heat monitoring |
| **Emergency Stop Button** | Red mushroom, NO | 1 | $5-10 | Safety cutoff |
| **Fuses** | 10A blade fuse | 2 | $1 each | Motor circuit protection |
| **Terminal Blocks** | 12-position | 2 | $5-10 each | Wire management |

---

## Total Cost Estimate

| Category | Low End | High End | Notes |
|----------|---------|----------|-------|
| **Microcontroller** | $5 | $10 | ESP32-C6 |
| **Sensors** | $35 | $60 | HWT905 + RS485 |
| **Motor Controllers** | $16 | $30 | 2x BTS7960 |
| **Motors** | $100 | $350 | Actuator + slewing |
| **Power Supplies** | $35 | $70 | 24V + 5V |
| **Switches/Hardware** | $20 | $50 | Limit switch, mounting |
| **Wiring/Connectors** | $20 | $40 | Cables, terminals |
| **Enclosure** | $15 | $30 | Weather protection |
| **Optional** | $0 | $50 | Sensors, safety |
| **TOTAL** | **$246** | **$690** | Excluding solar panels |

**Notes:**
- Prices in USD as of 2024
- AliExpress typically 40-60% cheaper than Amazon
- Quality varies - research seller reviews
- Shipping not included
- Tools not included (soldering, crimpers, etc.)

---

## Tools Required

### Essential
- Soldering iron and solder
- Wire strippers
- Crimping tool
- Multimeter
- Screwdriver set
- Drill and bits

### Recommended
- Heat shrink tubing and heat gun
- Cable ties
- Label maker
- Helping hands/PCB holder
- Fume extractor

### Nice to Have
- Oscilloscope (for debugging PWM)
- Logic analyzer (for RS485 debugging)
- Thermal camera (for heat management)

---

## Sourcing Tips

### Quality Considerations

**High Priority (don't cheap out):**
1. **Power Supply** - Use quality brand (Mean Well, TDK-Lambda)
2. **WIT-Motion Sensor** - Buy genuine, not clones
3. **Linear Actuator** - Critical for reliability
4. **Wiring** - Use correct gauge, quality insulation

**Can Save Money:**
- Generic BTS7960 (mostly same quality)
- ESP32-C6 clones (test before deploying)
- Enclosure (DIY from junction box)
- Connectors (AliExpress vs. Mouser)

### Vendor Recommendations

**Trusted Sources (Higher Cost):**
- Mouser Electronics
- Digikey
- Adafruit
- SparkFun
- Arrow Electronics

**Good Value (Moderate Risk):**
- Amazon (check reviews)
- eBay (verified sellers)
- Banggood
- RobotShop

**Lowest Cost (Higher Risk):**
- AliExpress (6-8 week shipping)
- Alibaba (bulk orders)
- DHgate

**Tips:**
- Always read reviews
- Check return policy
- Order 1-2 extras of critical components
- Allow extra time for shipping
- Test components upon arrival

---

## Pre-Purchase Checklist

Before ordering:

- [ ] Measured solar panel dimensions/weight
- [ ] Calculated required actuator stroke length
- [ ] Determined slewing drive torque requirements
- [ ] Verified 24V power supply sizing
- [ ] Confirmed RS485 interface on HWT905 order
- [ ] Checked BTS7960 includes heat sinks
- [ ] Selected weatherproof enclosure size
- [ ] Verified ESP32-C6 availability
- [ ] Ordered extra wire and connectors
- [ ] Have required tools available

---

## Assembly Order

Recommended build sequence:

1. **Test bench setup** - Test all electronics on workbench first
2. **ESP32 + HWT905** - Verify sensor communication
3. **BTS7960 testing** - Test motor controllers with test motor
4. **Power system** - Wire and test power supplies
5. **Homing switch** - Test limit switch triggering
6. **Integration** - Combine all components
7. **Enclosure** - Mount in weatherproof box
8. **Mechanical** - Install on tracker structure
9. **Final testing** - Full system test
10. **Weatherproofing** - Seal cable entries, test IP rating

---

## Useful Links

### Documentation
- ESP32-C6: https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/
- BTS7960: Search "BTS7960 datasheet" for pinout
- WIT-Motion: https://www.wit-motion.com/

### Calculators
- Wire gauge calculator: https://www.calculator.net/wire-size-calculator.html
- Actuator force calculator: (depends on panel size and angle)
- Power supply sizing: Volts × Amps = Watts

### Communities
- ESPHome Discord: Hardware help available
- Home Assistant Forums: Solar tracker projects
- Reddit r/esp32: Component recommendations

---

## Warranty & Support

**Typical Warranties:**
- ESP32 modules: 1-3 months
- BTS7960: 1-6 months
- Power supplies: 1-3 years (Mean Well)
- Linear actuators: 1-2 years
- WIT-Motion: 1 year

**Always:**
- Test immediately upon receipt
- Document issues with photos
- Contact seller within warranty period
- Keep all packaging for returns

---

## Safety Notes

⚠️ **Electrical Safety:**
- 24V DC is generally safe but use proper insulation
- Always disconnect power when working
- Use appropriate wire gauge for current
- Install fuses on motor circuits
- Ensure proper grounding

⚠️ **Mechanical Safety:**
- Linear actuators can exert significant force
- Add limit switches to prevent over-travel
- Emergency stop button recommended
- Test range of motion before mounting panels
- Ensure no pinch points

⚠️ **Weather Protection:**
- All outdoor electronics need IP65+ rating
- Use cable glands for all wire entries
- Apply dielectric grease to connectors
- Consider lightning protection for exposed metal
- Regular maintenance/inspection required

---

## Substitutions & Alternatives

If specific components unavailable:

**ESP32-C6 → ESP32-S3 or ESP32**
- Change board type in YAML
- Verify GPIO pin availability

**BTS7960 → VNH2SP30**
- Similar pinout and control
- Slightly lower current (30A vs 43A)
- May need code adjustments

**WIT-Motion HWT905 → MPU9250 + Processing**
- Much more complex software
- No RS485 (use I2C)
- Manual sensor fusion required
- Not recommended for beginners

**SeeedStudio RS485 → Generic MAX485**
- Verify pinout matches
- May need to control DE/RE manually
- Test before deployment

---

**Ready to order? Double-check quantities and specifications!**
