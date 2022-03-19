#include <Arduino.h>
#include <AsyncTCP.h>
#include <WiFi.h>
#include <map>
#include "rotlibtcp.h"
#include "cmd.h"

#define TCP_SERVER_PORT 4533

AsyncServer server(TCP_SERVER_PORT);
std::map<AsyncClient *, int> connectedClients;

void handleData(void *arg, AsyncClient *client, void *data, size_t len) {
  String cmd;
  for (int i = 0; i < len; i++) {
    cmd += ((char *)data)[i];
  }
  Serial.printf("M;Rotlibtcp rxdata IP: %s - %s\r\n", client->remoteIP().toString().c_str(), cmd.c_str());
  runCommand(cmd);
}

void handleError(void *arg, AsyncClient *client, int8_t error) {
  Serial.printf("M;Rotlibtcp error from client IP: %s - %s\r\n", client->remoteIP().toString().c_str(), client->errorToString(error));
}

void handleDisconnect(void *arg, AsyncClient *client) {
  Serial.printf("M;Rotlibtcp disconnected. IP: %s\r\n", client->remoteIP().toString().c_str());
  connectedClients.erase(client);
}

void handleTimeOut(void *arg, AsyncClient *client, uint32_t time) {
  Serial.printf("M;Rotlibtcp timeout. IP: %s\r\n", client->remoteIP().toString().c_str());
  connectedClients.erase(client);
}

void handleNewClient(void *arg, AsyncClient *client) {
  Serial.printf("M;New client on rotlibtcp. IP: %s\r\n", client->remoteIP().toString().c_str());
  client->onData(&handleData, NULL);
  client->onError(&handleError, NULL);
  client->onDisconnect(&handleDisconnect, NULL);
  client->onTimeout(&handleTimeOut, NULL);

  connectedClients[client] = 1;
}

void initRotlibTCP() {
  server.onClient(&handleNewClient, &server);
  server.begin();
  Serial.printf("M;Started RotLib TCP Server at port %d\r\n", TCP_SERVER_PORT);
}

void broadcastTcpMessage(const String &msg) {
  for (const std::pair<AsyncClient*,int> pair : connectedClients) {
    pair.first->write(msg.c_str(), msg.length());
  }
}

void disconnectAll() {
  for (const std::pair<AsyncClient*,int> pair : connectedClients) {
    pair.first->close();
  }
  connectedClients.clear();
}
