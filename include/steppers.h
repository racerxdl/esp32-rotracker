#pragma once

#include <cstdint>
#include <TMCStepper.h>
#include <AccelStepper.h>
#include "config.h"

void initSteppers();
void StepLoop();
void setEnableMotors(bool enabled);

uint16_t getElCurrent();
uint16_t getAzCurrent();
void setAzCurrentPosition(int steps);
void setElCurrentPosition(int steps);

AccelStepper &getElStepper();
AccelStepper &getAzStepper();

inline long elDegToStep(float deg) {
  deg = constrain(deg, EL_ANGLE_MIN, EL_ANGLE_MAX);
  return (long) (EL_STEPS_PER_DEG * deg);
}

inline long azDegToStep(float deg) {
  deg = constrain(deg, AZ_ANGLE_MIN, AZ_ANGLE_MAX);
  return (long) (AZ_STEPS_PER_DEG * deg);
}

inline float elStepToDeg(long steps) {
  return ((float)steps) / EL_STEPS_PER_DEG;
}

inline float azStepToDeg(long steps) {
  return ((float)steps) / AZ_STEPS_PER_DEG;
}
