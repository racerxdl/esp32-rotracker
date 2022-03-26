#include <unity.h>
#include <iostream>
#include <cstdlib>
#include "Sideral.h"
#include <iomanip>


// sidereal calculation constants
#define dc 0.0657098244
#define tc 1.00273791
#define gc 6.648605
#define g2000 6.5988098
#define lc 0.0497958000000001
#define nc -0.0159140999999998
#define siderealday 23.9344699

float LST_f;
struct tm LST_struct; // Local Sideral Time

#ifndef PI
# define PI 3.14159265358979f
#endif

void updateSideralClock() {
  time_t rawtime;
  time ( &rawtime );
  double julianDate = julianDateFromUnixEpoch((int64_t)rawtime);
  LST_f = GMTmeanSideralTime(julianDate);
  LST_f = fmod(LST_f + deg2rad(sidLongitude()), 2 * PI);
}

void test_sideral(void) {
  updateSideralClock();
  float sideralTime = LST_f;
  std::cout << "Sideral Time: " << sideralTime << "h" << std::endl;

  float ra, dec;
  float alt, az;
  float raO, decO;

  ra = 14.658055556;
  dec = -60.814361111;

  RADECtoAltAz(sideralTime, ra, dec, &alt, &az);

  AltAztoRADEC(sideralTime, alt, az, &raO, &decO);

  std::cout << "Alt: " << alt << std::endl;
  std::cout << "Az: " << az << std::endl;
  std::cout << "Ra: " << raO << std::endl;
  std::cout << "Dec: " << decO << std::endl;
}

void process() {
  initSideral( -23.630000, -46.631944);
  UNITY_BEGIN();
  RUN_TEST(test_sideral);
  UNITY_END();
}

#ifdef ARDUINO

#include <Arduino.h>
void setup() {
    delay(2000);

    process();
}

void loop() {
    digitalWrite(13, HIGH);
    delay(100);
    digitalWrite(13, LOW);
    delay(500);
}

#else

int main(int argc, char **argv) {
    process();
    return 0;
}

#endif