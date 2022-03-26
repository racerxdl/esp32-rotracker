#include "steppers.h"
#include "config.h"
#include "storage.h"

TMC2209Stepper azDriver(&TMC_SERIAL_PORT, R_SENSE, AZ_DRIVER_ADDRESS);
TMC2209Stepper elDriver(&TMC_SERIAL_PORT, R_SENSE, EL_DRIVER_ADDRESS);

AccelStepper elStepper = AccelStepper(elStepper.DRIVER, EL_STEP_PIN, EL_DIR_PIN);
AccelStepper azStepper = AccelStepper(azStepper.DRIVER, AZ_STEP_PIN, AZ_DIR_PIN);

void setEnableMotors(bool enabled) {
  digitalWrite(EN_PIN, !enabled);
}

AccelStepper &getElStepper() {
  return elStepper;
}
AccelStepper &getAzStepper() {
  return azStepper;
}

void initSteppers() {
  Serial.println("M;Initializing steppers serial");
  // Initializing Steppers Serial Port
  TMC_SERIAL_PORT.begin(115200);

  pinMode(EN_PIN, OUTPUT);
  setEnableMotors(false);

  Serial.printf("M;Initializing elevation axis MicroStepping: %2d - Current %04d mA\r\n", GetElevationMicrosteps(), GetElevationMotorCurrent());
  // Init Elevation
  pinMode(EL_STEP_PIN, OUTPUT);
  elDriver.begin();
  elDriver.toff(10);
  elDriver.rms_current(GetElevationMotorCurrent()); // Set motor RMS current
  elDriver.microsteps(GetElevationMicrosteps());

  elDriver.blank_time(24);
  elDriver.en_spreadCycle(true);
  elDriver.pwm_autoscale(true);
  elDriver.TCOOLTHRS(0xFFFFF); // 20bit max
  elDriver.semin(5);
  elDriver.semax(2);
  elDriver.sedn(0b11);
  elDriver.SGTHRS(4);
  elDriver.intpol(false);

  Serial.printf("M;Initializing azimuth axis MicroStepping: %2d - Current %04d mA\r\n", GetAzimuthMicrosteps(), GetAzimuthMotorCurrent());
  // Init Azimuth
  pinMode(AZ_STEP_PIN, OUTPUT);
  azDriver.begin();
  azDriver.toff(10);
  azDriver.rms_current(GetAzimuthMotorCurrent());  // Set motor RMS current
  azDriver.microsteps(GetAzimuthMicrosteps());

  azDriver.blank_time(24);
  azDriver.en_spreadCycle(true);    // Toggle spreadCycle on TMC2208/2209/2224
  azDriver.pwm_autoscale(true);      // Needed for stealthChop
  azDriver.TCOOLTHRS(0xFFFFF);       // 20bit max
  azDriver.semin(5);
  azDriver.semax(2);
  azDriver.sedn(0b11);
  azDriver.SGTHRS(4);
  azDriver.intpol(false);

  Serial.println("M;Configuring AccelStepper");
  // Configure accelstepper
  elStepper.setMaxSpeed(EL_MAX_SPEED);
  azStepper.setMaxSpeed(AZ_MAX_SPEED);
  elStepper.setAcceleration(EL_DEFAULT_ACCEL);
  azStepper.setAcceleration(AZ_DEFAULT_ACCEL);
  elStepper.setCurrentPosition(0);
  azStepper.setCurrentPosition(0);
  setEnableMotors(true);
}

void setAzCurrentPosition(int steps) {
  azStepper.setCurrentPosition(steps);
}

void setElCurrentPosition(int steps) {
  elStepper.setCurrentPosition(steps);
}

void updateSteppers() {
  elStepper.run();
  azStepper.run();
}

uint16_t getElCurrent() {
  return elDriver.cs2rms(elDriver.cs_actual());
}

uint16_t getAzCurrent() {
  return azDriver.cs2rms(azDriver.cs_actual());
}
