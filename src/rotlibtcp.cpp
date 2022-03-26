#include <Arduino.h>
#include <AsyncTCP.h>
#include <WiFi.h>
#include <map>
#include "rotlibtcp.h"
#include "cmd.h"

#define TCP_SERVER_PORT 4533

AsyncServer rotlibTcpServer(TCP_SERVER_PORT);
std::map<AsyncClient *, int> rotLibTcpConnectedClients;

void rotLibHandleData(void *arg, AsyncClient *client, void *data, size_t len) {
  String cmd;
  for (int i = 0; i < len; i++) {
    cmd += ((char *)data)[i];
  }
  //Serial.printf("M;Rotlibtcp rxdata IP: %s - %s\r\n", client->remoteIP().toString().c_str(), cmd.c_str());
  runCommand(cmd);
}

void rotLibHandleError(void *arg, AsyncClient *client, int8_t error) {
  Serial.printf("M;Rotlibtcp error from client IP: %s - %s\r\n", client->remoteIP().toString().c_str(), client->errorToString(error));
}

void rotLibHandleDisconnect(void *arg, AsyncClient *client) {
  Serial.printf("M;Rotlibtcp disconnected. IP: %s\r\n", client->remoteIP().toString().c_str());
  rotLibTcpConnectedClients.erase(client);
}

void rotLibHandleTimeout(void *arg, AsyncClient *client, uint32_t time) {
  Serial.printf("M;Rotlibtcp timeout. IP: %s\r\n", client->remoteIP().toString().c_str());
  rotLibTcpConnectedClients.erase(client);
}

void rotLibHandleNewClient(void *arg, AsyncClient *client) {
  Serial.printf("M;New client on rotlibtcp. IP: %s\r\n", client->remoteIP().toString().c_str());
  client->onData(&rotLibHandleData, NULL);
  client->onError(&rotLibHandleError, NULL);
  client->onDisconnect(&rotLibHandleDisconnect, NULL);
  client->onTimeout(&rotLibHandleTimeout, NULL);

  rotLibTcpConnectedClients[client] = 1;
}

void initRotlibTCP() {
  rotlibTcpServer.onClient(&rotLibHandleNewClient, &rotlibTcpServer);
  rotlibTcpServer.begin();
  Serial.printf("M;Started RotLib TCP Server at port %d\r\n", TCP_SERVER_PORT);
}

void rotLibBroadcastTcpMessage(const String &msg) {
  for (const std::pair<AsyncClient*,int> pair : rotLibTcpConnectedClients) {
    pair.first->write(msg.c_str(), msg.length());
  }
}

void rotLibDisconnectAll() {
  for (const std::pair<AsyncClient*,int> pair : rotLibTcpConnectedClients) {
    pair.first->close();
  }
  rotLibTcpConnectedClients.clear();
}
