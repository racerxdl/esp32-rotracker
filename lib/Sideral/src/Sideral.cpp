#include "Sideral.h"
#include <math.h>
#include <time.h>
#include <cstdint>

#ifndef PI
# define PI 3.14159265358979f
#endif

#define J2000_OFFSET 2451545.0
#define JULIAN_CENTURY 36525.0

float sinLat;
float cosLat;
float sinLon;
float cosLon;
float sidLat, sidLon;

float deg2rad(float deg) {
  return (deg * PI) / 180;
}

float rad2deg(float rad) {
  return (rad * 180) / PI;
}

void initSideral(float lat, float lon) {
  sinLat = sin(deg2rad(lat));
  sinLon = sin(deg2rad(lon));
  cosLat = cos(deg2rad(lat));
  cosLon = cos(deg2rad(lon));
  sidLat = lat;
  sidLon = lon;
}

float sidLatitude() {
  return sidLat;
}

float sidLongitude() {
  return sidLon;
}

void RADECtoAltAz(float sideralTime, float rightAscension, float declination, float *Alt, float *Az) {
  float HA = sideralTime - deg2rad(rightAscension * 15);
  HA = fmod(HA, PI*2);
  if (HA < 0) {
    HA += PI*2;
  }
  if (HA > PI) {
    HA = HA - PI * 2;
  }
  float dec = deg2rad(declination);

  float sinDec = sin(dec);
  float cosDec = cos(dec);
  float cosHA = cos(HA);
  float sinHA = sin(HA);

  *Az = atan2(sinHA, cosHA * sinLat - tan(dec) * cosLat);
  *Alt = asin(sinLat * sinDec + cosLat * cosDec * cosHA);
  *Az -= PI;
  if (*Az < 0) {
    *Az += 2 * PI;
  }

  *Az = rad2deg(*Az);
  *Alt = rad2deg(*Alt);
}

void AltAztoRADEC(float sideralTime, float Alt, float Az, float *rightAscension, float *declination) {
  Alt = deg2rad(Alt);
  Az = deg2rad(Az);

  if (Alt > PI / 2) {
    Alt = PI - Alt;
    Az += PI;
  }
  if (Alt < -PI / 2) {
    Alt = - PI - Alt;
    Az -= PI;
  }

  float sinEl = sin(Alt);
  float cosEl = cos(Alt);
  float cosAz = cos(Az);
  float sinDec = cosAz * cosEl * cosLat + sinEl * sinLat;
  float dec = asin(sinDec);
  float cosDec = cos(dec);
  float HA;

  if (cosLat < 0.00001) { // If in the poles, make some rought aproximation
    HA = Az + PI;
  } else {
    float v = (sinEl - sinLat * sinDec) / (cosLat * cosDec);
    HA = PI;
    if (sin(Az) > 0) {
      HA -= acos(-v);
    } else {
      HA += acos(-v);
    }
  }
  HA = rad2deg(HA + sideralTime);
  *rightAscension = HA / 15;
  *declination = rad2deg(dec);
}

double julianDateFromUnixEpoch(int64_t unixEpoch) {
  // Not valid for dates before 15 October 1582
  return (unixEpoch / 86400.0) + 2440587.5;
}

double julianDateFromUnixEpochMicros(int64_t unixEpoch) {
  // Not valid for dates before 15 October 1582
  return (unixEpoch / 86400000000.0) + 2440587.5;
}

float earthRotationAngle(double julianDate) {
  // IERS Technical Note No. 32
  float t = julianDate - J2000_OFFSET;
  float f = fmod(julianDate, 1);

  float theta = 2 * PI * (f + 0.7790572732640 + 0.00273781191135448 * t); // Equation #14
  theta = fmod(theta, 2 * PI);
  if (theta < 0) {
    theta += 2 * PI;
  }
  return theta;
}

float GMTmeanSideralTime(double julianDate) {
  //"Expressions for IAU 2000 precession quantities" N. Capitaine1,P.T.Wallace2, and J. Chapront
  // https://www.researchgate.net/publication/228853763_Expressions_for_IAU_2000_precession_quantities
  float t = (julianDate - J2000_OFFSET) / JULIAN_CENTURY;
  float t2 = t * t;
  float t3 = t2 * t;
  float t4 = t2 * t2;
  float t5 = t4 * t;
  float era = earthRotationAngle(julianDate);

  // Equation #42
  float gmst = era + (
    0.014506            +
    4612.156534   * t   +
    1.3915817     * t2  -
    0.00000044    * t3  -
    0.000029956   * t4  -
    0.0000000368  * t5
  ) / 60.0 / 60.0*PI / 180.0;

  gmst = fmod(gmst, 2 * PI);
  if (gmst < 0) {
    gmst += 2 * PI;
  }

  return gmst;
}
