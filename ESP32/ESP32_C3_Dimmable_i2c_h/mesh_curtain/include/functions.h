#pragma once

#include <painlessMesh.h>
#include <ArduinoJson.h>

#include "config.h"
// Shared globals (defined in src/main.cpp)
extern painlessMesh mesh;
extern uint32_t master;
extern byte target_set;
extern byte target_ont;
extern byte current_ont;
extern int state_ont;

int sendData(const char* command, char* responseBuf, size_t respBufLen, const int timeout, bool set_target, uint8_t retries=1);
void deserial(const char* message);
void set_Target_Pos(byte target_set);
void homeing();
void get_info();
void set_reset();
void ask_master();
void send_change();
