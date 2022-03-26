#include <time.h>
#include <sys/time.h>
#include <cstdlib>
#include "esp_sntp.h"

#include "clock.h"
#include "log.h"
#include "storage.h"
#include "Sideral.h"

time_t now;
struct timeval tv_now;
struct tm *tm_struct = NULL;
float LST_f;

float getSideralTime() {
  return LST_f;
}

int getHours() {
  if (tm_struct == NULL) {
    return 0;
  }
  return tm_struct->tm_hour;
}

int getMinutes() {
  if (tm_struct == NULL) {
    return 0;
  }
  return tm_struct->tm_min;
}

int getSeconds() {
  if (tm_struct == NULL) {
    return 0;
  }
  return tm_struct->tm_sec;
}

int getDay() {
  if (tm_struct == NULL) {
    return 0;
  }
  return tm_struct->tm_mday;
}

int getYear() {
  if (tm_struct == NULL) {
    return 0;
  }
  return tm_struct->tm_year;
}

int getMonth() {
  if (tm_struct == NULL) {
    return 0;
  }
  return tm_struct->tm_mon;
}

unsigned long getEpoch() {
  return (unsigned long)now;
}

void initClock() {
  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_setservername(0, "pool.ntp.br");
  sntp_init();
  setenv("TZ", GetTimeOffset(), 1);
  tzset();

  int retry = 0;
  const int retry_count = 15;
  while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
      Log::println("M;Waiting for system time to be set... (%d/%d)", retry, retry_count);
      vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
  updateClock();
  Log::println("M;Current time is %02d:%02d epoch %lu", getHours(), getMinutes(), getEpoch());
}

// Sideral Clock Calculation based on http://woodsgood.ca/projects/2015/06/14/equatio-sidereal-solar-clock/

long lastUpdate = 0;
int64_t lastSideralUpdate = 0;

void updateSideralClock() {
  int64_t time_us = (int64_t)tv_now.tv_sec * 1000000L + (int64_t)tv_now.tv_usec;
  // we assume millisecond precision so no need to update if millis hasnt changed
  if (tm_struct == NULL || ((time_us/1000) == (lastSideralUpdate/1000))) {
    return;
  }

  double julianDate = julianDateFromUnixEpochMicros(time_us);
  LST_f = GMTmeanSideralTime(julianDate);
  LST_f = fmod(LST_f + deg2rad(sidLongitude()), 2 * PI);
  lastSideralUpdate = time_us;
}

void updateClock() {
  if (millis() - lastUpdate >= 10) {
    now = time(NULL);
    gettimeofday(&tv_now, NULL);
    tm_struct = localtime(&now);
    updateSideralClock();
    lastUpdate = millis();
  }
}