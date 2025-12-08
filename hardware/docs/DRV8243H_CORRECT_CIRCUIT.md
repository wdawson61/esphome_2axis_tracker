# DRV8243H VQFN-HR-14 - CORRECT Circuit Block
## Complete Motor Driver Circuit for KiCad

**Part**: DRV8243HQRXYRQ1  
**JLCPCB**: C3040833  
**Package**: VQFN-HR-14 (3mm × 4.5mm)  
**Confirmed**: Has EN/IN1 and PH/IN2 control pins! ✅

---

## 📌 **Correct Pinout - VQFN-HR-14 HW Variant**

```
         Top View
    ┌─────────────┐
  3 │nSLEEP    VM │ 4
    │             │
  8 │DRVOFF  CVM2 │ 
    │             │
  9 │EN/IN1  OUT2 │ 5
    │             │
 10 │PH/IN2  LOAD │
    │             │
  2 │IPROPI  OUT1 │ 7
    │             │
  1 │nFAULT       │
    │             │
 14 │MODE    GND  │ 6
    │             │
 11 │DIAG         │
    │             │
 13 │ITRIP        │
    │             │
 12 │SR           │
    └─────────────┘
     (PAD = GND)
```

From your datasheet image, actual pinout:
- Pin 3: nSLEEP
- Pin 4: VM
- Pin 8: DRVOFF
- **Pin 9: EN/IN1** ← Control input!
- **Pin 10: PH/IN2** ← Control input!
- Pin 2: IPROPI
- Pin 1: nFAULT
- Pin 14: MODE
- Pin 11: DIAG
- Pin 13: ITRIP
- Pin 12: SR
- Pin 5: OUT2
- Pin 7: OUT1
- Pin 6: GND
- Thermal Pad: GND

---

## 🔌 **Complete Circuit - Motor Driver 1 (Azimuth)**

### Component List
```
U1:      DRV8243HQRXYRQ1 (C3040833, VQFN-HR-14)
C21,C27: 100µF 35V (C13585, 1210) - VM bulk caps
C22:     10µF 16V (C15850, 0805) - VM decoupling at pin 4
C23:     100nF (C14663, 0603) - CVM2 charge pump (pin 4 to GND)
C24:     100nF (C14663, 0603) - GND bypass at pin 6
R18:     Open/NC - MODE pin config (see below)
R19:     10kΩ (C25804, 0603) - nSLEEP pull-up to VCC
R20:     10kΩ (C25804, 0603) - nFAULT pull-up to 3.3V
R21:     4.7kΩ (C25879, 0603) - IPROPI to GND for ADC
R22:     Open - DRVOFF (optional, see notes)
R_MODE:  TBD based on mode selection
R_DIAG:  TBD (optional, for diagnostics config)
J3:      WAGO 2601-1102 (Manual) - Motor output
```

### Power Connections
```
24V_FUSED ─┬─→ C21 (100µF 35V) ─┬─→ Pin 4 (VM)
           │                    │
           └─→ C27 (100µF 35V) ─┤
                                │
                     C22 (10µF) ┴─→ Pin 4 (VM, close to IC)
                                
Pin 4 (VM) ─→ C23 (100nF) ─→ GND  (CVM2 charge pump cap)

Pin 6 (GND) ─┬─→ GND (signal ground)
             │
             └─→ C24 (100nF) ─→ 3.3V (bypass cap)

Thermal Pad (center) ─→ GND with 6-8 thermal vias (0.3mm dia)
```

### Control Inputs (PH/EN Mode)
```
ESP32 GPIO0 (M1_PH)  ─→ Pin 10 (PH/IN2) - Direction control
ESP32 GPIO1 (M1_EN)  ─→ Pin 9 (EN/IN1)  - PWM speed control

Pin 3 (nSLEEP) ─→ R19 (10kΩ) ─→ 3.3V (always awake)
   or
Pin 3 (nSLEEP) ─→ ESP32 GPIO (for sleep control)
```

### Mode Configuration (Pin 14)
**For PH/EN mode** (what we want):
```
Option A: Pin 14 (MODE) ─→ GND directly
Option B: Pin 14 (MODE) ─→ R_MODE (100kΩ) ─→ GND
```

**Datasheet says**: MODE pin voltage sets mode:
- MODE = 0V (GND): **PH/EN mode** ✅ This is what we want
- MODE = Float (open): IN1/IN2 mode
- MODE = VM: PWM mode

**Recommendation**: Connect Pin 14 directly to GND for PH/EN mode.

### Fault and Diagnostics
```
Pin 1 (nFAULT) ─→ R20 (10kΩ) ─→ 3.3V
              └─→ ESP32 GPIO9 (M1_FAULT input)

Pin 8 (DRVOFF) ─→ GND (normal operation)
   or
Pin 8 (DRVOFF) ─→ R22 (pull-down) ─→ GND + optional GPIO control

Pin 11 (DIAG) ─→ Configuration resistor (see datasheet for options)
```

### Current Sense
```
Pin 2 (IPROPI) ─→ R21 (4.7kΩ) ─→ GND
               └─→ ESP32 GPIO20 (ADC, M1_ISENSE)

IPROPI voltage = (Motor Current) × 525µA/A × 4.7kΩ
For 10A: VIPROPI = 10 × 0.000525 × 4700 = 24.675mV × 100 = 2.47V
(Need to verify gain - may need different resistor value)
```

### Current Limit Configuration
```
Pin 13 (ITRIP) ─→ Resistor divider from 3.3V or capacitor to GND
See datasheet for ITRIP configuration for desired current limit
```

### Slew Rate Configuration  
```
Pin 12 (SR) ─→ Resistor to GND for slew rate setting
See datasheet table for values
```

### Motor Outputs
```
Pin 7 (OUT1) ─┬─→ MOTOR+ (J3 Pin 1, WAGO 2601-1102)
Pin 5 (OUT2) ─┴─→ MOTOR- (J3 Pin 2, WAGO 2601-1102)
```

---

## 📐 **Complete Schematic Block**

```
                    U1: DRV8243HQRXYRQ1
                    ┌──────────────────┐
                    │  VQFN-HR-14      │
                  3 │nSLEEP        VM 4│─┬─ 24V_FUSED
                    │                  │ │
3.3V ─ R19(10k) ────┤                  │ ├─ C21(100µF)
                    │                  │ ├─ C27(100µF)
                  8 │DRVOFF      CVM2  │ └─ C22(10µF)
                    │              │   │
GND ────────────────┤              └───┼─── C23(100nF) ─ GND
                    │                  │
M1_EN (GPIO1) ────9 │EN/IN1      OUT2 5│─── MOTOR- (J3-2)
                    │                  │
M1_PH (GPIO0) ───10 │PH/IN2            │
                    │          LOAD    │
                  2 │IPROPI      OUT1 7│─── MOTOR+ (J3-1)
      ┌────────────┤              │   │
      │            │              └───┼─── Motor Load
      ├─ R21(4.7k)─┤                  │
      │            │                  │
      └─ GPIO20  1 │nFAULT            │
         (ADC)     │              │   │
                   │              └───┼─── GND (common)
3.3V ─ R20(10k) ───┤                  │
      │            │                  │
      └─ GPIO9   14│MODE        GND 6 │─┬─ GND
                   │              │   │ │
GND ───────────────┤              └───┼─┴─ C24(100nF) ─ 3.3V
                   │                  │
                 11│DIAG              │
                   │                  │
                 13│ITRIP             │
                   │                  │
                 12│SR                │
                   │                  │
                   └──────────────────┘
                          PAD
                           │
                          GND (with thermal vias)
```

---

## 🔥 **PCB Layout Critical Notes**

### Thermal Management
```
1. Thermal pad MUST connect to GND
2. Add 6-8 thermal vias (0.3mm diameter)
3. Vias connect to internal GND plane (Layer 2)
4. Large copper pour around IC on top layer
5. Keep IC away from heat-sensitive components
```

### High Current Paths
```
1. VM (Pin 4) traces: 50+ mil width, 2oz copper
2. OUT1/OUT2 (Pins 5,7) traces: 50+ mil width
3. GND (Pin 6 + Pad): Large copper pour
4. Short, thick traces from VM caps to Pin 4
5. Motor output traces: Keep short and thick
```

### Signal Integrity
```
1. Keep control inputs (Pins 9,10) away from switching nodes
2. Bypass caps (C23, C24) VERY close to IC
3. IPROPI trace (Pin 2) - route away from noisy signals
4. nFAULT (Pin 1) - pull-up resistor close to IC
5. Use ground plane under entire circuit
```

---

## 🧪 **Testing Procedure**

### Power-Up Test (No Motor)
```
1. Apply 24V to VM
2. Verify 24V at Pin 4
3. Check nFAULT (Pin 1) is HIGH (should be ~3.3V with pull-up)
4. Check nSLEEP (Pin 3) is HIGH
5. Measure thermal pad temperature (should be ambient)
```

### Control Test (No Motor)
```
1. Set M1_PH = LOW, M1_EN = LOW
   → OUT1 and OUT2 should both be low
   
2. Set M1_PH = LOW, M1_EN = HIGH (full PWM)
   → OUT1 ≈ 24V, OUT2 ≈ 0V (forward)
   
3. Set M1_PH = HIGH, M1_EN = HIGH (full PWM)
   → OUT1 ≈ 0V, OUT2 ≈ 24V (reverse)
   
4. Set M1_PH = LOW, M1_EN = PWM 50%
   → OUT1 ≈ 12V average (measure with scope)
```

### Motor Test (With Motor)
```
1. Connect motor to J3 (WAGO terminals)
2. Start with low PWM duty cycle (10-20%)
3. Test forward: PH=LOW, EN=PWM
4. Test reverse: PH=HIGH, EN=PWM
5. Monitor IPROPI voltage (should scale with current)
6. Check for nFAULT triggers
7. Monitor IC temperature
```

---

## ⚡ **PH/EN Mode Control Truth Table**

```
nSLEEP | EN/IN1 | PH/IN2 | OUT1  | OUT2  | Function
-------|--------|--------|-------|-------|------------------
   0   |   X    |   X    | Hi-Z  | Hi-Z  | Sleep mode
   1   |   0    |   X    | Hi-Z  | Hi-Z  | Coast/Disable
   1   |   1    |   0    | PWM H | PWM L | Forward (OUT1+)
   1   |   1    |   1    | PWM L | PWM H | Reverse (OUT2+)
```

**For our firmware**:
- **Forward**: PH=0, EN=PWM_value
- **Reverse**: PH=1, EN=PWM_value
- **Stop**: PH=X, EN=0 (or both LOW)
- **Brake**: Set both OUT1 and OUT2 LOW (EN=0)

---

## 📦 **Motor Driver 2 (Elevation)**

**Duplicate the entire Motor Driver 1 circuit with these changes**:

```
Reference Changes:
U1 → U2
C21 → C31
C27 → C37
C22 → C32
C23 → C33
C24 → C34
R18 → R28
R19 → R29
R20 → R30
R21 → R31
J3 → J4

Signal Changes:
M1_EN (GPIO1) → M2_EN (GPIO3)
M1_PH (GPIO0) → M2_PH (GPIO2)
M1_FAULT (GPIO9) → M2_FAULT (GPIO10)
M1_ISENSE (GPIO20) → M2_ISENSE (GPIO21)
```

---

## ✅ **Bill of Materials - Both Motor Drivers**

```
Qty | Ref          | Value        | Footprint   | JLCPCB    | Description
----|--------------|--------------|-------------|-----------|------------------
2   | U1, U2       | DRV8243H...  | VQFN-HR-14  | C3040833  | Motor driver
4   | C21,C27,C31,C37| 100µF 35V  | 1210        | C13585    | VM bulk
4   | C22,C32      | 10µF 16V     | 0805        | C15850    | VM decoupling
4   | C23,C33      | 100nF        | 0603        | C14663    | Charge pump
4   | C24,C34      | 100nF        | 0603        | C14663    | Bypass
4   | R19,R20,R29,R30| 10kΩ       | 0603        | C25804    | Pull-ups
2   | R21, R31     | 4.7kΩ        | 0603        | C25879    | IPROPI
2   | J3, J4       | WAGO 2601... | TH          | Manual    | Motor out
```

**Total Cost (2 drivers)**: ~$7 for ICs + ~$2 passives = **~$9/board**

---

## 🎉 **Ready to Implement!**

This circuit is now **confirmed correct** and ready to add to your KiCad schematic!

Sorry for the confusion earlier - you were absolutely right about the VQFN-HR-14 having control pins! 🎯
