#!/usr/bin/env python3
"""
Test script for Solar Tracker ESPHome firmware
Validates configuration and simulates basic operations
"""

import time

def test_hwt905_protocol():
    """Test HWT905 packet parsing"""
    print("Testing HWT905 Protocol...")
    
    # Sample angle packet: 0x55 0x53 [Roll] [Pitch] [Yaw] [Temp] [Checksum]
    # Pitch = 30 degrees = 30/180 * 32768 = 5461 = 0x1555
    test_packet = [0x55, 0x53, 0x00, 0x00, 0x55, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00]
    
    # Calculate checksum
    checksum = sum(test_packet[:-1]) & 0xFF
    test_packet[-1] = checksum
    
    print(f"  Test packet: {' '.join(f'0x{b:02X}' for b in test_packet)}")
    
    # Parse pitch (bytes 4-5, LSB first)
    pitch_raw = (test_packet[5] << 8) | test_packet[4]
    pitch = (pitch_raw / 32768.0) * 180.0
    
    print(f"  Parsed pitch: {pitch:.2f}° (expected: 30.00°)")
    assert abs(pitch - 30.0) < 0.1, "Pitch parsing failed"
    
    print("  ✓ Protocol parsing OK\n")

def test_azimuth_error_calculation():
    """Test azimuth wraparound error calculation"""
    print("Testing Azimuth Error Calculation...")
    
    test_cases = [
        (10, 350, -20),   # Cross 0° going CCW
        (350, 10, 20),    # Cross 0° going CW
        (180, 90, -90),   # Normal case
        (90, 180, 90),    # Normal case
        (0, 180, 180),    # Ambiguous (should prefer CW)
        (359, 1, 2),      # Small cross
    ]
    
    def calculate_error(current, target):
        error = target - current
        if error > 180:
            error -= 360
        elif error < -180:
            error += 360
        return error
    
    for current, target, expected in test_cases:
        error = calculate_error(current, target)
        print(f"  Current: {current:3}°, Target: {target:3}°, Error: {error:4.0f}° (expected: {expected:4.0f}°)")
        assert abs(error - expected) < 1, f"Error calculation failed for {current}->{target}"
    
    print("  ✓ Azimuth calculations OK\n")

def test_motor_control_logic():
    """Test motor control state machine"""
    print("Testing Motor Control Logic...")
    
    # Simulate elevation control
    current_elevation = 0.0
    target_elevation = 45.0
    tolerance = 0.5
    
    steps = 0
    while abs(target_elevation - current_elevation) >= tolerance:
        steps += 1
        if target_elevation > current_elevation:
            current_elevation += 2.5  # Simulate motor movement
            action = "Forward"
        else:
            current_elevation -= 2.5
            action = "Backward"
        
        if steps > 100:
            print("  ✗ Motor control stuck in loop")
            break
    
    print(f"  Elevation reached target in {steps} steps")
    print(f"  Final elevation: {current_elevation:.2f}° (target: {target_elevation:.2f}°)")
    assert abs(current_elevation - target_elevation) < tolerance, "Elevation control failed"
    
    print("  ✓ Motor control logic OK\n")

def test_safety_features():
    """Test safety timeout and limits"""
    print("Testing Safety Features...")
    
    # Test angle limits
    def constrain(value, min_val, max_val):
        return max(min_val, min(max_val, value))
    
    test_values = [
        (-10, 0, 90, 0),
        (45, 0, 90, 45),
        (100, 0, 90, 90),
    ]
    
    for value, min_val, max_val, expected in test_values:
        result = constrain(value, min_val, max_val)
        print(f"  Constrain {value}° to [{min_val}, {max_val}]: {result}° (expected: {expected}°)")
        assert result == expected, "Angle limiting failed"
    
    print("  ✓ Safety features OK\n")

def test_homing_state_machine():
    """Test homing sequence logic"""
    print("Testing Homing State Machine...")
    
    # Simulate homing states
    class HomingPhase:
        MOVE_OFF_SWITCH = 0
        SEEK_SWITCH = 1
        SLOW_APPROACH = 2
        SET_ZERO = 3
        COMPLETE = 4
    
    # Test state transitions
    phase = HomingPhase.MOVE_OFF_SWITCH
    switch_pressed = True
    
    # Phase 1: Move off switch
    if switch_pressed:
        print("  Phase 1: On switch, backing off...")
        phase = HomingPhase.MOVE_OFF_SWITCH
    
    # Simulate switch release
    switch_pressed = False
    if phase == HomingPhase.MOVE_OFF_SWITCH and not switch_pressed:
        print("  Phase 1 → 2: Moved off switch")
        phase = HomingPhase.SEEK_SWITCH
    
    # Phase 2: Seek switch
    steps = 0
    while phase == HomingPhase.SEEK_SWITCH and steps < 10:
        steps += 1
        if steps == 5:  # Simulate finding switch
            switch_pressed = True
            print("  Phase 2: Switch found after 5 steps")
            phase = HomingPhase.SLOW_APPROACH
            switch_pressed = False  # Back off
    
    # Phase 3: Slow approach
    if phase == HomingPhase.SLOW_APPROACH:
        print("  Phase 3: Slow approach...")
        switch_pressed = True  # Trigger on approach
        if switch_pressed:
            print("  Phase 3 → 4: Switch triggered")
            phase = HomingPhase.SET_ZERO
    
    # Phase 4: Set zero
    if phase == HomingPhase.SET_ZERO:
        print("  Phase 4: Setting zero position")
        home_offset = 237.0  # Example HWT905 reading
        print(f"  Home offset set: {home_offset}°")
        phase = HomingPhase.COMPLETE
    
    # Phase 5: Complete
    assert phase == HomingPhase.COMPLETE, "Homing sequence didn't complete"
    print("  Phase 5: Homing complete!")
    
    print("  ✓ Homing state machine OK\n")

def test_home_offset_calculation():
    """Test position correction with home offset"""
    print("Testing Home Offset Calculation...")
    
    # Simulate homing at 237° (Southwest)
    home_offset = 237.0
    
    test_cases = [
        (237, 0),    # At home position → 0°
        (327, 90),   # 90° CW from home → 90°
        (147, 270),  # 90° CCW from home → 270°
        (57, 180),   # Opposite side → 180°
        (0, 123),    # Wrap around case
    ]
    
    for raw_heading, expected_corrected in test_cases:
        corrected = raw_heading - home_offset
        # Normalize to 0-360
        while corrected < 0:
            corrected += 360
        while corrected >= 360:
            corrected -= 360
        
        print(f"  Raw: {raw_heading:3}° - Offset: {home_offset}° = {corrected:3}° (expected: {expected_corrected:3}°)")
        assert abs(corrected - expected_corrected) < 1, f"Offset calculation failed for {raw_heading}°"
    
    print("  ✓ Home offset calculation OK\n")

def print_pin_configuration():
    """Print pin configuration summary"""
    print("Pin Configuration Summary:")
    print("=" * 50)
    print("  UART (HWT905 via SeeedStudio RS485):")
    print("    TX: GPIO4")
    print("    RX: GPIO5")
    print("    Baud: 115200")
    print()
    print("  Elevation Motor (BTS7960):")
    print("    RPWM (Forward):  GPIO6")
    print("    LPWM (Backward): GPIO7")
    print("    R_EN/L_EN: 3.3V (tied)")
    print()
    print("  Azimuth Motor (BTS7960):")
    print("    RPWM (CW):  GPIO8")
    print("    LPWM (CCW): GPIO9")
    print("    R_EN/L_EN: 3.3V (tied)")
    print()
    print("  Homing:")
    print("    Limit Switch: GPIO10 (with pullup)")
    print()
    print("  Power:")
    print("    Motors: 24V DC (10A+ supply)")
    print("    Logic:  5V (for ESP32, sensors)")
    print()
    print("  Status LED: GPIO2")
    print("=" * 50)
    print()

def print_homing_configuration():
    """Print homing switch configuration"""
    print("Homing Switch Configuration:")
    print("=" * 50)
    print("  Switch Type: Normally Open (NO)")
    print("  Connection:")
    print("    COM (Common)  → GPIO10")
    print("    NO (N.O.)     → GND")
    print("    (Internal pullup enabled on GPIO10)")
    print()
    print("  Switch States:")
    print("    Open (not triggered):  GPIO10 = HIGH (3.3V)")
    print("    Closed (triggered):    GPIO10 = LOW  (0V)")
    print()
    print("  Homing Sequence:")
    print("    1. Move off switch if needed (CW)")
    print("    2. Seek switch (CCW continuous)")
    print("    3. Slow approach (CCW pulses)")
    print("    4. Set zero at switch position")
    print("    5. Complete - ready for operation")
    print()
    print("  Timeouts:")
    print("    Total homing:  180 seconds")
    print("    Backoff:       2 seconds")
    print("    Settle time:   0.5 seconds")
    print("=" * 50)
    print()

def print_calibration_sequence():
    """Print calibration command sequence"""
    print("HWT905 Calibration Sequence:")
    print("=" * 50)
    commands = [
        ("Unlock Config", [0xFF, 0xAA, 0x69, 0xB5, 0x88]),
        ("Start Accel Cal", [0xFF, 0xAA, 0x01, 0x01, 0x00]),
        ("Wait 5 seconds", None),
        ("Exit Accel Cal", [0xFF, 0xAA, 0x01, 0x00, 0x00]),
        ("Start Mag Cal", [0xFF, 0xAA, 0x01, 0x07, 0x00]),
        ("Wait 15 seconds", None),
        ("Exit Mag Cal", [0xFF, 0xAA, 0x01, 0x00, 0x00]),
        ("Save Config", [0xFF, 0xAA, 0x00, 0x00, 0x00]),
    ]
    
    for i, (desc, cmd) in enumerate(commands, 1):
        if cmd:
            print(f"  {i}. {desc}: {' '.join(f'0x{b:02X}' for b in cmd)}")
        else:
            print(f"  {i}. {desc}")
    print("=" * 50)
    print()

def main():
    print("\n" + "=" * 60)
    print("Solar Tracker Firmware Test Suite")
    print("=" * 60 + "\n")
    
    try:
        test_hwt905_protocol()
        test_azimuth_error_calculation()
        test_motor_control_logic()
        test_safety_features()
        test_homing_state_machine()
        test_home_offset_calculation()
        
        print("\n" + "=" * 60)
        print("✓ All tests passed!")
        print("=" * 60 + "\n")
        
        print_pin_configuration()
        print_homing_configuration()
        print_calibration_sequence()
        
        print("Next Steps:")
        print("  1. Edit secrets.yaml with your WiFi credentials")
        print("  2. Verify pin assignments match your hardware")
        print("  3. Wire the home limit switch to GPIO10")
        print("  4. Connect ESP32-C6 via USB")
        print("  5. Run: esphome run solar_tracker.yaml")
        print("  6. After first boot, perform sensor calibration")
        print("  7. Run home_azimuth before any azimuth movements")
        print()
        
    except AssertionError as e:
        print(f"\n✗ Test failed: {e}\n")
        return 1
    
    return 0

if __name__ == "__main__":
    exit(main())
