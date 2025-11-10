import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, uart
from esphome.const import (
    CONF_ID,
    CONF_TEMPERATURE,
    DEVICE_CLASS_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
    UNIT_METER_PER_SECOND_SQUARED,
    UNIT_DEGREE_PER_SECOND,
    UNIT_DEGREES,
)

DEPENDENCIES = ["uart"]
CODEOWNERS = ["@wdawson61"]

wit_motion_ns = cg.esphome_ns.namespace("wit_motion")
WitMotionComponent = wit_motion_ns.class_(
    "WitMotionComponent", cg.Component, uart.UARTDevice
)

CONF_ACCELERATION_X = "acceleration_x"
CONF_ACCELERATION_Y = "acceleration_y"
CONF_ACCELERATION_Z = "acceleration_z"
CONF_ANGULAR_VELOCITY_X = "angular_velocity_x"
CONF_ANGULAR_VELOCITY_Y = "angular_velocity_y"
CONF_ANGULAR_VELOCITY_Z = "angular_velocity_z"
CONF_ROLL = "roll"
CONF_PITCH = "pitch"
CONF_HEADING = "heading"

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(WitMotionComponent),
            cv.Optional(CONF_ACCELERATION_X): sensor.sensor_schema(
                unit_of_measurement=UNIT_METER_PER_SECOND_SQUARED,
                accuracy_decimals=3,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_ACCELERATION_Y): sensor.sensor_schema(
                unit_of_measurement=UNIT_METER_PER_SECOND_SQUARED,
                accuracy_decimals=3,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_ACCELERATION_Z): sensor.sensor_schema(
                unit_of_measurement=UNIT_METER_PER_SECOND_SQUARED,
                accuracy_decimals=3,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_ANGULAR_VELOCITY_X): sensor.sensor_schema(
                unit_of_measurement=UNIT_DEGREE_PER_SECOND,
                accuracy_decimals=2,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_ANGULAR_VELOCITY_Y): sensor.sensor_schema(
                unit_of_measurement=UNIT_DEGREE_PER_SECOND,
                accuracy_decimals=2,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_ANGULAR_VELOCITY_Z): sensor.sensor_schema(
                unit_of_measurement=UNIT_DEGREE_PER_SECOND,
                accuracy_decimals=2,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_ROLL): sensor.sensor_schema(
                unit_of_measurement=UNIT_DEGREES,
                accuracy_decimals=2,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_PITCH): sensor.sensor_schema(
                unit_of_measurement=UNIT_DEGREES,
                accuracy_decimals=2,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_HEADING): sensor.sensor_schema(
                unit_of_measurement=UNIT_DEGREES,
                accuracy_decimals=2,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_TEMPERATURE): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    if CONF_ACCELERATION_X in config:
        sens = await sensor.new_sensor(config[CONF_ACCELERATION_X])
        cg.add(var.set_acceleration_x_sensor(sens))
    if CONF_ACCELERATION_Y in config:
        sens = await sensor.new_sensor(config[CONF_ACCELERATION_Y])
        cg.add(var.set_acceleration_y_sensor(sens))
    if CONF_ACCELERATION_Z in config:
        sens = await sensor.new_sensor(config[CONF_ACCELERATION_Z])
        cg.add(var.set_acceleration_z_sensor(sens))

    if CONF_ANGULAR_VELOCITY_X in config:
        sens = await sensor.new_sensor(config[CONF_ANGULAR_VELOCITY_X])
        cg.add(var.set_angular_velocity_x_sensor(sens))
    if CONF_ANGULAR_VELOCITY_Y in config:
        sens = await sensor.new_sensor(config[CONF_ANGULAR_VELOCITY_Y])
        cg.add(var.set_angular_velocity_y_sensor(sens))
    if CONF_ANGULAR_VELOCITY_Z in config:
        sens = await sensor.new_sensor(config[CONF_ANGULAR_VELOCITY_Z])
        cg.add(var.set_angular_velocity_z_sensor(sens))

    if CONF_ROLL in config:
        sens = await sensor.new_sensor(config[CONF_ROLL])
        cg.add(var.set_roll_sensor(sens))
    if CONF_PITCH in config:
        sens = await sensor.new_sensor(config[CONF_PITCH])
        cg.add(var.set_pitch_sensor(sens))
    if CONF_HEADING in config:
        sens = await sensor.new_sensor(config[CONF_HEADING])
        cg.add(var.set_heading_sensor(sens))

    if CONF_TEMPERATURE in config:
        sens = await sensor.new_sensor(config[CONF_TEMPERATURE])
        cg.add(var.set_temperature_sensor(sens))
