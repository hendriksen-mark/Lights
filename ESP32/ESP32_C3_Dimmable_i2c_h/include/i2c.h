#ifndef I2C_H
#define I2C_H

#pragma once

#include <Wire.h>
#include <ArduinoJson.h>
#include <WebServer.h>

#include "config.h"
#include "custom_log.h"
#include "ethernet_server.h"
#include "i2c_http_content.h"

void handleNotFound_i2c();
void apply_scene_i2c(uint8_t new_scene, uint8_t light);
void send_alert(uint8_t light);
void process_lightdata_i2c(uint8_t light);
void request_lightdata(uint8_t light);
void i2c_setup();
void i2c_loop();

// Persistence for I2C lights
bool saveState_i2c();
bool restoreState_i2c();
bool saveConfig_i2c();
bool loadConfig_i2c();

#endif // I2C_H
