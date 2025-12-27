#pragma once
#include <Arduino.h>
#include <AccelStepper.h>

#include "config.h"

// Globals (defined in src/main.cpp)
extern AccelStepper stepper;
extern byte target;
extern byte pref_target;
extern byte current;
extern long uitvoeren;
extern long totalstep;
extern volatile bool ishome;
extern bool ismoved;
extern volatile bool gohome;
extern unsigned long prev_millis;
extern volatile bool stopRequested;
extern volatile unsigned long lastHomeEvent;
extern volatile bool homingActive;
extern volatile uint8_t homingStage;
extern long homingTarget;
// Homing parameters (tuned for your geartrain)
// Gear ratio: stepper 12T -> 30T (on same shaft 12T) -> 24T => overall 5:1 reduction
// microsteps per motor rev = MOTOR_STEPS * MICROSTEPS = 200 * 16 = 3200
// steps per rack rev = microsteps * 5 = 3200 * 5 = 16000
// linear per rack rev = pi * module * 24 = ~122.072832 mm (with module=1.61860385)
// mm per microstep = linear per rack rev / steps per rack rev = ~0.00762955 mm
constexpr int HOMING_FAST_MULT = 4; // coarse speed multiplier (reduced for safety)
constexpr int HOMING_SLOW_MULT = 1; // fine approach multiplier (slow for precision)
constexpr long STEPS_PER_RACK_REV = 5 * MOTOR_STEPS * MICROSTEPS; // 16000 steps
constexpr double MM_PER_RACK_REV = 3.141592653589793 * 1.61860385 * 24.0; // ~122.072832 mm
constexpr double MM_PER_MICROSTEP = MM_PER_RACK_REV / (double)STEPS_PER_RACK_REV; // ~0.00762955 mm
// Homing distances (choose coarse large enough to find switch, small backoff/fine for precision)
constexpr long HOMING_COARSE_STEPS = STEPS_PER_RACK_REV; // 1 rack rev (~122 mm)
constexpr long HOMING_BACKOFF_STEPS = (long)(10.0 / MM_PER_MICROSTEP); // ~10 mm backoff
constexpr long HOMING_FINE_STEPS = (long)(5.0 / MM_PER_MICROSTEP); // ~5 mm fine approach
constexpr unsigned long HOMING_DEBOUNCE_MS = 50;
constexpr unsigned long HOMING_TIMEOUT_MS = 30000;

extern volatile unsigned long homingStartTime;
extern volatile unsigned long homingTimestamp;
// Note: you said two stepper motors are driven by one TMC driver.
// Driving two motors in parallel from one driver is not generally recommended:
// - Current limit set by the driver will be shared; each motor may receive less current than expected.
// - Verify wiring and reduce `R_current` accordingly, or use one driver per motor for independent current control.

// Functions implemented in src/main.cpp
void stable();
void homeing();
void stopMotor();
