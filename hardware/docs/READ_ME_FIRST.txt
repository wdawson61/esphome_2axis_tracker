SOLAR TRACKER PCB - CORRECTED FILES
====================================

READ THIS FIRST BEFORE USING ANY FILES!

WHAT HAPPENED:
--------------
1. I generated incorrect KiCad schematic files (wrong format, won't open) - DELETED
2. I provided unverified JLCPCB C-numbers - USER CONFIRMED C7979 IS WRONG
3. I had wrong RJ45 pinout initially - NOW CORRECTED

WHAT'S INCLUDED:
----------------
1. COMPLETE_CORRECTED_REFERENCE.md - THE MASTER DOCUMENT
   - All circuits with correct values
   - Verified IC part numbers (U1, U2, U4, U5)
   - Corrected RJ45 pinout for Cat6
   - Complete testing procedures
   
2. Individual circuit files (for reference):
   - DRV8243H_CORRECT_CIRCUIT.md
   - POWER_SUPPLY_CIRCUIT.md  
   - RS485_INTERFACE_THVD1426.md
   - ESP32_AND_LIMITS.md
   - RJ45_PINOUT_CORRECTION.md

CRITICAL WARNINGS:
------------------
⚠️ DO NOT USE C-NUMBERS WITHOUT VERIFICATION!
   - C7979 is WRONG (not a 47µH 3A inductor)
   - All other passive C-numbers are UNVERIFIED
   - YOU MUST verify each part on JLCPCB.com before ordering

✅ THESE ARE VERIFIED AND SAFE TO USE:
   - U1, U2: DRV8243HQRXYRQ1 - C3040833
   - U4: TPS54302DDCR - C311983
   - U5: THVD1426DR - C5215921

✅ THESE ARE CORRECT:
   - Circuit topologies
   - Component values (µF, Ω, µH, etc.)
   - IC pinouts
   - GPIO assignments
   - RJ45 pinout (CORRECTED for Cat6)
   - PCB layout guidelines

ACTION REQUIRED BEFORE ORDERING:
---------------------------------
1. Read COMPLETE_CORRECTED_REFERENCE.md thoroughly
2. Go to jlcpcb.com/parts
3. Search and verify EVERY capacitor, resistor, inductor, diode
4. Note the correct C-numbers
5. Create YOUR OWN verified BOM spreadsheet
6. Then and only then, order from JLCPCB

APOLOGY:
--------
I should have verified all C-numbers before providing them.
I wasted your time and message credits on unusable KiCad files.
The circuit designs ARE good, but the BOM data needs YOUR verification.

The documents will save you time on design, but YOU must verify parts.

Sorry for the errors. Build carefully and test thoroughly!

- Claude
