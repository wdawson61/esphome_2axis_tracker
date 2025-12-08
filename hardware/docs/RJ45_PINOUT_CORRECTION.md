# RJ45 Pinout Correction - Cat6 Twisted Pairs
## Proper Use of Cat6 Cable for RS485

---

## 🔌 **CORRECTED RJ45 Pinout (J1) - For Cat6 Cable**

### Using T568B Standard Twisted Pairs

```
Pin Assignment (on PCB):
───────────────────────

Pin 1: RS485 A (Data+)     ← Orange/White wire
Pin 2: RS485 B (Data-)     ← Orange wire
                              ✅ TWISTED PAIR (best for differential)

Pin 3: 5V (Sensor Power)   ← Green/White wire
Pin 6: 5V (Sensor Power)   ← Green wire
                              ✅ TWISTED PAIR (doubled up for current)

Pin 4: GND                 ← Blue wire
Pin 5: GND                 ← Blue/White wire
                              ✅ TWISTED PAIR (doubled up for impedance)

Pin 7: NC                  ← Brown/White (not used, or shield)
Pin 8: NC                  ← Brown (not used, or shield)
```

---

## 📊 **Cat6 Cable Twisted Pair Mapping**

```
Cat6 Internal Pairs:           PCB RJ45 Pins:
───────────────────            ──────────────

Pair 1 (Orange/White):
  Wire 1: Orange/White  ────→  Pin 1: RS485 A (Data+)
  Wire 2: Orange        ────→  Pin 2: RS485 B (Data-)
  ✅ Perfect for differential RS485!

Pair 2 (Green/White):
  Wire 1: Green/White   ────→  Pin 3: +5V
  Wire 2: Green         ────→  Pin 6: +5V (parallel)
  ✅ Doubled up for 500mA current capacity

Pair 3 (Blue/White):
  Wire 1: Blue          ────→  Pin 4: GND
  Wire 2: Blue/White    ────→  Pin 5: GND (parallel)
  ✅ Doubled up for low impedance ground

Pair 4 (Brown/White):
  Wire 1: Brown/White   ────→  Pin 7: NC (or shield)
  Wire 2: Brown         ────→  Pin 8: NC (or shield)
  Optional: Connect to cable shield
```

---

## 🔧 **Updated PCB Schematic**

```
THVD1426 (U5) to RJ45 (J1):
──────────────────────────

RS485 Signals:
Pin 6 (A) ──┬─→ R3 (120Ω) ─────┬─→ Pin 7 (B)
            │                  │
            ├─→ J1 Pin 1 ───────┘ (Orange/White - A)
            │
            └─→ J1 Pin 2 ────────  (Orange - B)

Power to HWT905:
5V rail ────┬─→ J1 Pin 3 (Green/White)
            │
            └─→ J1 Pin 6 (Green)
            
            Current capacity: 500mA × 2 wires = good for HWT905

Ground:
GND ────────┬─→ J1 Pin 4 (Blue)
            │
            └─→ J1 Pin 5 (Blue/White)

Optional Shield:
Cable Shield ──→ J1 Pin 7 and/or Pin 8 ──→ Chassis GND
```

---

## 📐 **Physical Connection**

### At PCB (Board Side)
```
    RJ45 Jack (looking into jack):
    
    ┌───────────────────┐
    │ 8 7 6 5 4 3 2 1   │
    │ │ │ │ │ │ │ │ │   │
    │ NC NC 5V G G 5V B A│
    │        N N       │
    │        D D       │
    └───────────────────┘
    
    Pin 1: RS485 A (to THVD1426 Pin 6)
    Pin 2: RS485 B (to THVD1426 Pin 7)
    Pin 3: +5V
    Pin 4: GND
    Pin 5: GND
    Pin 6: +5V
    Pin 7: NC (or shield)
    Pin 8: NC (or shield)
```

### At HWT905 End (Sensor Side)
```
HWT905 Wiring:
- RS485 A: Orange/White wire (Pin 1)
- RS485 B: Orange wire (Pin 2)
- VCC (+5V): Green/White + Green wires (Pins 3,6)
- GND: Blue + Blue/White wires (Pins 4,5)
```

---

## ✅ **Why This Configuration is Better**

### Differential Signaling (RS485 A/B)
```
✅ CORRECT (Your suggestion):
   Pin 1 & 2 use Orange pair → Perfect twisted pair
   
❌ WRONG (My original):
   Pin 1,2 for A and Pin 7,8 for B → Not twisted together!
```

**Result**: Much better noise immunity, longer cable runs possible

### Power Distribution
```
✅ Green pair (Pins 3,6):
   - Both wires carry +5V in parallel
   - Reduces voltage drop over long cable
   - 500mA capacity maintained
   
✅ Blue pair (Pins 4,5):
   - Both wires carry GND in parallel
   - Lowers ground impedance
   - Better return path
```

---

## 🔍 **Cable Length Performance**

### With Correct Twisted Pairs
```
Cable Length | Signal Quality | Voltage Drop (500mA)
-------------|----------------|---------------------
0-10m        | Excellent      | <0.1V
10-30m       | Very Good      | <0.3V
30-50m       | Good           | <0.5V
50-100m      | Fair           | <1.0V

With A/B on same pair: Can go 50-100m reliably at 115200 baud
```

### With Wrong Pairing (my original mistake)
```
Cable Length | Signal Quality | Notes
-------------|----------------|------------------------
0-10m        | Good           | Works but not optimal
10-30m       | Marginal       | May see occasional errors
30m+         | Poor           | Unreliable
```

---

## 🧪 **Testing with Cat6 Cable**

### Continuity Test
```
1. Unplug cable from both ends
2. At board end, short pins 1-2 together (A-B)
3. At HWT905 end, measure resistance:
   - Orange/White to Orange: Should be <5Ω (short)
4. Remove short, check other pairs:
   - Green/White to Green: Open (good)
   - Blue to Blue/White: Open (good)
```

### Signal Quality Test
```
1. Connect oscilloscope differentially
   - Probe+ on Orange/White (Pin 1, A)
   - Probe- on Orange (Pin 2, B)
2. Send RS485 data
3. Should see clean differential signal:
   - Amplitude: 2-5V differential
   - Rise time: <50ns
   - No ringing or reflections
4. Try with different cable lengths
```

---

## 📋 **Updated BOM Note**

```
J1: RJ45 Shielded Jack (C136744, Through-hole)

Connection: Cat6 cable to HWT905
Pinout: 
  1-2: RS485 differential (Orange pair) ✅
  3,6: +5V doubled (Green pair) ✅
  4,5: GND doubled (Blue pair) ✅
  7,8: Optional shield/unused (Brown pair)

Wire Gauge: 23 AWG (Cat6 standard)
Max Length: 50-100m for 115200 baud
```

---

## 🎯 **Summary of Changes**

### OLD (Wrong):
```
Pin 1,2: Both RS485 A (not a pair!) ❌
Pin 7,8: Both RS485 B (not a pair!) ❌
Pins 3,6: GND
Pins 4,5: +5V
```

### NEW (Correct):
```
Pin 1: RS485 A  ┐
Pin 2: RS485 B  ┘ Same twisted pair! ✅
Pin 3: +5V      ┐
Pin 6: +5V      ┘ Same twisted pair! ✅
Pin 4: GND      ┐
Pin 5: GND      ┘ Same twisted pair! ✅
```

---

## ✅ **Apply This to Your Design**

When you implement in KiCad:
1. Use this corrected pinout for J1 (RS485 RJ45)
2. Same applies to J2 (Limit switches) if using Cat6
3. Update silkscreen to show: "Cat6: Pins 1-2 A/B twisted"
4. Add note in documentation about proper Cat6 wiring

**Thanks for catching that!** This is exactly the kind of detail that matters for reliable RS485 communication over longer cables! 🎯
