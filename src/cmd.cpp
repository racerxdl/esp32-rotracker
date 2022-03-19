#include "steppers.h"
#include "as5600.h"
#include "storage.h"
#include "wifi.h"
#include "log.h"

float elPos;
float azPos;

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
  char buff[64];
  
  elPos = elStepToDeg(getElStepper().currentPosition());
  azPos = azStepToDeg(getAzStepper().currentPosition());
  sprintf(buff, "%.6f\n%.6f\nRPRT 0", azPos, elPos);
//  for (int i = 0; i < 64; i++) {
//    if (buff[i] == '.') {
//      buff[i] = ',';
//    }
//  }
  Log::println("%s", buff);
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
  float calibAngle = startAngle + 45;
  float currentAngle = startAngle;
  long startStepPos = getAzStepper().currentPosition();
  long pos = getAzStepper().currentPosition();
  while (currentAngle < calibAngle) {
    pos += 10;
    getAzStepper().moveTo(pos);
    while (getAzStepper().currentPosition() != pos) {
      stepLoop();
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
  Log::println("M;Steps per degree: %d", stepsPerDeg);
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
