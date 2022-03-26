#pragma once
#include <WString.h>

String GetWifiSSID();
String GetWifiPassword();
String GetHostname();
String GetOTAPassword();
float GetAzimuthStepsPerDeg();
float GetElevationStepsPerDeg();

uint16_t GetAzimuthMotorCurrent();
uint16_t GetElevationMotorCurrent();

uint8_t GetAzimuthMicrosteps();
uint8_t GetElevationMicrosteps();

int GetElevationOffset();
int GetAzimuthOffset();

float GetLatitude();
float GetLongitude();
const char *GetTimeOffset();

void SaveWifiSSID(String ssid);
void SaveWifiPassword(String pass);
void SaveHostname(String hostname);
void SaveOTAPassword(String password);
void SaveAzimuthStepsPerDeg(float stepsPerDeg);
void SaveElevationStepsPerDeg(float stepsPerDeg);
void SaveLatitude(float latitude);
void SaveLongitude(float longitude);
void SaveTimeOffset(const char *timeoffset);
void SaveElevationOffset(int offset);
void SaveAzimuthOffset(int offset);

void SaveConfig();

void initStorage();
