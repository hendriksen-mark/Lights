#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#pragma once

#include "Button2.h"
#include "ESPRotary.h" //1.5.0
#include "painlessMesh.h"
#include <ArduinoJson.h>

#include "globals.h"
#include "buttons.h"

// Function declarations implemented in src/functions.cpp and src/main.cpp
void receivedCallback(uint32_t from, String &msg);
void ask_master();
void send_change();
String macToStr(const uint8_t *mac);

#endif // FUNCTIONS_H
