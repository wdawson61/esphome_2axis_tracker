# Azimuth Homing System Documentation

## Overview

The azimuth homing system establishes a known zero-reference position for the tracker's rotation axis using a limit switch. This is essential for accurate positioning since the HWT905 magnetometer readings can drift and don't provide an absolute position reference.

## Why Homing is Necessary

1. **Absolute Position Reference**: The HWT905 provides heading relative to magnetic north, which can vary with magnetic interference and requires calibration
2. **Power Loss Recovery**: After power cycles, the system needs to re-establish the tracker's physical position
3. **Drift Compensation**: Over time, integrated position errors can accumulate - homing provides a reset point
4. **Consistent Zero Position**: Defines a repeatable mechanical reference (e.g., "facing south")

## Hardware Requirements

### Limit Switch
- **Type**: Normally Open (NO) mechanical switch or proximity sensor
- **Rating**: Low current (signal level, 3.3V logic)
- **Mounting**: At the desired home/zero position (typically North or South facing)
- **Actuation**: Triggered by mechanical contact or proximity when at home position

### Motor Controller Notes (BTS7960)
- **Enable Pins**: Ensure R_EN and L_EN are tied to 3.3V for operation
- **Power Supply**: 24V, 10A+ recommended for dual motor operation
- **Heat Management**: BTS7960 modules have heat sinks - ensure adequate ventilation
- **Protection**: Built-in over-current, over-temperature, short-circuit protection
- **During Homing**: Motor runs continuously (not PWM) at full speed

### Recommended Switch Types
1. **Mechanical Limit Switch**: Standard SPDT switch, use NO (Normally Open) terminal
2. **Magnetic Reed Switch**: Non-contact, triggered by magnet on rotating assembly
3. **Optical Switch**: IR or photoelectric, non-contact
4. **Hall Effect Sensor**: Solid-state, non-contact, triggered by magnet

### Wiring

```
Limit Switch        ESP32-C6
----------------    -----------
Common (COM)   ---> GPIO10
Normally Open  ---> GND
                    (Internal pullup enabled)

When switch closes: GPIO10 → LOW (triggered)
When switch open:   GPIO10 → HIGH (via pullup)
```

**Important Notes:**
- Do NOT connect VCC to the switch - GPIO10 uses internal pullup
- Use twisted pair or shielded cable if switch is >30cm from ESP32
- Add 0.1µF capacitor across switch terminals if bounce is excessive

## Homing Sequence

### Phase 1: Move Off Switch (if needed)
If the system starts with the switch already triggered:
1. Motor rotates clockwise (away from switch)
2. Continues until switch releases
3. Runs for up to 2 seconds
4. Stops and waits 200ms for settling

### Phase 2: Seek Switch
Fast search for the home position:
1. Motor rotates counter-clockwise continuously
2. Monitors switch state
3. When switch triggers → stop immediately
4. Wait 200ms for settling
5. Back off 0.5 seconds clockwise

### Phase 3: Slow Approach
Precision positioning:
1. Motor pulses CCW in 100ms bursts
2. Pauses 100ms between pulses
3. Checks switch after each pulse
4. When switch triggers → stop immediately

### Phase 4: Set Zero
Establish reference position:
1. Wait 500ms for mechanical settling
2. Read current HWT905 heading
3. Store as `azimuth_home_offset_`
4. Mark system as homed (`azimuth_homed_ = true`)

### Phase 5: Complete
System ready for normal operation.

## State Machine Diagram

```
     START
       ↓
   [On Switch?] ──No──→ [SEEK_SWITCH]
       ↓                      ↓
      Yes               [Found Switch?]
       ↓                      ↓
[MOVE_OFF_SWITCH]            Yes
       ↓                      ↓
  [Off Switch?]         [SLOW_APPROACH]
       ↓                      ↓
      Yes               [Switch Triggered?]
       ↓                      ↓
   [SEEK_SWITCH]             Yes
       ↓                      ↓
  [Found Switch?]         [SET_ZERO]
       ↓                      ↓
      Yes                 [Wait 500ms]
       ↓                      ↓
 [SLOW_APPROACH]         [Store Offset]
                              ↓
                          [COMPLETE]
                              ↓
                          System Ready
```

## Usage

### Initial Setup

1. **Position the limit switch** at your desired zero/home position:
   - Recommended: South-facing (180° on compass)
   - Alternative: North-facing (0° on compass)
   - Must be easily reached by CCW rotation from any position

2. **Wire the switch** to GPIO10 and GND as shown above

3. **Power on the system** and connect to Home Assistant

4. **Run homing** before first use:
   ```yaml
   service: esphome.solar_tracker_home_azimuth
   ```

### Regular Operation

**Run homing:**
- After every power cycle
- After emergency stop
- If position accuracy degrades
- Before resuming tracking after long idle periods
- At least once per day for best accuracy

**Automatic homing** can be configured in Home Assistant:
```yaml
automation:
  - alias: "Home on Startup"
    trigger:
      - platform: homeassistant
        event: start
    action:
      - delay: 30  # Wait for system stabilization
      - service: esphome.solar_tracker_home_azimuth
```

### Service Call

```yaml
service: esphome.solar_tracker_home_azimuth
```

**Expected Duration**: 1-3 minutes depending on starting position

**Returns**: 
- Success: "Azimuth homed successfully - zero position set"
- Failure: Timeout after 3 minutes

## Position Calculation

After homing, all azimuth angles are calculated relative to the home position:

```cpp
corrected_heading = raw_hwt905_heading - azimuth_home_offset_

// Normalize to 0-360°
if (corrected_heading < 0) corrected_heading += 360;
if (corrected_heading >= 360) corrected_heading -= 360;
```

**Example:**
- Home position HWT905 reads: 237°
- Home offset stored: 237°
- Later, HWT905 reads: 327°
- Corrected heading: 327° - 237° = 90° (East-facing, as expected)

## Safety Features

### Timeout Protection
- **Total homing timeout**: 3 minutes (HOMING_TIMEOUT)
- If switch not found, stops and logs error
- Prevents infinite rotation

### Switch Debouncing
- Built into ESPHome binary sensor: 50ms delayed_on
- Prevents false triggers from contact bounce

### Emergency Stop
- Homing immediately aborts if emergency stop active
- Cannot start homing while emergency stop engaged

### Phase Timeouts
- Move off switch: 2 seconds maximum (HOMING_BACKOFF_TIME)
- Prevents getting stuck if switch is faulty

## Troubleshooting

### Homing Never Completes

**Possible Causes:**
1. **Switch not wired correctly**
   - Verify COM → GPIO10, NO → GND
   - Test with multimeter: should show open circuit when not pressed

2. **Switch positioned wrong**
   - Must be reachable by CCW rotation
   - Adjust physical position of switch

3. **Switch not triggering**
   - Check mechanical alignment
   - Verify switch actually closes when contacted
   - Replace if faulty

4. **Motor direction reversed**
   - CCW should move toward switch during seeking
   - Swap GPIO8 and GPIO9 if needed

### Homing Completes but Position Wrong

**Possible Causes:**
1. **Switch triggers at wrong position**
   - Reposition switch mechanically
   - Re-run homing after adjustment

2. **Mechanical backlash**
   - Slow approach phase minimizes this
   - Increase HOMING_SETTLE_TIME if needed

3. **HWT905 not calibrated**
   - Run sensor calibration first
   - Reduces heading drift

### Switch Triggers Intermittently

**Possible Causes:**
1. **Mechanical bounce**
   - Increase debounce time in YAML (50ms default)
   - Add hardware RC filter (resistor + capacitor)

2. **Poor electrical connection**
   - Check wire connections
   - Re-crimp or solder connections

3. **EMI from motors**
   - Route switch wires away from motor cables
   - Use shielded cable
   - Add 0.1µF capacitor across switch

## Configuration Parameters

Located in `solar_tracker.h`:

```cpp
const unsigned long HOMING_TIMEOUT = 180000;          // 3 minutes total
const unsigned long HOMING_BACKOFF_TIME = 2000;       // 2 seconds max backoff
const unsigned long HOMING_SLOW_APPROACH_TIME = 100;  // 100ms pulses
const unsigned long HOMING_SETTLE_TIME = 500;         // 500ms settling
```

**Tuning Guidance:**
- Increase HOMING_TIMEOUT if your tracker is very large/slow
- Decrease HOMING_SLOW_APPROACH_TIME for finer positioning (slower)
- Increase HOMING_SETTLE_TIME if mechanical vibrations are high
- Increase HOMING_BACKOFF_TIME if switch area is hard to exit

## Advanced Topics

### Multiple Home Positions

For systems with multiple reference points:
1. Wire additional switches to other GPIOs
2. Create separate homing routines for each
3. Use different offset values for different orientations

### Soft Homing (No Switch)

If limit switch is not available:
1. Manually position tracker at known orientation
2. Read current HWT905 heading
3. Manually set `azimuth_home_offset_` via service call
4. Mark as homed

**Not recommended** - loses automatic recovery benefits.

### Homing During Operation

Periodic re-homing can improve accuracy:
```yaml
automation:
  - alias: "Daily Re-home"
    trigger:
      - platform: time
        at: "02:00:00"  # 2 AM
    action:
      - service: esphome.solar_tracker_home_azimuth
```

Performs homing during low sun periods to maintain accuracy.

## Best Practices

1. **Mount switch securely** - any flex causes position errors
2. **Protect from weather** - use waterproof switches outdoors
3. **Test regularly** - verify switch operation monthly
4. **Document position** - note physical orientation of home position
5. **Backup configuration** - record home offset value for reference
6. **Monitor logs** - check homing completes successfully
7. **Use consistent direction** - always home CCW for repeatability

## Integration with Sun Tracking

After homing, the corrected heading allows accurate sun tracking:

```yaml
automation:
  - alias: "Track Sun"
    trigger:
      - platform: time_pattern
        minutes: "/5"
    condition:
      - condition: template
        value_template: "{{ is_state('binary_sensor.azimuth_homed', 'on') }}"
    action:
      - service: esphome.solar_tracker_set_azimuth
        data:
          angle: "{{ state_attr('sun.sun', 'azimuth') }}"
```

The condition ensures tracking only occurs after successful homing.

## Maintenance

**Weekly:**
- Verify switch triggers correctly
- Check for mechanical wear

**Monthly:**
- Clean switch contacts if mechanical
- Verify mounting is secure
- Test full homing sequence

**Annually:**
- Replace mechanical switches (preventive)
- Verify positioning accuracy
- Re-calibrate HWT905 sensor

## Summary

The homing system provides:
- ✅ Absolute position reference independent of magnetometer drift
- ✅ Automatic recovery after power loss
- ✅ Repeatable zero position for accurate sun tracking
- ✅ Multi-phase approach for reliable switch detection
- ✅ Built-in safety with timeouts and error handling
- ✅ Simple limit switch hardware requirement

**Critical for accurate long-term operation!**
