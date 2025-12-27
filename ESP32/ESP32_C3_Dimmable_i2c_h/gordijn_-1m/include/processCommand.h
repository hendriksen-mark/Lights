#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <AccelStepper.h>

#include "config.h"
#include "globals.h"

void senddebug();
void processSerialData(SoftwareSerial &esp8266);
String processCommand(String inCmd);
String handleMeadeSetInfo(String inCmd);
String handleMeadeGetInfo(String inCmd);
String jsoninfo();
