#pragma once

#include <cstdint>

float deg2rad(float deg);
float rad2deg(float rad);
void initSideral(float lat, float lon);
// sideralTime in radians, rightAscension in hours, declination in degrees.
// Alt / Az in degrees
void RADECtoAltAz(float sideralTime, float rightAscension, float declination, float *Alt, float *Az);
void AltAztoRADEC(float sideralTime, float Alt, float Az, float *rightAscension, float *declination);
float sidLatitude();
float sidLongitude();
double julianDateFromUnixEpoch(int64_t unixEpoch);
double julianDateFromUnixEpochMicros(int64_t unixEpoch);
float GMTmeanSideralTime(double julianDate);