#pragma once

#include <cstdint>
#include <Wire.h>

class AS5600 {
public:
  AS5600();

  bool init();
  float getAngle();
  bool hasMagnet();
  int isAgcOverflow();
  void update();
private:
  uint8_t readByte(uint8_t addr);
};

AS5600 &getAzEncoder();
