#pragma once

#include <NeoPixelBus.h>
#include <LittleFS.h>

#include "config.h"
#include "debug.h"

extern RgbColor red;
extern RgbColor green;
extern RgbColor white;
extern RgbColor black;

void functions_setup();
void ChangeNeoPixels_info();
void infoLight(RgbColor color);
void blinkLed(uint8_t count, uint16_t interval = 200);
void factoryReset();
