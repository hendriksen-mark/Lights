#pragma once

#include <TMCStepper.h>

#include "config.h"

extern TMC2209Stepper *driver;

void configureRAdriver(uint16_t RA_SW_RX, uint16_t RA_SW_TX, float rsense, byte driveraddress, int rmscurrent, int stallvalue);
