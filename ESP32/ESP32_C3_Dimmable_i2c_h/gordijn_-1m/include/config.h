#define ESP_SW_RX            A0
#define ESP_SW_TX            A1

#define DEBUG true

#define home_switch 2

#define totalrond 123l

#define DIR 13//8
#define STEP 12//7
#define ENABLE 9//4
#define DRIVER_ADDRESS 0b00  // Set by MS1/MS2. LOW/LOW in this case
#define R_SENSE 0.11f
#define SW_RX            11//6
#define SW_TX            10//5
#define RA_INVERT_DIR 1

#define MOTOR_STEPS 200l
#define MICROSTEPS 16l
#define motorSpeed 625//*16=10000 // speed voor full step
#define motorAcc 400//*16=3200 // Acceleration voor full step

#define UART_STEALTH_MODE 1
#define GUIDE_MICROSTEPPING       MICROSTEPS
#define R_current 1500//mA
