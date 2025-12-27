#pragma once

#include <NeoPixelBus.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPUpdateServer.h>
#include <new>

#include "config.h"
#include "debug.h"
#include "functions.h"
#include "color.h"
#include "ws2811_http_content.h"

void handleNotFound_ws();
void apply_scene_ws(uint8_t new_scene);
void processLightdata(uint8_t light, float transitiontime = 4);
RgbColor blending(const float left[3], const float right[3], uint8_t pixel);
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
RgbColor blendingEntert(const float left[3], const float right[3], float pixel);
void entertainment();
void ws_loop();
