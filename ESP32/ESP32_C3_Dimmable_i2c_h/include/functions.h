#pragma once

#include <NeoPixelBus.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

#include "config.h"
#include "debug.h"

extern RgbColor red;
extern RgbColor green;
extern RgbColor white;
extern RgbColor black;

void functions_setup();
void ChangeNeoPixels_info();
void setInfoLedBrightness(float brightness);
void infoLight(RgbColor color);
void infoLedOff();
void infoLedFadeIn(RgbColor color, uint16_t duration = 500);
void infoLedFadeOut(uint16_t duration = 500);
void infoLedPulse(RgbColor color, uint8_t pulses = 1, uint16_t pulseDuration = 1000);
void infoLedIdle();
void infoLedBusy();
void infoLedSuccess();
void infoLedError();
void blinkLed(uint8_t count, uint16_t interval = 200);
void factoryReset();
void resetESP();

// Generic JSON file helpers (shared by multiple modules)
bool readJsonFile(const char* path, JsonDocument &doc);
bool writeJsonFile(const char* path, JsonDocument &doc);
