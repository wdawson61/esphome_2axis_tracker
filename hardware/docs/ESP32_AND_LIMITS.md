# ESP32-C6 Module & Limit Switches
## Complete Connection Guide

---

## 📱 **ESP32-C6 Module (XIAO)**

**Module**: Seeed XIAO ESP32-C6  
**JLCPCB**: Manual assembly (not on JLCPCB)  
**Source**: Seeed Studio, DigiKey, Mouser  
**Price**: ~$7/unit  

---

## 🔌 **XIAO ESP32-C6 Pinout**

```
        USB-C
          │
    ┌─────┴─────┐
    │   XIAO    │
    │  ESP32-C6 │
    │           │
Left Side:       │  Right Side:
D0  ─────────────┘  ───────── D10
D1  ─────────────┐  ───────── D9
D2  ─────────────┤  ───────── D8
D3  ─────────────┤  ───────── D7
D4  ─────────────┤  ───────── D20
D5  ─────────────┤  ───────── D21
D6  ─────────────┤  ───────── 3V3
5V  ─────────────┘  ───────── GND
```

**Physical Pins** (castellated edges for SMT, or add headers):
- 14 GPIO pins total
- USB-C for programming
- Built-in 3.3V regulator (from 5V input)
- 5V input from buck converter

---

## 📐 **Complete Pin Assignments**

### Motor Control Outputs
```
D0  (GPIO0)  → Motor 1 PH (Direction)     → U1 Pin 10
D1  (GPIO1)  → Motor 1 EN (PWM Speed)     → U1 Pin 9
D2  (GPIO2)  → Motor 2 PH (Direction)     → U2 Pin 10
D3  (GPIO3)  → Motor 2 EN (PWM Speed)     → U2 Pin 9
```

### RS485 Communication
```
D4  (GPIO4)  → RS485 TX                   → U5 Pin 4 (D)
D5  (GPIO5)  → RS485 RX                   → U5 Pin 1 (R)
D6  (GPIO6)  → NOT USED (auto-direction on THVD1426)
```

### Limit Switches Inputs
```
D7  (GPIO7)  → Limit East                 → J2 Pin 1
D8  (GPIO8)  → Limit West                 → J2 Pin 3
```

### Motor Driver Faults & Current Sense
```
D9  (GPIO9)  → Motor 1 nFAULT (input)     → U1 Pin 9
D10 (GPIO10) → Motor 2 nFAULT (input)     → U2 Pin 9
D20 (GPIO20) → Motor 1 IPROPI (ADC)       → U1 Pin 2
D21 (GPIO21) → Motor 2 IPROPI (ADC)       → U2 Pin 2
```

### LED Indicators
```
GPIO18 → LED_MOTOR1 (yellow)
GPIO19 → LED_MOTOR2 (yellow)
GPIO22 → LED_POWER (green)
GPIO23 → LED_ACTIVITY (blue)
```

### Power
```
5V   → 5V from buck converter (TPS54302 output)
GND  → Common ground
3V3  → Output from module (to RS485, pull-ups, LEDs)
```

---

## 🔧 **ESP32-C6 Schematic Block**

```
Power Input:
───────────
5V (from U4 buck) ──┬─→ XIAO 5V pin
                    │
                    └─→ C13 (10µF, C15850) ─→ GND

GND ─────────────────→ XIAO GND pin

3V3 Output:
───────────
XIAO 3V3 pin ─────┬─→ C14 (10µF, C15850) ─→ GND
                  │
                  ├─→ THVD1426 VCC (U5 Pin 8)
                  │
                  ├─→ Pull-up resistors (motor faults, limits)
                  │
                  └─→ LED circuits


Motor 1 Control:
───────────────
XIAO D0 (GPIO0) ────→ Motor Driver 1 Pin 10 (PH)
XIAO D1 (GPIO1) ────→ Motor Driver 1 Pin 9 (EN)
XIAO D9 (GPIO9) ←──── Motor Driver 1 Pin 1 (nFAULT)
XIAO D20(GPIO20)←──── Motor Driver 1 Pin 2 (IPROPI)


Motor 2 Control:
───────────────
XIAO D2 (GPIO2) ────→ Motor Driver 2 Pin 10 (PH)
XIAO D3 (GPIO3) ────→ Motor Driver 2 Pin 9 (EN)
XIAO D10(GPIO10)←──── Motor Driver 2 Pin 1 (nFAULT)
XIAO D21(GPIO21)←──── Motor Driver 2 Pin 2 (IPROPI)


RS485 Interface:
───────────────
XIAO D4 (GPIO4) ────→ THVD1426 Pin 4 (D)
XIAO D5 (GPIO5) ←──── THVD1426 Pin 1 (R)


Limit Switches:
──────────────
XIAO D7 (GPIO7) ←──── Limit East signal
XIAO D8 (GPIO8) ←──── Limit West signal


LEDs:
────
XIAO GPIO18 ─→ R_LED3 (330Ω) ─→ LED3 (Yellow) ─→ GND
XIAO GPIO19 ─→ R_LED4 (330Ω) ─→ LED4 (Yellow) ─→ GND
XIAO GPIO22 ─→ R_LED1 (330Ω) ─→ LED1 (Green)  ─→ GND
XIAO GPIO23 ─→ R_LED2 (330Ω) ─→ LED2 (Blue)   ─→ GND
```

---

## 💡 **LED Indicator Circuit**

### Component List
```
LED1: Green 0805 (C2297) - Power
LED2: Blue 0805 (C72041) - Activity/WiFi
LED3: Yellow 0805 (C2296) - Motor 1 active
LED4: Yellow 0805 (C2296) - Motor 2 active
LED5: Red 0805 (C84256) - Fault (optional)

R_LED1-4: 330Ω 0603 (C23138) - LED current limit
R_LED5: 330Ω 0603 (C23138) - Fault LED
```

### Circuit
```
Power LED (always on):
────────────────────
3V3 ─→ R_LED1 (330Ω) ─→ LED1 (Green) ─→ GND
Current: (3.3V - 2.0V) / 330Ω ≈ 4mA


Activity LED (firmware controlled):
──────────────────────────────────
GPIO23 ─→ R_LED2 (330Ω) ─→ LED2 (Blue) ─→ GND
Current: (3.3V - 2.8V) / 330Ω ≈ 1.5mA (blue has higher Vf)


Motor LEDs (firmware controlled):
────────────────────────────────
GPIO18 ─→ R_LED3 (330Ω) ─→ LED3 (Yellow) ─→ GND
GPIO19 ─→ R_LED4 (330Ω) ─→ LED4 (Yellow) ─→ GND
Current: (3.3V - 2.0V) / 330Ω ≈ 4mA each


Fault LED (optional, OR gate from both nFAULT):
──────────────────────────────────────────────
U1 Pin 1 (nFAULT1) ──┬── D6 (1N4148) ───┬─→ R_LED5 (330Ω) ─→ LED5 (Red) ─→ GND
                     │                   │
U2 Pin 1 (nFAULT2) ──┴── D7 (1N4148) ───┘
                                         │
                                     R_PULL (10kΩ) ─→ 3V3

When either nFAULT goes LOW, diode conducts and LED lights.
```

---

## 🚪 **Limit Switch Interface**

### Component List
```
J2:    RJ45 shielded jack (C136744, Through-hole)
R5:    10kΩ 0603 (C25804) - East limit pull-up
R6:    10kΩ 0603 (C25804) - West limit pull-up
D4:    PESD5V0S1BA (C558488, SOT-23) - TVS on East
D5:    PESD5V0S1BA (C558488, SOT-23) - TVS on West
C15:   100nF 0603 (C14663) - East debounce (optional)
C16:   100nF 0603 (C14663) - West debounce (optional)
```

### RJ45 Pinout (J2)
```
Pin 1: Limit East Signal
Pin 2: GND
Pin 3: Limit West Signal
Pin 4: GND
Pin 5: NC (or +5V for active sensors)
Pin 6: NC (or +5V for active sensors)
Pin 7: NC
Pin 8: NC
```

### Circuit
```
East Limit Switch:
─────────────────
3V3 ─→ R5 (10kΩ) ──┬─→ GPIO7 (input)
                   │
                   ├─→ D4 (TVS to GND)
                   │
                   ├─→ C15 (100nF to GND, debounce)
                   │
                   └─→ J2 Pin 1 ───┐
                                   │
           (External Switch) ──────┴─→ J2 Pin 2 (GND)

Logic:
- Switch open: GPIO7 = HIGH (pulled up)
- Switch closed: GPIO7 = LOW (grounded)


West Limit Switch:
─────────────────
3V3 ─→ R6 (10kΩ) ──┬─→ GPIO8 (input)
                   │
                   ├─→ D5 (TVS to GND)
                   │
                   ├─→ C16 (100nF to GND, debounce)
                   │
                   └─→ J2 Pin 3 ───┐
                                   │
           (External Switch) ──────┴─→ J2 Pin 4 (GND)

Logic:
- Switch open: GPIO8 = HIGH (pulled up)
- Switch closed: GPIO8 = LOW (grounded)
```

---

## 🔌 **External Limit Switch Wiring**

### Passive Switch (NC - Normally Closed recommended)
```
   At East Limit             RJ45 Cable              PCB
   ─────────────             ──────────              ───
   
   [NC Switch] ──── Pin 1 (Signal) ──── R5 pull-up ──── GPIO7
        │
       GND ───────── Pin 2 (GND) ───────── GND ───────── GND

Normal operation: Switch closed, GPIO7 = LOW
At limit: Switch opens, GPIO7 = HIGH
```

**Why NC (Normally Closed)?**
- Fail-safe: Wire break = limit triggered (safe)
- Broken wire detected immediately
- More reliable than NO (Normally Open)

### Active Switch (less common)
```
   At East Limit             RJ45 Cable              PCB
   ─────────────             ──────────              ───
   
   [Sensor] ─┬── Pin 5 (5V Power) ──── 5V rail
             │
             └── Pin 1 (Signal) ──── GPIO7 (already has pull-up)
   
   GND ─────── Pin 2 (GND) ──────── GND
   
Sensor output: HIGH when not triggered, LOW when triggered
```

---

## 📊 **Complete BOM - ESP32 & Accessories**

```
Component          | Qty | Footprint | JLCPCB    | Price  | Assembly
-------------------|-----|-----------|-----------|--------|----------
XIAO ESP32-C6      |  1  | Module    | Manual    | $7.00  | Manual
10µF Cap           |  2  | 0805      | C15850    | $0.04  | JLCPCB
10kΩ Resistor      |  2  | 0603      | C25804    | $0.00  | JLCPCB
330Ω Resistor      |  4  | 0603      | C23138    | $0.00  | JLCPCB
Green LED          |  1  | 0805      | C2297     | $0.01  | JLCPCB
Blue LED           |  1  | 0805      | C72041    | $0.01  | JLCPCB
Yellow LED         |  2  | 0805      | C2296     | $0.02  | JLCPCB
Red LED (opt)      |  1  | 0805      | C84256    | $0.01  | JLCPCB
TVS Diode          |  2  | SOT-23    | C558488   | $0.20  | JLCPCB
100nF Cap (opt)    |  2  | 0603      | C14663    | $0.00  | JLCPCB
RJ45 Jack          |  1  | TH        | C136744   | $0.30  | Manual
────────────────────────────────────────────────────────────────────
Total per board:                              ~$7.59
```

**Manual Assembly**: XIAO module, RJ45 jack  
**JLCPCB Assembly**: All SMD components

---

## 🧪 **Testing Procedures**

### Power-On Test
```
1. Apply 5V to XIAO 5V pin
2. Check 3.3V output: Should be 3.28-3.35V
3. Check LED1 (green): Should be lit (power indicator)
4. Press BOOT button on XIAO: LED should change (board alive)
5. Check current draw: <100mA idle
```

### GPIO Output Test
```
1. Upload blink sketch to GPIO18
2. LED3 (yellow) should blink
3. Measure GPIO18 with DMM: 0V / 3.3V alternating
4. Repeat for other GPIOs
```

### GPIO Input Test (Limit Switches)
```
1. Measure GPIO7 (East limit): Should be HIGH (~3.3V)
2. Short J2 Pin 1 to Pin 2 (GND)
3. GPIO7 should go LOW (~0V)
4. Remove short: GPIO7 returns to HIGH
5. Repeat for GPIO8 (West limit)
```

### Motor Control Test
```
1. Set GPIO0 (M1_PH) = LOW
2. Set GPIO1 (M1_EN) = 50% PWM
3. Measure U1 Pin 10: Should be LOW
4. Measure U1 Pin 9: Should show 50% duty PWM
5. Motors not connected yet!
```

### RS485 Test
```
1. Loopback: Connect U5 Pin 6 (A) to Pin 7 (B)
2. Send 0x55 from GPIO4 (TX)
3. Should receive 0x55 on GPIO5 (RX)
4. Remove loopback
5. Connect HWT905 and test real communication
```

---

## ✅ **Firmware Configuration**

### Pin Definitions
```cpp
// Motor 1 - Azimuth (Slewing Drive)
#define M1_PH_PIN    0   // GPIO0 - Direction
#define M1_EN_PIN    1   // GPIO1 - PWM Speed
#define M1_FAULT_PIN 9   // GPIO9 - Fault input
#define M1_ISENSE_PIN 20 // GPIO20 - Current sense ADC

// Motor 2 - Elevation (Linear Actuator)
#define M2_PH_PIN    2   // GPIO2 - Direction
#define M2_EN_PIN    3   // GPIO3 - PWM Speed
#define M2_FAULT_PIN 10  // GPIO10 - Fault input
#define M2_ISENSE_PIN 21 // GPIO21 - Current sense ADC

// RS485 - HWT905 IMU
#define RS485_TX_PIN 4   // GPIO4 - TX to THVD1426
#define RS485_RX_PIN 5   // GPIO5 - RX from THVD1426
// No DE pin needed - auto-direction!

// Limit Switches
#define LIM_EAST_PIN 7   // GPIO7 - East limit (NC switch)
#define LIM_WEST_PIN 8   // GPIO8 - West limit (NC switch)

// LEDs
#define LED_POWER_PIN  22  // GPIO22 - Green (always on)
#define LED_ACTIVITY_PIN 23 // GPIO23 - Blue (WiFi/heartbeat)
#define LED_MOTOR1_PIN 18  // GPIO18 - Yellow (M1 active)
#define LED_MOTOR2_PIN 19  // GPIO19 - Yellow (M2 active)
```

### Setup Code
```cpp
void setup() {
  // Motor outputs
  pinMode(M1_PH_PIN, OUTPUT);
  pinMode(M1_EN_PIN, OUTPUT);
  pinMode(M2_PH_PIN, OUTPUT);
  pinMode(M2_EN_PIN, OUTPUT);
  
  // Motor faults (inputs with pull-ups)
  pinMode(M1_FAULT_PIN, INPUT_PULLUP);
  pinMode(M2_FAULT_PIN, INPUT_PULLUP);
  
  // Limit switches (inputs with external pull-ups)
  pinMode(LIM_EAST_PIN, INPUT);  // External pull-up on board
  pinMode(LIM_WEST_PIN, INPUT);  // External pull-up on board
  
  // LEDs
  pinMode(LED_POWER_PIN, OUTPUT);
  digitalWrite(LED_POWER_PIN, HIGH);  // Power LED always on
  
  pinMode(LED_ACTIVITY_PIN, OUTPUT);
  pinMode(LED_MOTOR1_PIN, OUTPUT);
  pinMode(LED_MOTOR2_PIN, OUTPUT);
  
  // RS485 UART
  Serial1.begin(115200, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);
  
  // ADC for current sensing
  analogReadResolution(12);  // 12-bit ADC on ESP32-C6
}
```

---

## ✅ **Complete and Ready!**

You now have **ALL circuit blocks**:
1. ✅ Power Supply (TPS54302)
2. ✅ Motor Drivers (2× DRV8243H)
3. ✅ RS485 Interface (THVD1426)
4. ✅ ESP32-C6 Module connections
5. ✅ Limit Switches
6. ✅ LED Indicators

**Total board cost**: ~$42/board (10 qty)
- Components: ~$27
- PCB + Assembly: ~$15

**Ready to implement in KiCad!** 🚀
