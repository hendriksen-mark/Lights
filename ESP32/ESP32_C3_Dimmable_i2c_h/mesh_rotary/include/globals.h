#pragma once

#include <Arduino.h>
#include <painlessMesh.h>
#include <ArduinoJson.h>
#include <DebugLog.h>

#include "config.h"

extern painlessMesh mesh;
extern uint32_t master;
extern int value;
extern bool change;
extern unsigned long MasterPreviousMillis;

// Value codes for actions and motion
enum ValueCode {
    ROTARY_RIGHT = 1000,
    ROTARY_LEFT  = 2000,
    PRESS_SHORT  = 3000,
    PRESS_REPEAT = 3001,
    PRESS_LONG_RELEASE = 3003,
    PRESS_LONG_PRESS = 3010,
    MOTION_DETECTED = 1,
};

// Number of bytes in a MAC address
constexpr size_t MAC_BYTES = 6;
extern const uint8_t mac0[][MAC_BYTES];

enum RoomType {
    slaapkamer = 0,
    woonkamer,
    keuken,
    gang,
    badkamer,
    gang_beweging,
    badkamer_beweging
};
