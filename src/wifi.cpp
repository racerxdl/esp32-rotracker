#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include "esp_sntp.h"

#include "wifi.h"
#include "storage.h"
#include "rotlibtcp.h"
#include "log.h"
#include "stellarium.h"

String ssid     = "";
String password = "";
String otaPassword = "";
String hostname = "";

int lastProgress = -1;
bool inOTA = false;

void initWiFi() {
  sntp_servermode_dhcp(1);
  ssid = GetWifiSSID();
  password = GetWifiPassword();
  hostname = GetHostname();
  otaPassword = GetOTAPassword();
  inOTA = false;
  Log::println("M;Chip ID: 0x%08x", (long int)ESP.getEfuseMac());

  // Set Hostname.
  if (hostname == "") {
    hostname = "TRACKER-";
    hostname += String((long int)ESP.getEfuseMac(), HEX);
  }
  Log::println("M;Hostname: %s", hostname.c_str());

  Log::println("M;Setting up WiFi");
  Log::println("M;SSID: %s", ssid.c_str());

  String tmp = "WiFi (";
  tmp += ssid;
  tmp += ")";

  if (WiFi.getMode() != WIFI_STA) {
    WiFi.mode(WIFI_STA);
    delay(10);
  }

  if (WiFi.SSID() != ssid || WiFi.psk() != password) {
    Log::println("M;WiFi config changed.");
    // ... Try to connect to WiFi station.
    WiFi.begin(ssid.c_str(), password.c_str());

    // ... Pritn new SSID
    Log::println("M;New SSID: %s", WiFi.SSID());
  } else {
    WiFi.begin();
  }

  WiFi.setHostname(hostname.c_str());
  Log::println("M;Hostname: %s", WiFi.getHostname());

  unsigned long startTime = millis();
  Serial.print("M;");
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
    delay(100);
    Serial.write('.');
    yield();
  }

  Serial.println();

  // Check connection
  if (WiFi.status() == WL_CONNECTED) {
    // ... print IP Address
    Log::println("M;IP address: %s", WiFi.localIP().toString().c_str());

    tmp = "WiFi (";
    tmp += ssid;
    tmp += ")";

    delay(1500);
  } else {
    Log::println("M;Can not connect to WiFi station. Go into AP mode.");

    // Go into software AP mode.
    WiFi.mode(WIFI_AP);

    delay(10);

    WiFi.softAP("TRACKER", "1234567890");

    delay(1500);

    Log::println("M;IP address: %s", WiFi.softAPIP().toString().c_str());
  }

  // Start OTA server.
  ArduinoOTA.setHostname((const char *)hostname.c_str());

  ArduinoOTA.onStart([]() {
    inOTA = true;
    Log::println("M;OTA Update Start");
  });

  ArduinoOTA.onEnd([]() {
    Log::println("M;OTA Update End");
    rotLibDisconnectAll();
    stellariumDisconnectAll();
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    String progressStr;
    int currentProgress = ((progress / (total / 100)));
    if (lastProgress != currentProgress) {
      Log::println("M;OTA Update Progress: %d %%", currentProgress);
      lastProgress = currentProgress;
    }
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Log::println("M;OTA Update Error [%u]: ", error);
    if (error == OTA_AUTH_ERROR) Log::println("M;Auth failed");
    else if (error == OTA_BEGIN_ERROR) Log::println("M;Start Failed");
    else if (error == OTA_CONNECT_ERROR) Log::println("M;Connection failed");
    else if (error == OTA_RECEIVE_ERROR) Log::println("M;Receive Error");
    else if (error == OTA_END_ERROR) Log::println("M;End Fail");
  });
  ArduinoOTA.setPassword(otaPassword.c_str());
  ArduinoOTA.begin();
}

void updateWiFi() {
  ArduinoOTA.handle();
}

bool InOTA() {
  return inOTA;
}
