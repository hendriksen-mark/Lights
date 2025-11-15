#pragma once

#include <Wire.h>
#include <ArduinoJson.h>
#include "debug.h"
#include "ethernet_server.h"

#define I2C_DEV_ADDR 0x55

void handleNotFound_i2c();
void apply_scene_i2c(uint8_t new_scene,  uint8_t light);
void send_alert(uint8_t light);
void process_lightdata_i2c(uint8_t light);
void lightEngine_i2c();
void request_lightdata(uint8_t light);
void i2c_setup();
void i2c_loop();
