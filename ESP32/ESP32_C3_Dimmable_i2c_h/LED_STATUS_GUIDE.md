# Info LED Status Guide

This document describes all the LED status indicators used by the ESP32 device.

## Overview

The device uses a single RGB LED (info LED) to provide visual feedback about system state, network connectivity, and service activity. The LED is controlled via the `INFO_DATA_PIN` (default: GPIO 12).

## Configuration

- **Default Brightness**: 30% (configurable via `setInfoLedBrightness()`)
- **LED Type**: WS2812/WS2811 RGB LED
- **Control Method**: NeoPixelBus library

---

## System States

### 🔵 Idle State
- **Color**: Dim Blue (RGB: 0, 0, 255 @ 9% brightness)
- **Pattern**: Solid
- **Meaning**: System is running normally, waiting for activity
- **Trigger**: After successful startup and setup completion
- **Function**: `infoLedIdle()`

### ⚪ Startup
- **Color**: White (RGB: 255, 255, 255)
- **Pattern**: Smooth fade-in (500ms)
- **Meaning**: Device is booting up
- **Trigger**: During `setup()` function
- **Function**: `infoLedFadeIn(white, 500)`

---

## Network & Connectivity

### 🔴 Network Connecting
- **Color**: Red (RGB: 255, 0, 0)
- **Pattern**: Pulse/breathing (500ms per pulse)
- **Meaning**: Attempting to connect to network (Ethernet/WiFi)
- **Trigger**: During initial network connection attempt
- **Function**: `infoLedPulse(red, 1, 500)`

### 🟢 Network Connected
- **Color**: Green (RGB: 0, 255, 0)
- **Pattern**: Two quick pulses (400ms total)
- **Meaning**: Successfully connected to network
- **Trigger**: After network connection established
- **Function**: `infoLedSuccess()`

### 🟠 WiFi Reconnecting
- **Color**: Orange (RGB: 255, 100, 0)
- **Pattern**: Single pulse (300ms)
- **Meaning**: WiFi connection lost, attempting to reconnect
- **Trigger**: When WiFi disconnect detected in main loop
- **Function**: `infoLedPulse(RgbColor(255, 100, 0), 1, 300)`

---

## Service Activity (Color-Coded by Service)

### 🔵 WS2811 Service Request
- **Color**: Cyan (RGB: 0, 255, 255)
- **Pattern**: Brief flash during request processing
- **Meaning**: WS2811 LED strip control request being processed
- **Trigger**: HTTP PUT request to `/state` on WS2811 server (port 81)
- **Function**: `infoLight(RgbColor(0, 255, 255))`

### 🟡 I2C Service Request
- **Color**: Yellow (RGB: 255, 255, 0)
- **Pattern**: Brief flash during request processing
- **Meaning**: I2C light control request being processed
- **Trigger**: HTTP PUT request to `/state` on I2C server (port 80)
- **Function**: `infoLight(RgbColor(255, 255, 0))`

### 🟢 Mesh Network Message
- **Color**: Green (RGB: 0, 255, 0)
- **Pattern**: Brief flash during message processing
- **Meaning**: Mesh network message received
- **Trigger**: Mesh callback when message arrives
- **Function**: `infoLight(RgbColor(0, 255, 0))`

### 🟣 Entertainment Mode Active
- **Color**: Purple (RGB: 128, 0, 128)
- **Pattern**: Pulse every 500ms
- **Meaning**: UDP entertainment stream is active (e.g., Ambilight, music sync)
- **Trigger**: While receiving entertainment UDP packets
- **Duration**: Continues until 1.5s timeout or manual stop
- **Function**: `infoLight(RgbColor(128, 0, 128))`

---

## Status Indicators

### 🟢 Success
- **Color**: Green (RGB: 0, 255, 0)
- **Pattern**: Two quick pulses (400ms total)
- **Meaning**: Operation completed successfully
- **Uses**:
  - Network connection established
  - WiFi reconnection successful
  - Command execution successful
- **Function**: `infoLedSuccess()`

### 🔴 Error
- **Color**: Red (RGB: 255, 0, 0)
- **Pattern**: Three fast blinks (100ms interval)
- **Meaning**: Error or failure occurred
- **Uses**:
  - File system mount failure
  - I2C communication errors (NO ACK, buffer overflow)
  - Network connection failures
  - Unknown device errors
- **Function**: `infoLedError()`

### 🟠 Busy/Processing
- **Color**: Orange (RGB: 255, 165, 0)
- **Pattern**: Single pulse (1000ms)
- **Meaning**: System is busy with a long operation
- **Uses**:
  - File system formatting in progress
  - Factory reset in progress
- **Function**: `infoLedBusy()`

---

## Special Operations

### 🔴 Factory Reset
- **Sequence**:
  1. Solid Red (500ms warning)
  2. 8 fast blinks (100ms interval)
  3. Orange pulse (formatting)
  4. Two Magenta pulses (RGB: 255, 0, 255) before restart
- **Meaning**: Factory reset in progress - all settings will be erased
- **Trigger**: Factory reset command
- **Function**: `factoryReset()`

### 🟠 System Restart
- **Sequence**:
  1. Solid Orange
  2. 3 blinks (200ms interval)
  3. Smooth fade out (500ms)
  4. Device restarts
- **Meaning**: System is restarting
- **Trigger**: Reset command
- **Function**: `resetESP()`

### 🔴 File System Error & Format
- **Sequence**:
  1. Red error indication (3 blinks)
  2. 500ms pause
  3. Orange busy pulse (formatting)
  4. 300ms pause
- **Meaning**: File system failed to mount and is being formatted
- **Trigger**: LittleFS.begin() failure during startup
- **Function**: Multiple calls in `functions_setup()`

---

## LED Control Functions

### Basic Control
- `infoLight(RgbColor color)` - Set LED to solid color (with brightness applied)
- `infoLedOff()` - Turn LED off
- `setInfoLedBrightness(float brightness)` - Adjust brightness (0.0 - 1.0)

### Animations
- `infoLedFadeIn(RgbColor color, uint16_t duration)` - Smooth fade in
- `infoLedFadeOut(uint16_t duration)` - Smooth fade out
- `infoLedPulse(RgbColor color, uint8_t pulses, uint16_t duration)` - Breathing effect
- `blinkLed(uint8_t count, uint16_t interval)` - Simple on/off blinks

### Status Helpers
- `infoLedIdle()` - Enter idle state (dim blue)
- `infoLedBusy()` - Show busy indicator (orange pulse)
- `infoLedSuccess()` - Show success (2 green pulses)
- `infoLedError()` - Show error (3 red blinks)

---

## Quick Reference Table

| State | Color | Pattern | Meaning |
|-------|-------|---------|---------|
| **Idle** | Dim Blue | Solid | Normal operation |
| **Startup** | White | Fade in | Booting |
| **Network Connecting** | Red | Pulse | Connecting to network |
| **Connected** | Green | 2 pulses | Network ready |
| **WiFi Issue** | Orange | Pulse | Reconnecting WiFi |
| **WS2811 Request** | Cyan | Flash | LED strip command |
| **I2C Request** | Yellow | Flash | I2C light command |
| **Mesh Message** | Green | Flash | Mesh data received |
| **Entertainment** | Purple | Pulse (500ms) | UDP stream active |
| **Success** | Green | 2 pulses | Operation OK |
| **Error** | Red | 3 blinks | Error occurred |
| **Busy** | Orange | Pulse | Processing |
| **Factory Reset** | Red → Magenta | Complex | Resetting device |
| **Restart** | Orange | Fade out | Rebooting |

---

## Troubleshooting

### LED Not Working
1. Check `INFO_DATA_PIN` configuration in `config.h` or `platformio.ini`
2. Verify LED is connected to correct GPIO pin
3. Ensure WS2812/WS2811 LED is properly powered
4. Check if `ChangeNeoPixels_info()` is called during setup

### LED Too Bright/Dim
- Adjust brightness: `setInfoLedBrightness(0.5)` for 50% brightness
- Default is 30% to avoid excessive brightness
- Valid range: 0.0 (off) to 1.0 (maximum)

### LED Shows Wrong Colors
- Ensure using WS2812-compatible RGB LED (not RGBW)
- Check color order matches your LED type (RGB vs GRB)
- Verify NeoPixelBus method matches your LED model

---

## Technical Details

### Auto-Initialization
All LED functions automatically initialize the LED hardware if not already initialized. If `strip_info` is NULL, the function calls `ChangeNeoPixels_info()` automatically.

### Brightness Control
Brightness is applied automatically via `applyBrightness()` helper function. This ensures consistent brightness across all color operations while preserving color accuracy.

### Performance
- LED updates use hardware RMT channel (ESP32 RMT0)
- Non-blocking operation (except during fade animations)
- Minimal impact on main loop performance
- Entertainment mode updates throttled to 500ms intervals

---

**Last Updated**: December 26, 2025  
**Compatible with**: ESP32-C3, ESP32-S3, ESP32 WROVER modules
