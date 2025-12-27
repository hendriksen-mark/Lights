#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <AccelStepper.h>

#include "config.h"
#include "globals.h"

void senddebug();
void processSerialData(SoftwareSerial &esp8266);
// processCommand: takes null-terminated command input and writes response into respBuf (size respBufLen).
// Returns true if a response was written into respBuf.
bool processCommand(const char *inCmd, char *respBuf, size_t respBufLen);
bool handleMeadeSetInfo(const char *inCmd, char *respBuf, size_t respBufLen);
bool handleMeadeGetInfo(const char *inCmd, char *respBuf, size_t respBufLen);
// jsoninfo writes JSON into respBuf; returns true on success
bool jsoninfo(char *respBuf, size_t respBufLen);
