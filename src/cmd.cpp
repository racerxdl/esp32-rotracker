#include "steppers.h"
#include "as5600.h"
#include "storage.h"
#include "wifi.h"
#include "log.h"
#include "rotlibtcp.h"

String strip(String src) {
  int R = src.indexOf("\r");
  int N = src.indexOf("\n");
  if (R == -1 && N == -1) {
    return src;
  }

  if (R == -1) { // N isnt -1
    return src.substring(0, N);
  }

  if (R > N) { // \n is first
    return src.substring(0, N);
  }

  // \r is first
  return src.substring(0, R);
}

String firstParam(String src, int *indexOut) {
  int firstParamEndIndex = src.indexOf(" ");
  String res = src.substring(0, firstParamEndIndex);
  if (indexOut != NULL) {
    *indexOut = firstParamEndIndex;
  }
  return res;
}

void printPos() {
  float elPos = elStepToDeg(getElStepper().currentPosition());
  float azPos = azStepToDeg(getAzStepper().currentPosition());
  Log::println("%.6f\n%.6f\nRPRT 0", azPos, elPos);
}

void gotoPosition(String cmdData) {
  if (cmdData[cmdData.length()-1] == '\r') {
    cmdData = cmdData.substring(0, cmdData.length()-1);
  }
  for (int i = 0; i < cmdData.length(); i++) {
    if (cmdData[i] == ',') {
      cmdData[i] = '.';
    }
  }
  int firstParamIndex = cmdData.indexOf(" ");
  String tmp1 = cmdData.substring(0, firstParamIndex);
  cmdData = cmdData.substring(firstParamIndex);
  float azTarget = tmp1.toFloat();
  String tmp2 = cmdData.substring(cmdData.indexOf(" "));
  float elTarget = tmp2.toFloat();
  Log::println("M;Target set to %.2f;%.2f", elTarget, azTarget);
  getElStepper().moveTo(elDegToStep(elTarget));
  getAzStepper().moveTo(azDegToStep(azTarget));
}

void printCmdUnknown() {
  Log::println("M;Invalid command");
  Log::println("RPRT 100");
}

void stopRotor() {
  getElStepper().moveTo(getElStepper().currentPosition());
  getAzStepper().moveTo(getAzStepper().currentPosition());
}

void parkRotor() {
  stopRotor();
  getElStepper().moveTo(0);
  getAzStepper().moveTo(0);
}

void printInfo() {
  Log::println("M;Teske's Lab Rotor Controller");
  Log::println("M;Current time is %02d:%02d epoch %d", getHours(), getMinutes(), getEpoch());
}

void calibAz() {
  Log::println("M;Calibrating Azimuth");
  getAzEncoder().update();
  float startAngle = getAzEncoder().getAngle();
  Log::println("M;Starting angle: ");
  Log::println(String(startAngle).c_str());
  float calibAngle = startAngle + 60;
  float currentAngle = startAngle;
  long startStepPos = getAzStepper().currentPosition();
  long pos = getAzStepper().currentPosition();
  Log::println("M;Coarse Angle Moving");
  while (calibAngle - currentAngle > 5) { // Move faster for > 5 degrees diference
    pos += 200;
    getAzStepper().moveTo(pos);
    while (getAzStepper().currentPosition() != pos) {
      StepLoop();
      yield();
    }
    getAzEncoder().update();
    currentAngle = getAzEncoder().getAngle();
    Log::println("M;Angle: %.2f - Steps: %d", currentAngle, pos - startStepPos);
  }
  Log::println("M;Fine Angle Moving");
  while (currentAngle < calibAngle) {
    pos += 10;
    getAzStepper().moveTo(pos);
    while (getAzStepper().currentPosition() != pos) {
      StepLoop();
      yield();
    }
    getAzEncoder().update();
    currentAngle = getAzEncoder().getAngle();
    Log::println("M;Angle: %.2f - Steps: %d", currentAngle, pos - startStepPos);
  }

  getAzEncoder().update();
  currentAngle = getAzEncoder().getAngle();

  float deltaAngle = currentAngle - startAngle;
  long deltaSteps = pos - startStepPos;
  Log::println("M;Delta Angle: %.2f - Delta Steps: %d", deltaAngle, deltaSteps);

  float stepsPerDeg = deltaSteps / deltaAngle;
  Log::println("M;Steps per degree: %f", stepsPerDeg);
}

void zeroAngles() {
  setAzCurrentPosition(0);
  setElCurrentPosition(0);
  Log::println("M;Angles set to zero");
}

void setWifiPass(String cmdData) {
  String pass = strip(cmdData);
  Log::println("M;Setting wifi password to \"%s\"", pass.c_str());
  SaveWifiPassword(pass);
}

void setSSID(String cmdData) {
  String ssid = strip(cmdData);
  Log::println("M;Setting SSID to \"%s\"", ssid.c_str());
  SaveWifiSSID(ssid);
}

void setHostname(String cmdData) {
  String hostname = strip(cmdData);
  Log::println("M;Setting hostname to \"%s\"", hostname.c_str());
  SaveHostname(hostname);
}

void setOTAPass(String cmdData) {
  String pass = strip(cmdData);
  Log::println("M;Setting OTA Password to \"%s\"", pass.c_str());
  SaveOTAPassword(pass);
}

void reboot() {
  Log::println("M;Disconnecting all clients");
  disconnectAll();
  Log::println("M;Rebooting");
  ESP.restart();
}

void runCommand(String cmdData) {
  int stripPos;
  String cmd = firstParam(cmdData, &stripPos);
  cmdData = cmdData.substring(stripPos+1);

  if (cmd == "get_pos")       { printPos();             } else
  if (cmd == "set_pos")       { gotoPosition(cmdData);  } else
  if (cmd == "stop")          { stopRotor();            } else
  if (cmd == "park")          { parkRotor();            } else
  if (cmd == "calib_az")      { calibAz();              } else
  if (cmd == "zero_angles")   { zeroAngles();           } else
  if (cmd == "set_ssid")      { setSSID(cmdData);       } else
  if (cmd == "set_wifipass")  { setWifiPass(cmdData);   } else
  if (cmd == "set_hostname")  { setHostname(cmdData);   } else
  if (cmd == "set_otapass")   { setOTAPass(cmdData);    } else
  if (cmd == "reboot")        { reboot();               } else
  if (cmd == "get_info")      { printInfo();            } else {
    // Short version of commands
    switch (cmd[0]) {
      case 'p': printPos();             break;
      case 'P': gotoPosition(cmdData);  break;
      case 'S': stopRotor();            break;
      case 'K': parkRotor();            break;
      case '_': printInfo();            break;
      default:  printCmdUnknown();
    }
  }
}
String inputString = "";
void CmdInit() {
  inputString.reserve(200);
}

void CmdLoop() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar != '\r' && inChar != '\n') {
      inputString += inChar;
    }

    if (inChar == '\n') {
      runCommand(inputString);
      inputString = "";
    }
  }
}