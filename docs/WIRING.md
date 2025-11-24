# Wiring Diagram and Pin Assignments

## ESP32-C3 Pinout Reference

```
                    ESP32-C3
                 ┌─────────────┐
                 │             │
            3V3  │ 1       21  │  GPIO21 (TX) ──┐
            GND  │ 2       20  │  GPIO20 (RX) ──┤
            EN   │ 3       19  │                │
         GPIO4   │ 4       18  │                │
         GPIO5   │ 5       10  │  GPIO10 (DE) ──┤
         GPIO6   │ 6        9  │                │
         GPIO7   │ 7        8  │                │
                 │             │                │
                 └─────────────┘                │
                                                │
                                                │
                    RS485 Module               │
                 ┌─────────────┐               │
                 │             │               │
            VCC  │ 1       RO  │ ──────────────┘
            GND  │ 2       DI  │ ───────────────────┐
             DE  │ 3        A  │ ──┐                │
             RE  │ 4        B  │ ──┤                │
                 │             │   │                │
                 └─────────────┘   │                │
                      │    │       │                │
                      │    │       │                │
                      └────┘       │                │
                  (tie together)   │                │
                                   │                │
                                   │                │
                              Modbus Device         │
                            ┌─────────────┐         │
                            │             │         │
                            │      A/D+   │ ────────┘
                            │      B/D-   │ ─────────────┘
                            │             │
                            └─────────────┘
```

## Detailed Connection Table

### ESP32-C3 to RS485 Connections

| ESP32-C3 Pin | Function | RS485 Module Pin | RS485 Function |
|--------------|----------|------------------|----------------|
| GPIO20       | RX       | RO               | Receiver Output |
| GPIO21       | TX       | DI               | Driver Input |
| GPIO10       | DE/RE    | DE               | Driver Enable |
| GPIO10       | DE/RE    | RE               | Receiver Enable (LOW=RX) |
| 3.3V or 5V   | Power    | VCC              | Power Supply |
| GND          | Ground   | GND              | Ground |

### RS485 Module to Modbus Device

| RS485 Pin | Function | Modbus Terminal | Notes |
|-----------|----------|-----------------|-------|
| A         | Data +   | A or D+         | Non-inverting |
| B         | Data -   | B or D-         | Inverting |

## RS485 Module Types

### Common RS485 Modules

1. **MAX485 Module**
   - Most common
   - Single 5V or 3.3V supply
   - Separate DE and RE pins (can be tied together)

2. **SP485/SP3485 Module**
   - Similar to MAX485
   - Better ESD protection

3. **Automatic Direction Control Modules**
   - Only needs TX, RX (no DE pin)
   - Slightly more expensive
   - Simplifies wiring

## Power Supply Options

### Option 1: 3.3V Power (Recommended)
```
ESP32-C3 3.3V → RS485 Module VCC
ESP32-C3 GND  → RS485 Module GND
```
- Simpler
- Direct connection
- Suitable for most MAX485 modules

### Option 2: 5V Power
```
ESP32-C3 5V (USB) → RS485 Module VCC
ESP32-C3 GND      → RS485 Module GND
```
- Better for longer cables
- Stronger signal
- Check ESP32-C3 board has 5V pin available

## DE/RE Pin Configuration

The RS485 module has two control pins:
- **DE (Driver Enable)**: HIGH = Transmit mode
- **RE (Receiver Enable)**: LOW = Receive mode

### Wiring Options

**Option 1: Tie Together (Used in this project)**
```
ESP32 GPIO10 ─┬─ RS485 DE
              └─ RS485 RE
```
- Simplest method
- Uses one GPIO pin
- Software controls mode

**Option 2: Separate Control (Advanced)**
```
ESP32 GPIO10 ── RS485 DE
ESP32 GPIO9  ── RS485 RE (inverted)
```
- More control
- Uses two GPIO pins

## Cable Specifications

### RS485 Bus Cable Requirements

- **Cable Type**: Twisted pair
- **Impedance**: 120Ω characteristic
- **Gauge**: 22-24 AWG
- **Shielding**: Recommended for industrial environments
- **Max Length**: 1200m (4000ft) at 9600 baud

### Recommended Cables
- CAT5/CAT5e/CAT6 (use one twisted pair)
- Belden 9842 (2-conductor twisted pair)
- Industrial RS485 cable

## Termination Resistors

For reliable communication on longer cables (>10m):

```
    120Ω                                    120Ω
     ┌─┐                                    ┌─┐
  A ─┤ ├─ A ═══════════════════════════ A ─┤ ├─ A
     └─┘                                    └─┘
  B ────── B ═══════════════════════════ B ────── B

  First Device                          Last Device
```

- Place 120Ω resistor between A and B
- Only at first and last device on bus
- Not needed for short cables (<3m)

## Testing Setup

### Minimal Test Configuration

```
USB Cable
   │
   ↓
┌─────────────┐
│  ESP32-C3   │
│             │
│  GPIO20 ────┼─── RO
│  GPIO21 ────┼─── DI      ┌─────────────┐
│  GPIO10 ────┼─── DE/RE   │  RS485      │
│  3.3V   ────┼─── VCC ────┼─ A      A ──┼── Modbus
│  GND    ────┼─── GND     │  B      B ──┼── Device
└─────────────┘            └─────────────┘
                                │
                           (Short cable,
                            no termination
                            needed)
```

## Common Wiring Mistakes

❌ **Wrong Polarity**
- Swapping A and B lines (won't work at all)

❌ **Missing Ground**
- Not connecting GND between ESP32 and RS485 module

❌ **DE/RE Not Controlled**
- Leaving DE/RE floating or tied to wrong state

❌ **Wrong Voltage**
- Using 5V on ESP32 GPIO pins (use 3.3V logic)

❌ **No Termination on Long Cables**
- Reflections cause errors on cables >10m

## Voltage Levels

### Logic Levels
| Signal | ESP32-C3 | MAX485 Module |
|--------|----------|---------------|
| HIGH   | 3.3V     | 3.3V or 5V    |
| LOW    | 0V       | 0V            |

### RS485 Differential Levels
| State | Voltage (A-B) |
|-------|---------------|
| Mark (1) | -200mV to -6V |
| Space (0) | +200mV to +6V |

## Multi-Drop Configuration

For multiple devices on one RS485 bus:

```
┌──────────┐     ┌──────────┐     ┌──────────┐     ┌──────────┐
│ ESP32-C3 │     │ Slave 1  │     │ Slave 2  │     │ Slave 3  │
│ (Master) │     │  Addr=1  │     │  Addr=2  │     │  Addr=3  │
└────┬─┬───┘     └────┬─┬───┘     └────┬─┬───┘     └────┬─┬───┘
     A B              A B              A B              A B
     │ │              │ │              │ │              │ │
     └─┼──────────────┼─┼──────────────┼─┼──────────────┼─┘
       └──────────────┴─┴──────────────┴─┴──────────────┘

   120Ω                                              120Ω
  Termination                                    Termination
```

- Maximum 32 devices (standard)
- Maximum 256 devices (with repeaters)
- Each slave must have unique address

## Alternative Pin Assignments

If GPIO20/21/10 are not available:

```cpp
// Any available GPIO can be used
#define RS485_RX 3   // Change to available pin
#define RS485_TX 4   // Change to available pin  
#define RS485_DE 5   // Change to available pin
```

Note: Some pins on ESP32-C3 are strapping pins - avoid GPIO2, GPIO8, GPIO9 if possible.

## PCB Layout Recommendations

If designing a custom PCB:

1. Keep RS485 traces short and parallel
2. Place termination resistor close to connector
3. Use ground plane under RS485 traces
4. Add TVS diodes for ESD protection
5. Separate analog/digital grounds if mixed signal
