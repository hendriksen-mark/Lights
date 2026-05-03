#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#pragma once

#include "globals.h"

// Function declarations implemented in src/functions.cpp and src/main.cpp
void serialWait();
void infoLedPulse(uint8_t pulses, uint16_t pulseDuration);
void infoLedFadeIn(uint16_t duration);
void infoLedFadeOut(uint16_t duration);
void setLedColor(uint32_t color);
void setLedColorRGB(uint8_t r, uint8_t g, uint8_t b);
void setLedError(LedErrorCode errorCode);
void ledBlink(uint32_t color, uint8_t times, uint16_t duration);
void set_Target_Pos(byte target_set);
void senddebug();
void send_change(bool toMaster);
void restoreNormalMotionProfile();
// Motor control / homing
void stable();
void homeing();
void stopMotor();

#endif // FUNCTIONS_H
