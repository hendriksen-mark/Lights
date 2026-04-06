#ifndef CONFIG_H
#define CONFIG_H

#pragma once

#define DEVICE_NAME "curtain"

#define MESH_PREFIX "HomeMesh"
#define MESH_PASSWORD "Qwertyuiop1"
#define MESH_PORT 5555

#define REQUEST_TIMEOUT 2000

#define LED_PIN 8 // Define the GPIO for the LED on SuperMini

#define HOME_SWITCH 4

#define TOTALROND 123l

#define DIR 6
#define STEP 7
#define ENABLE 21
#define DRIVER_ADDRESS_1 0b00 // Left motor driver - MS1/MS2 = LOW/LOW
#define DRIVER_ADDRESS_2 0b01 // Right motor driver - MS1/MS2 = HIGH/LOW
#define R_SENSE 0.11f
#define SW_RX 9
#define SW_TX 8
#define INVERT_DIR true
#define INVERT_STEP false
#define INVERT_ENABLE false

#define MOTOR_STEPS 200l
#define MICROSTEPS 16l
#define MOTORSPEED 625 //*16=10000 // speed voor full step
#define MOTORACC 400   //*16=3200 // Acceleration voor full step

#define UART_STEALTH_MODE true
#define GUIDE_MICROSTEPPING MICROSTEPS
#define R_CURRENT 1500 // mA
#define STALL_VALUE 100 // TMC2209 stall threshold (0-255, higher = more sensitive)

#endif // CONFIG_H
