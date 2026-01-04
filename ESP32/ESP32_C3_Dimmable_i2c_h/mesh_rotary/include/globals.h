#ifndef GLOBALS_H
#define GLOBALS_H

#pragma once

#include <Arduino.h>
#include <painlessMesh.h>
#include <ArduinoJson.h>
#include <DebugLog.h>

#include "config.h"

extern painlessMesh mesh;
extern uint32_t master;
extern volatile int value;
extern volatile bool change;
extern unsigned long MasterPreviousMillis;
extern unsigned long lastMotionMillis;

// Value codes for actions and motion
enum ValueCode : uint16_t
{
    MOTION_DETECTED = 1,
    MOTION_CLEARED = 2,
    ROTARY_RIGHT = 1000,
    ROTARY_LEFT = 2000,
    PRESS_SHORT = 3000,
    PRESS_REPEAT = 3001,
    PRESS_LONG_RELEASE = 3003,
    PRESS_LONG_PRESS = 3010
};

// Number of bytes in a MAC address
constexpr size_t MAC_BYTES = 6;
constexpr size_t JSON_BUFFER_SIZE = 128;

extern const uint8_t mac0[][MAC_BYTES];

enum RoomType : uint8_t
{
    slaapkamer = 0,
    woonkamer,
    keuken,
    gang,
    badkamer,
    gang_beweging,
    badkamer_beweging
};

// Helper function to check if room is motion detector
constexpr bool isMotionDetector(RoomType roomType)
{
    return roomType == gang_beweging || roomType == badkamer_beweging;
}

#endif // GLOBALS_H
