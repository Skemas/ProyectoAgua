#ifndef DEBUG_H
#define DEBUG_H
#include <Arduino.h>

// Activa trazas en serie sin ensuciar código
#define DEBUG 1

#if DEBUG
  #define LOG(...)    Serial.printf(__VA_ARGS__)
  #define LOGLN(...)  do { Serial.printf(__VA_ARGS__); Serial.println(); } while(0)
#else
  #define LOG(...)
  #define LOGLN(...)
#endif

#endif // DEBUG_H
