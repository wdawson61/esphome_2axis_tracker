# RS485 Interface Circuit - HWT905 IMU Communication
## THVD1426DR Auto-Direction Transceiver

**IC**: THVD1426DR  
**JLCPCB**: C5215921  
**Package**: SOIC-8  
**Purpose**: RS485 communication to WitMotion HWT905 IMU  

---

## 🎯 **Why THVD1426DR?**

**Advantages**:
- ✅ **Auto-direction control** - Uses DI pin, no separate DE/RE needed!
- ✅ **±12kV IEC ESD protection** - Built-in, reduces external TVS needs
- ✅ **12Mbps speed** - Way more than 115200 baud needed
- ✅ **3-5.5V operation** - Works with ESP32's 3.3V logic
- ✅ **Low power**: 1.7mA typical
- ✅ **TI part** - Matches DRV8243 motor drivers
- ✅ **Fail-safe** - Receiver output HIGH when bus is open/idle
- ✅ **On JLCPCB** - Extended part, available

---

## 📌 **THVD1426 Pinout - SOIC-8**

```
Top View:
    ┌──────────┐
 1  │R     VCC │ 8
 2  │NC    B   │ 7
 3  │NC    A   │ 6
 4  │D    GND  │ 5
    └──────────┘
```

**Pin Functions**:
- Pin 1 (R): Receiver Output → to ESP32 RX
- Pin 2,3 (NC): No Connect (auto-direction, no DE/RE pins!)
- Pin 4 (D): Driver Input → from ESP32 TX
- Pin 5 (GND): Ground
- Pin 6 (A): RS485 A (Data+)
- Pin 7 (B): RS485 B (Data-)
- Pin 8 (VCC): 3.3V or 5V power

**Auto-Direction Magic**:
The THVD1426 automatically switches between transmit and receive based on the D (Driver Input) pin activity. No need for ESP32 to control DE/RE!

---

## 🔌 **Complete RS485 Circuit**

### Component List
```
U5:    THVD1426DR (C5215921, SOIC-8) - RS485 transceiver
J1:    RJ45 shielded jack (C136744, Through-hole) - To HWT905

Termination:
R3:    120Ω (C22790, 0603) - RS485 termination A-B

Protection (optional, IC has ±12kV ESD):
D2:    PESD5V0S1BA (C558488, SOT-23) - Extra TVS on A (optional)
D3:    PESD5V0S1BA (C558488, SOT-23) - Extra TVS on B (optional)

Decoupling:
C7:    100nF (C14663, 0603) - VCC bypass
C8:    10µF (C15850, 0805) - VCC bulk

Fail-safe biasing (optional):
R7:    560Ω (C22849, 0603) - A to VCC (fail-safe HIGH)
R8:    560Ω (C22849, 0603) - B to GND (fail-safe LOW)
```

---

## 📐 **Complete Schematic**

```
THVD1426 Connections (SO SIMPLE!):
──────────────────────────────────

ESP32-C6 to THVD1426:
GPIO4 (RS485_TX) ────→ Pin 4 (D) - Data In to transceiver
GPIO5 (RS485_RX) ←──── Pin 1 (R) - Receive Out from transceiver

*** NO DE/RE PINS NEEDED! Auto-direction control! ***

Power:
3.3V ──┬─→ C7 (100nF) ──┬─→ Pin 8 (VCC)
       │                │
       └─→ C8 (10µF) ───┘

Note: Can use 5V if preferred, but 3.3V works fine

Pin 5 (GND) ─→ GND

Pin 2, 3 (NC) ─→ No connection (leave floating)

RS485 Bus:
Pin 6 (A) ──┬─→ D2 (TVS, optional) ─→ GND
            │
            ├─→ RJ45 Pin 1,2 (RS485 A, twisted pair)
            │
            ├─→ R7 (560Ω, optional fail-safe) ─→ 3.3V
            │
            └─→ R3 (120Ω termination) ─→ Pin 7 (B)

Pin 7 (B) ──┬─→ D3 (TVS, optional) ─→ GND
            │
            ├─→ RJ45 Pin 7,8 (RS485 B, twisted pair)
            │
            └─→ R8 (560Ω, optional fail-safe) ─→ GND


RJ45 Connector Pinout (to HWT905):
──────────────────────────────────

J1 (RJ45 Jack, C136744):
Pin 1: RS485 A (Data+)     ← From THVD1426 Pin 6
Pin 2: RS485 A (Data+)     ← Parallel for reliability
Pin 3: GND                 ← Common ground
Pin 4: 5V (Sensor Power)   ← From 5V rail (500mA max)
Pin 5: 5V (Sensor Power)   ← Parallel for current
Pin 6: GND                 ← Common ground  
Pin 7: RS485 B (Data-)     ← From THVD1426 Pin 7
Pin 8: RS485 B (Data-)     ← Parallel for reliability

Notes:
- Pins 1,2: Twisted pair for RS485 A
- Pins 7,8: Twisted pair for RS485 B
- Pins 3,6: Ground (can use as shield)
- Pins 4,5: 5V power to HWT905 (up to 500mA)
```

---

## 🔧 **Detailed Circuit Diagram**

```
                    U5: THVD1426DR
                    ┌──────────────┐
GPIO4 (TX) ────────→│4  D     VCC 8│←─┬── 3.3V
                    │              │  │
GPIO5 (RX) ←────────│1  R       B 7│──┼── RS485_B
                    │              │  │
      (NC) ─────────│2  NC      A 6│──┼── RS485_A
                    │              │  │
      (NC) ─────────│3  NC    GND 5│  │
                    └──────────────┘  │
                           │          │
                          GND        GND
                           │          │
                        C7 ┴ 100nF    │
                        C8 ┴ 10µF     │
                                      │
RS485 Bus with Protection:            │
──────────────────────────            │
                                      │
RS485_A (Pin 6) ──┬─→ D2 (TVS, opt) ─→ GND
                  │
                  ├─→ R7 (560Ω, opt) ─→ 3.3V (fail-safe)
                  │
                  ├─→ R3 (120Ω) ─────┬─→ RS485_B
                  │                  │
                  └─→ J1-1,2 ────────┘

RS485_B (Pin 7) ──┬─→ D3 (TVS, opt) ─→ GND
                  │
                  ├─→ R8 (560Ω, opt) ─→ GND (fail-safe)
                  │
                  └─→ J1-7,8

J1 (RJ45):
Pin 1,2:  RS485 A
Pin 3,6:  GND
Pin 4,5:  5V (to HWT905, 500mA max)
Pin 7,8:  RS485 B
```

---

## 🎛️ **Configuration Options**

### Minimal Configuration (Recommended)
```
Components needed:
- U5 (THVD1426DR)
- C7, C8 (decoupling)
- R3 (120Ω termination)
- J1 (RJ45 jack)

Total: 5 components!
```

**Why minimal works**:
- Built-in ±12kV ESD protection (TVS diodes optional)
- Built-in fail-safe (biasing resistors optional)
- Auto-direction control (no DE/RE needed)

### Full Protection Configuration
```
Add:
- D2, D3 (TVS diodes for extra ESD protection)
- R7, R8 (Fail-safe biasing)

Total: 9 components
```

**Use if**:
- Long cable runs (>10m)
- Harsh industrial environment
- Lightning protection needed

---

## ⚙️ **Firmware Configuration**

### ESP32 Arduino Code
```cpp
// RS485 with THVD1426 - SO SIMPLE!

#define RS485_TX_PIN 4  // GPIO4 to THVD1426 Pin 4 (D)
#define RS485_RX_PIN 5  // GPIO5 from THVD1426 Pin 1 (R)
// NO DE PIN NEEDED! Auto-direction! 🎉

HardwareSerial RS485(1);  // Use UART1

void setup() {
  // Initialize RS485 UART
  RS485.begin(115200, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);
  
  // That's it! No DE/RE pin control needed!
}

void loop() {
  // Send data - auto switches to transmit
  RS485.write(data_buffer, length);
  
  // Receive data - auto switches to receive
  if (RS485.available()) {
    byte received = RS485.read();
  }
  
  // The THVD1426 handles everything automatically!
}
```

**Comparison with manual DE/RE control**:
```cpp
// OLD WAY (MAX13487E or similar):
digitalWrite(DE_PIN, HIGH);  // Enable transmitter
RS485.write(data);
digitalWrite(DE_PIN, LOW);   // Enable receiver

// NEW WAY (THVD1426):
RS485.write(data);  // Done! Auto-direction handles it!
```

---

## 🧪 **Testing Procedure**

### Power-Up Test
```
1. Apply 3.3V to VCC (Pin 8)
2. Check voltage at Pin 8: Should be 3.3V
3. Check GND at Pin 5: Should be 0V
4. Measure Pin 1 (R) with no activity: Should be HIGH (~3.3V) - fail-safe
5. Short A and B together: Pin 1 should still be HIGH (fail-safe works)
```

### Loopback Test (No HWT905)
```
1. Remove RJ45 cable
2. Connect jumper: Pin 6 (A) to Pin 7 (B)  
3. ESP32 send test byte: 0x55
4. ESP32 should receive: 0x55
5. Remove jumper: Pin 1 should go HIGH (fail-safe)
```

### Communication Test (With HWT905)
```
1. Connect RJ45 cable to HWT905
2. Power on HWT905 (check 5V on J1 pins 4,5)
3. ESP32 send: 0xFF 0xAA 0x27 (read angle command)
4. Should receive back: angle data packet
5. Monitor with scope: See differential signal on A-B
```

### Signal Quality Test
```
Use oscilloscope:
1. Probe A and B differentially
2. Send continuous data pattern
3. Check:
   - Differential voltage: 2-6V typical
   - Rise/fall time: <50ns
   - No ringing or overshoot
   - Clean eye diagram at 115200 baud
```

---

## 🔥 **PCB Layout Guidelines**

### Component Placement
```
Recommended layout (left to right):

ESP32 ──→ [2-3cm] ──→ THVD1426 ──→ [<1cm] ──→ R3 ──→ RJ45
                         │
                      C7, C8
                      (close!)
```

### Critical Traces
```
1. VCC (Pin 8) to C7: <5mm, 10mil width
2. D (Pin 4) from ESP32: 20mil width, ground pour around
3. R (Pin 1) to ESP32: 20mil width, ground pour around
4. A (Pin 6) to RJ45: 50mil width, differential pair
5. B (Pin 7) to RJ45: 50mil width, differential pair
6. Keep A and B traces parallel and equal length (±2mm)
```

### Ground Plane
```
1. Solid GND plane under THVD1426
2. Connect Pin 5 with multiple vias to GND plane
3. Keep digital signals away from A/B traces
4. Use guard traces (grounded) around A/B if needed
```

### Differential Pair Routing
```
For A and B traces:
- Parallel routing: Yes
- Spacing: 15-20 mil
- Trace width: 15-20 mil (for 100Ω differential impedance)
- Length matching: ±2mm
- No vias if possible
- If vias needed: Use them on both traces symmetrically
```

---

## 💡 **Design Tips**

### Do's ✅
- Use 120Ω termination resistor (R3)
- Place decoupling caps (C7, C8) very close to IC
- Route A and B as differential pair
- Use twisted pair cable for A and B
- Ground the RJ45 shield to PCB GND
- Add test points on A and B for scope probing

### Don'ts ❌
- Don't route A and B on different layers
- Don't stub A or B traces
- Don't place high-speed digital near A/B
- Don't forget 120Ω termination
- Don't use ribbon cable (use twisted pair!)
- Don't exceed 500mA on 5V sensor power

---

## 🔍 **Troubleshooting**

### No Communication
```
Problem: ESP32 sends, no response from HWT905

Check:
1. VCC at Pin 8: Should be 3.3V
2. Pin 1 (R) idle state: Should be HIGH (~3.3V)
3. Measure A-B voltage: Should be 0V when idle
4. Check RJ45 cable: Correct pinout?
5. HWT905 power: 5V present on pins 4,5?
6. Baud rate: 115200 set correctly?
7. UART pins: TX=4, RX=5 correct in firmware?
```

### Garbled Data
```
Problem: Receiving corrupted data

Check:
1. Termination resistor R3: Is it 120Ω and connected?
2. Cable length: <30m recommended for 115200 baud
3. A and B traces: Are they differential pair?
4. Ground loop: Is there a proper ground connection?
5. EMI: Are A/B traces away from switching power supplies?
6. Baud rate: Exactly 115200 on both sides?
```

### Intermittent Communication
```
Problem: Sometimes works, sometimes doesn't

Check:
1. Power supply stability: Ripple on 3.3V rail?
2. Loose connections: RJ45 jack soldering?
3. Cable quality: Good twisted pair cable?
4. EMI/RFI: Near motors or high-power circuits?
5. Temperature: IC too hot? Check thermal management
```

---

## 📊 **Performance Specifications**

```
Parameter              | Min  | Typ  | Max  | Unit | Notes
-----------------------|------|------|------|------|------------------
Supply Voltage         | 3.0  | 3.3  | 5.5  | V    | Use 3.3V or 5V
Supply Current         | -    | 1.7  | 3.0  | mA   | Typical operation
Data Rate              | -    | -    | 12   | Mbps | We use 115.2 kbps
Differential Output    | 1.5  | -    | 5    | V    | Loaded bus
Common Mode Range      | -7   | -    | 12   | V    | Receiver input
ESD Protection (HBM)   | -    | ±12  | -    | kV   | IEC 61000-4-2
ESD Protection (IEC)   | -    | ±12  | -    | kV   | Contact discharge
Operating Temp         | -40  | 25   | 125  | °C   | Industrial grade
```

---

## 💰 **BOM Cost - RS485 Interface**

### Minimal Configuration
```
Component          | Qty | JLCPCB    | Unit Price | Total
-------------------|-----|-----------|------------|-------
THVD1426DR (SMD)   |  1  | C5215921  | $1.71      | $1.71
100nF Cap (SMD)    |  1  | C14663    | $0.001     | $0.00
10µF Cap (SMD)     |  1  | C15850    | $0.02      | $0.02
120Ω Resistor (SMD)|  1  | C22790    | $0.001     | $0.00
RJ45 Jack (TH)     |  1  | C136744   | $0.30      | $0.30
─────────────────────────────────────────────────────────
Total per board:                              ~$2.03
```

### Full Protection Configuration
```
Add:
TVS Diodes (2×)    |  2  | C558488   | $0.10      | $0.20
560Ω Resistors (2×)|  2  | C22849    | $0.001     | $0.00
─────────────────────────────────────────────────────────
Total per board:                              ~$2.23
```

**JLCPCB Assembly**: SMD components only (~$1.75 parts + $2 assembly)  
**Manual Assembly**: RJ45 jack (~$0.30, you solder)

---

## ✅ **Ready for KiCad Implementation**

This RS485 interface is **simple, robust, and cost-effective**!

Key advantages of THVD1426:
- ✅ **2-wire control** (TX, RX only - no DE!)
- ✅ **Built-in ESD protection**
- ✅ **Auto-direction** (firmware simplified)
- ✅ **Industrial grade** (-40 to +125°C)
- ✅ **Low cost** (~$2/board)
- ✅ **TI part** (matches motor drivers)
- ✅ **On JLCPCB** (C5215921)

Next up: ESP32-C6 connections and limit switches! 🚀
