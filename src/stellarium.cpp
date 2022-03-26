#include <Arduino.h>
#include <math.h>
#include <AsyncTCP.h>
#include <map>
#include "stellarium.h"
#include "clock.h"
#include "storage.h"
#include "steppers.h"
#include "Sideral.h"
#include "log.h"


#define TCP_SERVER_PORT 10001

AsyncServer stellariumServer(TCP_SERVER_PORT);
std::map<AsyncClient *, int> stellariumConnectedClients;

float lastAscension;
float lastDeclination;
int lastUpdateElSteps = 0;
int lastUpdateAzSteps = 0;
bool stellariumTracking = false;

void GetRADEC(float *rightAscension, float *declination) {
  int elSteps = getElStepper().currentPosition();
  int azSteps = getAzStepper().currentPosition();
  float elAngle = deg2rad(elStepToDeg(elSteps) + GetElevationOffset());
  float azAngle = deg2rad(azStepToDeg(azSteps) + GetAzimuthOffset());
  AltAztoRADEC(getSideralTime(), elAngle, azAngle, rightAscension, declination);
}

void GoToRADEC(float rightAscension, float declination) {
  float elAngle, azAngle;
  RADECtoAltAz(getSideralTime(), rightAscension, declination, &elAngle, &azAngle);
  int elSteps = elDegToStep(elAngle - GetElevationOffset());
  int azSteps = azDegToStep(azAngle - GetAzimuthOffset());
  //Log::println("Going to %.4f - %.4f", elAngle, azAngle);
  getElStepper().moveTo(elSteps);
  getAzStepper().moveTo(azSteps);
  lastAscension = rightAscension;
  lastDeclination = declination;
}

float stellariumGetLastAscension() {
  return lastAscension;
}

float stellariumGetLastDeclination() {
  return lastDeclination;
}

// TCP Server
StellariumPacket rxPacket;
void stellariumHandleData(void *arg, AsyncClient *client, void *data, size_t len) {
  if (len > sizeof(StellariumPacket)) {
    len = sizeof(StellariumPacket);
  }
  memset(&rxPacket, 0, sizeof(StellariumPacket));
  memcpy(&rxPacket, data, len);
  float RA = (float)rxPacket.rightAscension;
  RA /= 0x80000000u;
  RA *= PI;

  float dec = (float)rxPacket.declination;
  dec /= 0x80000000u;
  dec *= PI;

  dec = rad2deg(dec);
  RA = rad2deg(RA) / 15;

  // Serial.printf("M;Stellarium RAW: %08x %08x\r\n", packet.rightAscension, packet.declination);
  // Serial.printf("M;Stellarium rxdata IP: %s - RA=%0.4f | DEC=%0.4f\r\n", client->remoteIP().toString().c_str(), RA, dec);
  Serial.printf("M;Stellarium goto from IP: %s - RA=%0.4f | DEC=%0.4f\r\n", client->remoteIP().toString().c_str(), RA, dec);
  if (stellariumTracking) {
    stellariumTracking = false;
    lastAscension = RA;
    lastDeclination = dec;
  }
  stellariumTracking = true;
  GoToRADEC(RA, dec);
}

void stellariumHandleError(void *arg, AsyncClient *client, int8_t error) {
  Serial.printf("M;Stellarium error from client IP: %s - %s\r\n", client->remoteIP().toString().c_str(), client->errorToString(error));
}

void stellariumHandleDisconnect(void *arg, AsyncClient *client) {
  Serial.printf("M;Stellarium disconnected. IP: %s\r\n", client->remoteIP().toString().c_str());
  stellariumConnectedClients.erase(client);
  stellariumTracking = false;
}

void stellariumHandleTimeout(void *arg, AsyncClient *client, uint32_t time) {
  Serial.printf("M;Stellarium timeout. IP: %s\r\n", client->remoteIP().toString().c_str());
  stellariumConnectedClients.erase(client);
}

void stellariumHandleNewClient(void *arg, AsyncClient *client) {
  Serial.printf("M;New client on stellarium. IP: %s\r\n", client->remoteIP().toString().c_str());
  client->onData(&stellariumHandleData, NULL);
  client->onError(&stellariumHandleError, NULL);
  client->onDisconnect(&stellariumHandleDisconnect, NULL);
  client->onTimeout(&stellariumHandleTimeout, NULL);

  stellariumConnectedClients[client] = 1;
}


void stellariumBroadcastTcpMessage(const StellariumPacket &packet) {
  for (const std::pair<AsyncClient*,int> pair : stellariumConnectedClients) {
    pair.first->write((char *)&packet, sizeof(StellariumPacket));
  }
}

void stellariumDisconnectAll() {
  for (const std::pair<AsyncClient*,int> pair : stellariumConnectedClients) {
    pair.first->close();
  }
  stellariumConnectedClients.clear();
}

void initStellarium() {
  initSideral(GetLatitude(), GetLongitude());

  stellariumServer.onClient(&stellariumHandleNewClient, &stellariumServer);
  stellariumServer.begin();
  Serial.printf("M;Started Stellarium TCP Server at port %d\r\n", TCP_SERVER_PORT);
}

unsigned long lastStellariumUpdate = 0;
unsigned long lastStellariumTrack = 0;

StellariumPacket packet;

void updateStellarium() {
  static float ra, dec; // Avoid alocating memory

  unsigned long updateDelta = millis() - lastStellariumUpdate;
  unsigned long trackDelta = millis() - lastStellariumTrack;

  if (updateDelta >= 250 && !stellariumConnectedClients.empty()) {
    GetRADEC(&ra, &dec);
    ra = deg2rad(ra * 15);
    dec = deg2rad(dec);
    packet.length = sizeof(StellariumPacket);
    packet.type = 0;
    packet.rightAscension =  static_cast<uint32_t>(floor(0.5 + ra*((static_cast<unsigned int>(0x80000000))/PI)));;
    packet.declination = static_cast<int32_t>(floor(0.5 + dec*((static_cast<unsigned int>(0x80000000))/PI)));
    packet.time = getEpoch();
    packet.status = 0;
    stellariumBroadcastTcpMessage(packet);
    lastStellariumUpdate = millis();
  }

  if (stellariumTracking && trackDelta >= 100) {
    GoToRADEC(lastAscension, lastDeclination);
    lastStellariumTrack = millis();
  }
}