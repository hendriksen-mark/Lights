#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// WiFi Configuration
// ============================================
#define WIFI_CHECK_INTERVAL 30000  // Check WiFi every 30 seconds (millis)

// ============================================
// WS2811 LED Strip Configuration
// ============================================
#define LIGHT_NAME_WS2811 "Hue WS2811 strip"  // Default light name
#define LIGHT_VERSION 4.1
#define LIGHT_NAME_MAX_LENGTH 32        // Longer name will get stripped
#define ENTERTAINMENT_TIMEOUT 1500      // Entertainment stream timeout (millis)
#define DATA_PIN 4                      // WS2811 data pin
// #define POWER_MOSFET_PIN 35          // WS2812 consume ~1mA/led when off. By installing a MOSFET it will cut the power to the leds when lights are off.

// ============================================
// I2C Dimmable Lights Configuration
// ============================================
#define light_name_i2c "Dimmable Hue Light ESP32"  // Default light name
#define LIGHT_VERSION_i2c 2.1
#define LIGHTS_COUNT_i2c 7              // Number of I2C lights
#define LIGHT_interval 60000            // Light interval (millis)

// ============================================
// Mesh Network Configuration
// ============================================
#define MESH_PREFIX     "HomeMesh"
#define MESH_PASSWORD   "Qwertyuiop1"
#define MESH_PORT       5555
#define MESH_CONNECT_MODE     WIFI_AP_STA
#define MESH_CHANNEL    1
#define MESH_HIDDEN     true

// Bridge IP Configuration
#define BRIDGE_IP_OCTET_1   192
#define BRIDGE_IP_OCTET_2   168
#define BRIDGE_IP_OCTET_3   1
#define BRIDGE_IP_OCTET_4   25      // Default last octet (can be changed via web interface)

// ============================================
// Stepper Motor Configuration
// ============================================
#define RES 2                           // Stepper resolution
#define DIR 8                           // Direction pin
#define STEP 7                          // Step pin
#define ENABLE 4                        // Enable pin
#define DRIVER_ADDRESS "0b00"           // Set by MS1/MS2. LOW/LOW in this case
#define totalrond 123                   // Total rounds

// ============================================
// Serial Communication Configuration
// ============================================
#define ESP_SW_RX "A0"
#define ESP_SW_TX "A1"
#define DEBUG "false"

// ============================================
// Sensor Types Configuration
// ============================================
#define switchType "ZLLSwitch"
#define motionType "ZLLPresence"

#endif // CONFIG_H
