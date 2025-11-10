# Hardware Wiring Diagram

## Complete System Wiring

```
                                    ┌──────────────────────────────────┐
                                    │     ESP32-C6 DevKit              │
                                    │                                  │
                                    │  GPIO16 (TX) ──────┐            │
                                    │  GPIO17 (RX) ──────┤            │
                                    │                    │            │
                                    │  GPIO4  ──────┐    │            │
                                    │  GPIO5  ──────┤    │            │
                                    │  GPIO6  ──────┤    │            │
                                    │  GPIO7  ──────┤    │            │
                                    │                │    │            │
                                    │  3.3V ─────────┤    │            │
                                    │  GND ──────────┼────┼────┐       │
                                    │  USB-C (Power) │    │    │       │
                                    └────────────────┼────┼────┼───────┘
                                                     │    │    │
                    ┌────────────────────────────────┘    │    │
                    │                                     │    │
                    │  RS485 TRANSCEIVER                  │    │
                    │  ┌──────────────────┐               │    │
                    ├──┤ TX  (to GPIO16)  │               │    │
                    ├──┤ RX  (from GPIO17)│               │    │
                    │  │ VCC (3.3V/5V)    │───────────────┘    │
                    │  │ GND              │────────────────────┤
                    │  │ A+  ─────────┐   │                    │
                    │  │ B-  ─────────┤   │                    │
                    │  └──────────────┼───┘                    │
                    │                 │                        │
                    │  WIT-MOTION     │                        │
                    │  HWT905         │                        │
                    │  ┌──────────────┼──┐                     │
                    │  │ RS485 A+  ───┘  │                     │
                    │  │ RS485 B-  ──────┘                     │
                    │  │ VCC (5-27V)     │                     │
                    │  │ GND             │─────────────────────┤
                    │  └─────────────────┘                     │
                    │                                          │
                    │  ELEVATION MOTOR DRIVER (H-Bridge #1)    │
                    │  ┌──────────────────────────┐            │
                    │  │ IN1  <───────────────────┤ GPIO4      │
                    │  │ IN2  <───────────────────┤ GPIO5      │
                    │  │ VCC  (Logic)             │────────────┤
                    │  │ GND  (Logic)             │────────────┤
                    │  │ VMS  (Motor Power 12V)   │◄───────┐   │
                    │  │ GND  (Motor)             │────────┼───┤
                    │  │ OUT1 ───────┐            │        │   │
                    │  │ OUT2 ───────┤            │        │   │
                    │  └─────────────┼────────────┘        │   │
                    │                │                     │   │
                    │  LINEAR        │                     │   │
                    │  ACTUATOR      │                     │   │
                    │  ┌─────────────┼────┐                │   │
                    │  │ Motor+ ─────┘    │                │   │
                    │  │ Motor- ──────────┘                │   │
                    │  └──────────────────┘                │   │
                    │                                      │   │
                    │  AZIMUTH MOTOR DRIVER (H-Bridge #2)  │   │
                    │  ┌──────────────────────────┐        │   │
                    │  │ IN1  <───────────────────┤ GPIO6  │   │
                    │  │ IN2  <───────────────────┤ GPIO7  │   │
                    │  │ VCC  (Logic)             │────────┤   │
                    │  │ GND  (Logic)             │────────┤   │
                    │  │ VMS  (Motor Power 12V)   │◄───────┤   │
                    │  │ GND  (Motor)             │────────┤   │
                    │  │ OUT1 ───────┐            │        │   │
                    │  │ OUT2 ───────┤            │        │   │
                    │  └─────────────┼────────────┘        │   │
                    │                │                     │   │
                    │  AZIMUTH       │                     │   │
                    │  MOTOR         │                     │   │
                    │  ┌─────────────┼────┐                │   │
                    │  │ Motor+ ─────┘    │                │   │
                    │  │ Motor- ──────────┘                │   │
                    │  └──────────────────┘                │   │
                    │                                      │   │
                    │  POWER SUPPLY                        │   │
                    │  ┌──────────────────────────┐        │   │
                    │  │ 12V ─────────────────────┼────────┘   │
                    │  │ GND ─────────────────────┼────────────┘
                    │  └──────────────────────────┘
                    └────────────────────────────────────────────
```

## Component Details

### ESP32-C6 DevKit Pinout (Used Pins)

| Pin    | Function              | Connection                |
|--------|-----------------------|---------------------------|
| GPIO16 | UART TX               | RS485 Transceiver TX      |
| GPIO17 | UART RX               | RS485 Transceiver RX      |
| GPIO4  | Digital Output        | Elevation Motor Forward   |
| GPIO5  | Digital Output        | Elevation Motor Backward  |
| GPIO6  | Digital Output        | Azimuth Motor Forward     |
| GPIO7  | Digital Output        | Azimuth Motor Backward    |
| 3.3V   | Power                 | RS485 Module VCC (if 3.3V)|
| GND    | Ground                | Common Ground             |

### RS485 Transceiver Module

Common modules: MAX485, MAX3485, etc.

| Pin | Function                  | Connection              |
|-----|---------------------------|-------------------------|
| TX  | Transmit Data             | ESP32 GPIO16            |
| RX  | Receive Data              | ESP32 GPIO17            |
| VCC | Power (3.3V or 5V)        | ESP32 3.3V or External  |
| GND | Ground                    | Common GND              |
| A+  | RS485 A (Non-inverted)    | HWT905 A+               |
| B-  | RS485 B (Inverted)        | HWT905 B-               |

**Note:** Some modules have RE/DE (Receive/Driver Enable) pins. Connect both to ESP32 GPIO or tie to appropriate logic levels.

### WIT-Motion HWT905 IMU

| Pin     | Function              | Connection        |
|---------|-----------------------|-------------------|
| VCC     | Power (5-27V DC)      | 5V or 12V Supply  |
| GND     | Ground                | Common GND        |
| RS485-A | RS485 A+ (Green)      | Transceiver A+    |
| RS485-B | RS485 B- (Blue/White) | Transceiver B-    |

**Default Settings:**
- Baud Rate: 115200 (can be configured)
- Protocol: WIT Binary Protocol
- Output Rate: 100Hz default

### H-Bridge Motor Driver (e.g., L298N)

| Pin  | Function              | Connection              |
|------|-----------------------|-------------------------|
| IN1  | Input 1               | ESP32 GPIO4 or GPIO6    |
| IN2  | Input 2               | ESP32 GPIO5 or GPIO7    |
| VCC  | Logic Power (5V)      | 5V Supply               |
| GND  | Logic Ground          | Common GND              |
| VMS  | Motor Power (12V)     | 12V Supply              |
| GND  | Power Ground          | Common GND              |
| OUT1 | Motor Output 1        | Motor +                 |
| OUT2 | Motor Output 2        | Motor -                 |
| ENA  | Enable A (PWM)        | High (or PWM if needed) |

**Truth Table:**
| IN1 | IN2 | Motor Action |
|-----|-----|--------------|
| LOW | LOW | Stop/Brake   |
| HIGH| LOW | Forward      |
| LOW | HIGH| Reverse      |
| HIGH| HIGH| Brake        |

## Power Requirements

### Power Supplies

**Option 1: Separate Supplies (Recommended)**
- **Logic Supply:** 5V, 1A (for ESP32-C6)
  - Use USB-C power adapter or regulated 5V supply
- **Motor Supply:** 12V, 3-5A (for motors)
  - Use 12V DC power adapter rated for total motor current
- **Sensor Supply:** 5-12V shared with HWT905

**Option 2: Single Supply**
- Use a 12V supply with:
  - Buck converter (12V → 5V) for ESP32
  - Direct 12V to motor drivers and HWT905

### Current Requirements

| Component           | Voltage | Current    |
|---------------------|---------|------------|
| ESP32-C6            | 5V      | 200-500mA  |
| WIT-Motion HWT905   | 5-27V   | 50-100mA   |
| Linear Actuator     | 12V     | 1-3A       |
| Azimuth Motor       | 12V     | 1-3A       |
| H-Bridge Driver     | 12V     | 50mA logic |

**Total Estimated:** 5V@1A + 12V@5A (max with both motors running)

## Wire Gauge Recommendations

| Connection Type          | Wire Gauge (AWG) | Notes                    |
|--------------------------|------------------|--------------------------|
| ESP32 Logic Signals      | 22-26 AWG        | Low current              |
| RS485 Communication      | 22-24 AWG        | Twisted pair recommended |
| Motor Power (12V)        | 16-18 AWG        | Short runs (<3ft)        |
| Motor Power (12V, long)  | 14-16 AWG        | Longer runs (3-10ft)     |
| Ground/Common            | 16-18 AWG        | Heavy gauge recommended  |

## Cable Lengths

| Connection              | Maximum Length | Notes                           |
|-------------------------|----------------|---------------------------------|
| UART to RS485           | 6 ft (2m)      | Short as possible               |
| RS485 Bus               | 4000 ft (1200m)| Use shielded twisted pair       |
| Motor Driver to Motor   | 10 ft (3m)     | Keep short to reduce voltage drop|
| Power Supply to Drivers | 10 ft (3m)     | Use adequate wire gauge         |

## Grounding

**Critical:** All components must share a common ground!

```
Power Supply GND
    ├─── ESP32 GND
    ├─── RS485 Module GND
    ├─── HWT905 GND
    ├─── H-Bridge #1 Logic GND
    ├─── H-Bridge #1 Power GND
    ├─── H-Bridge #2 Logic GND
    └─── H-Bridge #2 Power GND
```

## Additional Components (Recommended)

### Protection

1. **Reverse Polarity Protection**
   - Diode (1N5400 or higher) in series with power input

2. **Motor Driver Protection**
   - Flyback diodes (usually built into H-bridge modules)
   - Capacitors (100µF) across motor terminals

3. **Logic Protection**
   - 100nF capacitors near ESP32 VCC pin
   - Ferrite beads on motor power lines

### Optional Sensors

1. **Limit Switches**
   - Connect to ESP32 GPIO pins (e.g., GPIO8, GPIO9)
   - Use pull-up resistors (10kΩ)
   - Add in YAML:
     ```yaml
     binary_sensor:
       - platform: gpio
         pin:
           number: GPIO8
           mode: INPUT_PULLUP
         name: "Elevation Limit Min"
     ```

2. **Current Sensors**
   - INA219 or INA226 modules
   - Monitor motor current draw
   - Detect stalls or overload

3. **Voltage Monitor**
   - ADC pin (ESP32 GPIO1-5)
   - Voltage divider to monitor supply voltage

## Testing Procedure

1. **Power Test (No Motors)**
   - Connect only ESP32 and verify 5V supply
   - Check WiFi connectivity
   - Verify serial logs

2. **Sensor Test**
   - Connect RS485 and HWT905
   - Verify sensor readings in web interface
   - Check heading, acceleration data

3. **Motor Test (One at a time)**
   - Connect one motor driver and motor
   - Test with small movements (1-2 degrees)
   - Verify direction and stop functions

4. **Full System Test**
   - Connect all components
   - Test with increasing angles
   - Monitor for overheating

## Safety Checklist

- [ ] All grounds connected to common point
- [ ] Motor voltage within specification
- [ ] Wire gauge adequate for current
- [ ] No loose connections
- [ ] Motors can move freely (no obstructions)
- [ ] Emergency stop accessible
- [ ] Initial testing done at low speeds/angles
- [ ] Limit switches functional (if installed)
- [ ] Adequate ventilation for motor drivers
- [ ] Fire extinguisher nearby for initial testing

## Common Wiring Mistakes

1. **Floating Ground:** Not connecting all grounds together
2. **Reversed RS485:** Swapping A+ and B- pins
3. **Insufficient Power:** Undersized power supply or wires
4. **Motor Phase:** Motors connected backward (just swap wires)
5. **Logic Levels:** Using 5V signals on 3.3V-only pins (ESP32 GPIOs are 5V tolerant, but verify)
6. **No Current Limit:** Not using motor driver current limiting
7. **Long Unshielded Wires:** EMI interference with RS485

## Troubleshooting

See the main README.md and QUICKSTART.md for detailed troubleshooting steps.
