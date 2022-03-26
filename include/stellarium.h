#pragma once

#include <cstdint>

void initStellarium();
void GetRADEC(float *rightAscension, float *declination);
float stellariumGetLastAscension();
float stellariumGetLastDeclination();
void stellariumDisconnectAll();
void updateStellarium();

struct __attribute__((__packed__)) StellariumPacket {
  uint16_t length;
  uint16_t type;
  int64_t time;
  uint32_t rightAscension;
  int32_t declination;
  int32_t status;
};