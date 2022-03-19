#pragma once

#define TMC_SERIAL_PORT Serial2   // TMC2208/TMC2224 HardwareSerial port
#define EL_DRIVER_ADDRESS 0b11    // TMC2209 Driver address according to MS1 and MS2
#define AZ_DRIVER_ADDRESS 0b10    // TMC2209 Driver address according to MS1 and MS2

// Ohms
#define R_SENSE 0.11f 

#define EL_STEP_PIN      33 // Step
#define EL_DIR_PIN       32 // Direction
#define AZ_STEP_PIN      26 // Step
#define AZ_DIR_PIN       25 // Direction
#define EN_PIN           27 // Enable

#define STALL_VALUE     100 // [0..255]

// mA
#define EL_RMS_CURRENT 1800
#define AZ_RMS_CURRENT 1800

// Axis Config
#define EL_STEPS_PER_DEG 344.00f
#define AZ_STEPS_PER_DEG 173.55f
#define EL_MICROSTEP 2
#define AZ_MICROSTEP 2

// Steps per second
#define EL_MAX_SPEED 1200
#define AZ_MAX_SPEED 1200

#define EL_DEFAULT_ACCEL 1000
#define AZ_DEFAULT_ACCEL 1000

// Max angle in degree
#define EL_ANGLE_MIN -88
#define EL_ANGLE_MAX  88

#define AZ_ANGLE_MIN -360
#define AZ_ANGLE_MAX  360
