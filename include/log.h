#pragma once

#include "Arduino.h"

class Log {
public:
  static void printf(const char* format, ...);
  static void println(const char* format, ...);
};
