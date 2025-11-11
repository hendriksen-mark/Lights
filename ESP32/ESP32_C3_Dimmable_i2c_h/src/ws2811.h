#pragma once

#include <NeoPixelBus.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <HTTPUpdateServer.h>

#include "debug.h"
#include "functions.h"
#include "server.h"

void convertHue(uint8_t light);
void convertXy(uint8_t light);
void convertCt(uint8_t light);
void handleNotFound_ws();
void apply_scene_ws(uint8_t new_scene);
void processLightdata(uint8_t light, float transitiontime = 4);
RgbColor blending(float left[3], float right[3], uint8_t pixel);
void candleEffect();
void firePlaceEffect();
RgbColor convFloat(float color[3]);
void cutPower();
void lightEngine();
void saveState();
void restoreState();
bool saveConfig();
bool loadConfig();
void ChangeNeoPixels(uint16_t newCount);
void ws_setup();
RgbColor blendingEntert(float left[3], float right[3], float pixel);
void entertainment();
void ws_loop();