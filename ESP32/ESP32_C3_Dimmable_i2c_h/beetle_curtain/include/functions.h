#pragma once

#include "globals.h"

// Function declarations implemented in src/functions.cpp and src/main.cpp
void set_Target_Pos(byte target_set);
void senddebug();
void ask_master();
void send_change();
// Motor control / homing
void stable();
void homeing();
void stopMotor();

