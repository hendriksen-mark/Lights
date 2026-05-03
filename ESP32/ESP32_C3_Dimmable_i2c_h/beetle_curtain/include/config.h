#ifndef CONFIG_H
#define CONFIG_H

#pragma once

#define DEVICE_NAME "curtain"

#define MESH_PREFIX "HomeMesh"
#define MESH_PASSWORD "Qwertyuiop1"
#define MESH_PORT 5555

#define REQUEST_TIMEOUT 2000

#define LED_PIN 0 // WS2812B RGB LED pin on SuperMini
#define LED_COUNT 1 // Number of WS2812B LEDs in the strip

#define HOME_SWITCH 4

#define DIR 6
#define STEP 7
#define ENABLE 9
#define DRIVER_ADDRESS_1 0b00 // Left motor driver - MS1/MS2 = LOW/LOW
#define DRIVER_ADDRESS_2 0b01 // Right motor driver - MS1/MS2 = HIGH/LOW
#define R_SENSE 0.11f
#define SW_RX 20
#define SW_TX 21
#define INVERT_DIR true
#define INVERT_STEP false
#define INVERT_ENABLE true

#define TOTALROND 123l
#define MOTOR_STEPS 200l
#define MICROSTEPS 16l  // UART-controlled microstepping (requires mstep_reg_select=1)
#define MOTORSPEED 625 //*16=10000 // speed voor full step
#define HOME_MOTOR_SPEED 200 //*16=3200 // homing speed for coarse approach
#define MOTORACC 400   //*16=3200 // Acceleration voor full step
#define HOME_MOTOR_ACC 200   //*16=1600 // homing acceleration for coarse approach
#define TOTALSTEPS (TOTALROND * MOTOR_STEPS * MICROSTEPS)
#define HOMING_ZERO_OFFSET_TURNS 3l // after switch trigger, move right this many motor turns then set position to 0

#define UART_STEALTH_MODE true  // false = SpreadCycle (STRONG/loud), true = StealthChop (weak/quiet)
#define GUIDE_MICROSTEPPING MICROSTEPS
#define USE_VREF 0  // 0 = UART control, 1 = VREF potentiometer control
#define R_CURRENT_PERCENT 100 // % (100% with R_SENSE=0.11 gives ~2A RMS current)
#define STALL_VALUE 100 // TMC2209 stall threshold (0-255, higher = more sensitive)

#endif // CONFIG_H
