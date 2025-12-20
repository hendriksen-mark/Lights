#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// General Configuration
// ============================================
#define INFO_DATA_PIN 12               // Data pin for info LED strip

// ============================================
// WiFi Configuration
// ============================================
#define WIFI_CHECK_INTERVAL 30000  // Check WiFi every 30 seconds (millis)

// MAC Address Configuration
#define MAC_ADDR_0  0xDA
#define MAC_ADDR_1  0xAD
#define MAC_ADDR_2  0xEB
#define MAC_ADDR_3  0xFF
#define MAC_ADDR_4  0xEF
#define MAC_ADDR_5  0xDE

// ============================================
// WS2811 LED Strip Configuration
// ============================================
#define LIGHT_NAME_WS2811 "Hue WS2811 strip"  // Default light name
#define LIGHT_VERSION 4.1
#define light_protocol_ws "native_multi"
#define light_model_ws "LST002"
#define light_type_ws "ws2812_strip"
#define LIGHT_NAME_MAX_LENGTH 32        // Longer name will get stripped
#define light_port_ws 81                // Web server port for WS2811 lights
#define ENTERTAINMENT_TIMEOUT 1500      // Entertainment stream timeout (millis)
#define DATA_PIN 4                      // WS2811 data pin
// #define POWER_MOSFET_PIN 35          // WS2812 consume ~1mA/led when off. By installing a MOSFET it will cut the power to the leds when lights are off.

// ============================================
// I2C Dimmable Lights Configuration
// ============================================
#define light_name_i2c "Dimmable Hue Light ESP32"  // Default light name
#define LIGHT_VERSION_i2c 2.1
#define light_protocol_i2c "native_multi"
#define light_model_i2c "LWB010"
#define light_type_i2c "dimmable_light"
#define light_port_i2c 80               // Web server port for I2C lights
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
#define gordijn_port    82
#define mesh_port       83

// Bridge IP Configuration
#define BRIDGE_IP_OCTET_1   192
#define BRIDGE_IP_OCTET_2   168
#define BRIDGE_IP_OCTET_3   1
#define BRIDGE_IP_OCTET_4   25      // Default last octet (can be changed via web interface)
#define BRIDGE_PORT        80

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
