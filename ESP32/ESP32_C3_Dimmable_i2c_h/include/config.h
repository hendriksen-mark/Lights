#ifndef CONFIG_H
#define CONFIG_H

#pragma once

// ============================================
// General Configuration
// ============================================
#ifndef DEVICE_NAME
#define DEVICE_NAME "ESP32_Light"
#endif
#ifndef INFO_DATA_PIN
#define INFO_DATA_PIN 12               // Data pin for info LED strip
#endif

// ============================================
// WiFi Configuration
// ============================================
#define WIFI_CHECK_INTERVAL 30000  // Check WiFi every 30 seconds (millis)

// ============================================
// Ethernet Configuration
// ============================================
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
#define LIGHT_PROTOCOL_WS "native_multi"
#define LIGHT_MODEL_WS "LST002"
#define LIGHT_TYPE_WS "ws2812_strip"
#define LIGHT_NAME_MAX_LENGTH 32        // Longer name will get stripped
#define LIGHT_PORT_WS 81                // Web server port for WS2811 lights
#define ENTERTAINMENT_TIMEOUT 1500      // Entertainment stream timeout (millis)
#define LIGHT_COUNT_WS 9              // Number of WS2811 lights
#define PIXEL_COUNT_WS 9             // Number of pixels per WS2811 light
#define TRANSITION_LEDS_WS 0           // Number of transition leds between virtual lights
#define LIGHT_ONPIN_WS 8              // Hardware on switch pin
#define LIGHT_OFFPIN_WS 9             // Hardware off switch pin
#ifndef DATA_PIN
#define DATA_PIN 4                      // WS2811 data pin
#endif
// #define POWER_MOSFET_PIN 35          // WS2812 consume ~1mA/led when off. By installing a MOSFET it will cut the power to the leds when lights are off.

// ============================================
// I2C Dimmable Lights Configuration
// ============================================
#define LIGHT_NAME_I2C "Dimmable Hue Light ESP32"  // Default light name
#define LIGHT_VERSION_I2C 2.1
#define LIGHT_PROTOCOL_I2C "native_multi"
#define LIGHT_MODEL_I2C "LWB010"
#define LIGHT_TYPE_I2C "dimmable_light"
#define LIGHT_PORT_I2C 80               // Web server port for I2C lights
#define LIGHT_COUNT_I2C 7              // Number of I2C lights
#define LIGHT_INTERVAL 60000            // Light interval (millis)

// ============================================
// Mesh Network Configuration
// ============================================
#define MESH_PREFIX     "HomeMesh"
#define MESH_PASSWORD   "Qwertyuiop1"
#define MESH_PORT       5555
#define MESH_CONNECT_MODE     WIFI_AP_STA
#define MESH_CHANNEL    1
#define MESH_HIDDEN     true
#define PORDIJN_SERVER_PORT    82
#define MESH_SERVER_PORT       83

// Bridge IP Configuration
#define BRIDGE_IP_OCTET_1   192
#define BRIDGE_IP_OCTET_2   168
#define BRIDGE_IP_OCTET_3   1
#define BRIDGE_IP_OCTET_4   25      // Default last octet (can be changed via web interface)
#define BRIDGE_PORT         80

// ============================================
// Sensor Types Configuration
// ============================================
#define switchType "ZLLSwitch"
#define motionType "ZLLPresence"

#endif // CONFIG_H
