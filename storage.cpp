#include "storage.h"
#include <EEPROM.h>
#include "CRC32.h"
#include "config.h"

#define SSID_LENGTH     64
#define PASSWORD_LENGTH 64
#define HOSTNAME_LENGTH 64

CRC32 crc;

struct Config {
  // WIFI
  char SSID[SSID_LENGTH];
  char WifiPassword[PASSWORD_LENGTH];
  char Hostname[HOSTNAME_LENGTH];
  char OTAPassword[PASSWORD_LENGTH];

  // Rotor
  float AzimuthStepsPerDeg;
  uint16_t AzimuthMotorCurrent;
  uint8_t AzimuthMicrosteps;
  
  float ElevationStepsPerDeg;
  uint16_t ElevationMotorCurrent;
  uint8_t ElevationMicrosteps;
  
  uint32_t CRC;
} currentConfig;

const size_t ConfigLength = sizeof(currentConfig);

void updateCRC() {
  crc.reset();
  crc.setPolynome(0x04C11DB7);
  crc.add((uint8_t*)&currentConfig, ConfigLength - 4); // Excluding CRC field
  currentConfig.CRC = crc.getCRC();
}

bool checkCRC(uint8_t *data, int length, uint32_t expected) {
  crc.reset();
  crc.setPolynome(0x04C11DB7);
  crc.add(data, length);
  return crc.getCRC() == expected;
}

void ConfigDefaults() {
  strncpy(currentConfig.SSID, "TRACKER", SSID_LENGTH);
  strncpy(currentConfig.WifiPassword, "1234567890", PASSWORD_LENGTH);
  strncpy(currentConfig.Hostname, "esp32-tracker", HOSTNAME_LENGTH);
  strncpy(currentConfig.OTAPassword, "1234567890", PASSWORD_LENGTH);

  currentConfig.AzimuthStepsPerDeg = AZ_STEPS_PER_DEG;
  currentConfig.ElevationStepsPerDeg = EL_STEPS_PER_DEG;
  currentConfig.AzimuthMotorCurrent = AZ_RMS_CURRENT;
  currentConfig.ElevationMotorCurrent = EL_RMS_CURRENT;
  currentConfig.AzimuthMicrosteps = AZ_MICROSTEP;
  currentConfig.ElevationMicrosteps = EL_MICROSTEP;
}

void ReadConfig() {
  char *c = (char *)(&currentConfig);
  for (int i=0; i<ConfigLength;i++) {
    c[i] = EEPROM.read(i);
  }

  // Pad all strings to be null terminated
  currentConfig.SSID[SSID_LENGTH-1] = 0x00;
  currentConfig.WifiPassword[PASSWORD_LENGTH-1] = 0x00;
  currentConfig.OTAPassword[PASSWORD_LENGTH-1] = 0x00;
  currentConfig.Hostname[HOSTNAME_LENGTH-1] = 0x00;

  if (!checkCRC((uint8_t*)&currentConfig, ConfigLength-4, currentConfig.CRC)) {
    Serial.println("Invalid configuration CRC. Loading defaults...");
    ConfigDefaults();
    SaveConfig();
  }
}

void SaveConfig() {
  currentConfig.SSID[SSID_LENGTH-1] = 0x00;
  currentConfig.WifiPassword[PASSWORD_LENGTH-1] = 0x00;
  currentConfig.OTAPassword[PASSWORD_LENGTH-1] = 0x00;
  currentConfig.Hostname[HOSTNAME_LENGTH-1] = 0x00;

  updateCRC();
  
  char *c = (char *)(&currentConfig);
  for (int i=0; i<ConfigLength;i++) {
    EEPROM.write(i, c[i]);
  }
  EEPROM.commit();
}

String GetWifiSSID() {
  return String(currentConfig.SSID);
}

String GetWifiPassword() {
  return String(currentConfig.WifiPassword);
}
String GetOTAPassword() {
  return String(currentConfig.OTAPassword);
}

String GetHostname() {
  return String(currentConfig.Hostname);
}


uint16_t GetAzimuthMotorCurrent() {
  return currentConfig.AzimuthMotorCurrent;
}

uint16_t GetElevationMotorCurrent() {
  return currentConfig.ElevationMotorCurrent;
}

uint8_t GetAzimuthMicrosteps() {
  return currentConfig.AzimuthMicrosteps;
}

uint8_t GetElevationMicrosteps() {
  return currentConfig.ElevationMicrosteps;
}

void SaveWifiSSID(String ssid) {
    int maxLen = SSID_LENGTH-1;
    if (ssid.length() < maxLen) {
      maxLen = ssid.length();
    }
    for (int i = 0; i < SSID_LENGTH; i++) {
      if (i < maxLen) {
        currentConfig.SSID[i] = ssid[i];
      } else {
        currentConfig.SSID[i] = 0x00;
      }
    }
    SaveConfig();
}

void SaveWifiPassword(String pass) {
    int maxLen = PASSWORD_LENGTH-1;
    if (pass.length() < maxLen) {
      maxLen = pass.length();
    }
    for (int i = 0; i < PASSWORD_LENGTH; i++) {
      if (i < maxLen) {
        currentConfig.WifiPassword[i] = pass[i];
      } else {
        currentConfig.WifiPassword[i] = 0x00;
      }
    }
    SaveConfig();
}

void SaveOTAPassword(String pass) {
    int maxLen = PASSWORD_LENGTH-1;
    if (pass.length() < maxLen) {
      maxLen = pass.length();
    }
    for (int i = 0; i < PASSWORD_LENGTH; i++) {
      if (i < maxLen) {
        currentConfig.OTAPassword[i] = pass[i];
      } else {
        currentConfig.OTAPassword[i] = 0x00;
      }
    }
    SaveConfig();
}

void SaveHostname(String hostname) {
    int maxLen = HOSTNAME_LENGTH-1;
    if (hostname.length() < maxLen) {
      maxLen = hostname.length();
    }
    for (int i = 0; i < HOSTNAME_LENGTH; i++) {
      if (i < maxLen) {
        currentConfig.Hostname[i] = hostname[i];
      } else {
        currentConfig.Hostname[i] = 0x00;
      }
    }
    SaveConfig();
}

void SaveAzimuthStepsPerDeg(float stepsPerDeg) {
  currentConfig.AzimuthStepsPerDeg = stepsPerDeg;
  SaveConfig();
}

void SaveElevationStepsPerDeg(float stepsPerDeg) {
  currentConfig.ElevationStepsPerDeg = stepsPerDeg;
  SaveConfig();
}

void InitStorage() {
  EEPROM.begin(ConfigLength);
  ReadConfig();
}
