#include "Arduino.h"
#include "log.h"
#include "rotlibtcp.h"

#define BUFFER_SIZE 256

char buffer[BUFFER_SIZE];

void Log::printf(const char* format, ...) {
  va_list argptr;
  va_start(argptr, format);
  vsnprintf(buffer, BUFFER_SIZE, format, argptr);
  va_end(argptr);
  Serial.print(buffer);
  rotLibBroadcastTcpMessage(String(buffer));
}

void Log::println(const char* format, ...) {
  String str(format); // Gambiarra at the finest
  str += "\r\n";
  va_list argptr;
  va_start(argptr, format);
  vsnprintf(buffer, BUFFER_SIZE, str.c_str(), argptr);
  va_end(argptr);
  Serial.print(buffer);
  rotLibBroadcastTcpMessage(String(buffer));
}
