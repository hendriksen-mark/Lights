#pragma once

#include <TMCStepper.h>

#include "globals.h"
#include "config.h"

void configureRAdriver(uint16_t RA_SW_RX, uint16_t RA_SW_TX, float rsense, byte driveraddress, int rmscurrent, int stallvalue);
