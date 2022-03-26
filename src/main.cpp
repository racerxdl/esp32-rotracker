#include <Arduino.h>

#include "steppers.h"
#include "config.h"
#include "cmd.h"
#include "as5600.h"
#include "storage.h"
#include "wifi.h"
#include "rotlibtcp.h"
#include "log.h"
#include "clock.h"
#include "stellarium.h"

void setup() {
  delay(2000); // For ESP32 that fails to boot properly when flashing
  Serial.begin(115200);
  Log::println("M;Initializing");
  Wire.begin();
  if (!getAzEncoder().init()) {
    Log::println("M;Error initializing Azimuth Encoder");
    while(true);
  }

  Log::println("M;Initializing storage");
  initStorage();

  Log::println("M;Initializing wifi");
  initWiFi();
  initClock();

  initRotlibTCP();
  initStellarium();

  Log::println("M;Initializing Steppers");
  initSteppers();

  Log::println("M;Initializing Azimuth Control");
  getAzEncoder().update();
  if (getAzEncoder().hasMagnet()) {
    float azAngle = getAzEncoder().getAngle();
    int azSteps = azDegToStep(azAngle);
    setAzCurrentPosition(azSteps);

    Log::println("M;Current Azimuth Position %.2f deg | %d steps", azAngle, azSteps);
  } else {
    Log::println("M;Magnet not found in encoder. Check azimuth magnet position.");
  }
  Log::println("M;Done");
  initCmd();
}

void loop() {
  updateWiFi();
  updateClock();
  updateCmd();
  updateSteppers();
  updateStellarium();
}
