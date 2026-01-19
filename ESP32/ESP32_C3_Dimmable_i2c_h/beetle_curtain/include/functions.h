#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#pragma once

#include "globals.h"

// Function declarations implemented in src/functions.cpp and src/main.cpp
void set_Target_Pos(byte target_set);
void senddebug();
void send_change(bool toMaster);
// Motor control / homing
void stable();
void homeing();
void stopMotor();

#endif // FUNCTIONS_H
