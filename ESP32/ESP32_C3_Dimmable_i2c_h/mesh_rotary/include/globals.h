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

#define NUMBER_OF_MAC 7
extern byte mac0[][NUMBER_OF_MAC];

enum RoomType {
    slaapkamer = 0,
    woonkamer,
    keuken,
    gang,
    badkamer,
    gang_beweging,
    badkamer_beweging
};
