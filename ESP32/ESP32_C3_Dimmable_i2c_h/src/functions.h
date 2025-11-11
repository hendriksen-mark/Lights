#pragma once

#include <NeoPixelBus.h>
#include <LittleFS.h>

#include "debug.h"

RgbColor red = RgbColor(255, 0, 0);
RgbColor green = RgbColor(0, 255, 0);
RgbColor white = RgbColor(255);
RgbColor black = RgbColor(0);

void functions_setup();
void ChangeNeoPixels_info();
void infoLight(RgbColor color);
void blinkLed(uint8_t count, uint16_t interval = 200);
void factoryReset();