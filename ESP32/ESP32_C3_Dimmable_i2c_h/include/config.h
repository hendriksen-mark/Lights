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
#define INFO_DATA_PIN 12 // Data pin for info LED strip
#endif
#define LOG_FILE_NAME "/log.txt"
#define LOG_MAX_FILE_SIZE_BYTES (1300 * 1024) // ~1.30 MB
#define MESH_CONFIG_PATH "/mesh_config.json"
#define WS_CONFIG_PATH "/ws_config.json"
#define WS_STATE_PATH "/ws_state.json"
#define I2C_CONFIG_PATH "/i2c_config.json"
#define I2C_STATE_PATH "/i2c_state.json"
#define MAX_UPLOAD_SIZE (300 * 1024)

// ============================================
// LOG Server Configuration
// ============================================
#define LOG_SERVER_PORT     2001
#define MAX_LOG_CONNECTIONS 1   // Log clients (reduced to fit socket limit)
#define LOG_CLIENT_TIMEOUT  60000  // 60 seconds in milliseconds (longer for log clients)

// ============================================
// NTP Time Configuration
// ============================================
#define NTP_ENABLED         true            // Enable NTP time synchronization
#define NTP_SERVER          "pool.ntp.org"  // NTP server to use
#define NTP_UPDATE_INTERVAL 3600000         // Update every hour (3600 seconds)
#define NTP_TIMEZONE_OFFSET 1               // UTC+1 for Netherlands (CET)
#define NTP_DST_OFFSET      1               // Additional hour for DST (CEST)
#define NTP_TIMEOUT         10000           // 10 second timeout for NTP requests


// ============================================
// Ethernet Configuration
// ============================================
#ifndef W5500_INT_GPIO
#define W5500_INT_GPIO 9 // GPIO pin for W5500 interrupt
#endif

// ============================================
// WS2811 LED Strip Configuration
// ============================================
#define LIGHT_NAME_WS2811 "Hue WS2811 strip" // Default light name
#define LIGHT_VERSION 4.1
#define LIGHT_PROTOCOL_WS "native_multi"
#define LIGHT_MODEL_WS "LST002"
#define LIGHT_TYPE_WS "ws2812_strip"
#define LIGHT_NAME_MAX_LENGTH 32   // Longer name will get stripped
#define LIGHT_PORT_WS 81           // Web server port for WS2811 lights
#define ENTERTAINMENT_TIMEOUT 1500 // Entertainment stream timeout (millis)
#define LIGHT_COUNT_WS 9           // Number of WS2811 lights
#define PIXEL_COUNT_WS 9           // Number of pixels per WS2811 light
#define TRANSITION_LEDS_WS 0       // Number of transition leds between virtual lights
#define TRANSITION_FRAME_MS_DEFAULT 6 // Default ms between transition frames
#ifndef DATA_PIN
#define DATA_PIN 4 // WS2811 data pin
#endif
// Maximum allowed runtime virtual lights to protect heap on embedded device
#ifndef MAX_RUNTIME_LIGHTS
#define MAX_RUNTIME_LIGHTS 10
#endif

// ============================================
// I2C Dimmable Lights Configuration
// ============================================
#define LIGHT_NAME_I2C "Dimmable Hue Light ESP32" // Default light name
#define LIGHT_VERSION_I2C 2.1
#define LIGHT_PROTOCOL_I2C "native_multi"
#define LIGHT_MODEL_I2C "LWB010"
#define LIGHT_TYPE_I2C "dimmable_light"
#define LIGHT_PORT_I2C 80    // Web server port for I2C lights
#define LIGHT_COUNT_I2C 7    // Number of I2C lights
#define LIGHT_INTERVAL 60000 // Light interval (millis)

// ============================================
// Mesh Network Configuration
// ============================================
#define MESH_PREFIX "HomeMesh"
#define MESH_PASSWORD "Qwertyuiop1"
#define MESH_PORT 5555
#define MESH_CONNECT_MODE WIFI_AP_STA
#define MESH_CHANNEL 1
#define MESH_HIDDEN true
#define PORDIJN_SERVER_PORT 82

#define BRIDGE_IP "192.168.1.25" // DIYhue Bridge IP as a string; last octet can be changed via web interface
#define BRIDGE_PORT 80 // Default DIYhue Bridge port (can be changed via web interface)

// ============================================
// Sensor Types Configuration
// ============================================
#define switchType "ZLLSwitch"
#define motionType "ZLLPresence"

#endif // CONFIG_H
