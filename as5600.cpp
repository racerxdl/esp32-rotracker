#include "as5600.h"
#include "Arduino.h"

#define AS5600_ADDR 0x36
#define STATUS_ADDR 0x0B
#define AGC_ADDR 0x1A

struct __attribute__((__packed__)) statusFlags {
  uint8_t 
          : 2, 
        MD: 1,
        ML: 1,
        MH: 1;
};

struct __attribute__((__packed__)) registerData {
  statusFlags flags;
  uint16_t rawAngle;
  uint16_t angle;
  uint8_t agc;
  uint16_t magnitude;
};


union {
  uint8_t data[8];
  registerData regs;
} Registers;

// Endianess switch
void swapBytes(uint8_t *arr) {
  uint8_t tmp = arr[0];
  arr[0] = arr[1];
  arr[1] = tmp;
}

AS5600::AS5600() {}

bool AS5600::init() {
  Wire.beginTransmission(AS5600_ADDR);
  uint8_t error = Wire.endTransmission();
  if (error != 0) {
    return false;
  }

  return true;
}

uint8_t AS5600::readByte(uint8_t addr) {
  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(addr);
  Wire.endTransmission();
  Wire.requestFrom(AS5600_ADDR, 1);

  while (Wire.available() == 0);
  return Wire.read();
}

void AS5600::update() {
  uint8_t tmp;
  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(STATUS_ADDR);
  Wire.endTransmission();
  
  Wire.requestFrom(AS5600_ADDR, 5);
  for (int i = 0; i < 5; i++) {
    Registers.data[i] = Wire.read();
  }

  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(AGC_ADDR);
  Wire.endTransmission();
  
  Wire.requestFrom(AS5600_ADDR, 3);
  for (int i = 5; i < 8; i++) {
    while (!Wire.available());
    Registers.data[i] = Wire.read();
  }
  swapBytes(&Registers.data[1]); // Raw Angle
  swapBytes(&Registers.data[3]); // Angle
  swapBytes(&Registers.data[6]); // Magnitude
}

float AS5600::getAngle() {
  return ((float)(Registers.regs.angle)) * 0.087f;
}

int AS5600::isAgcOverflow() {
  if (Registers.regs.flags.ML) {
    return -1;
  }
  if (Registers.regs.flags.MH) {
    return 1;
  }

  return 0;
}

bool AS5600::hasMagnet() {
  return Registers.regs.flags.MD;  
}

AS5600 as5600;

AS5600 &getAzEncoder() {
  return as5600;
}
