#ifndef MESH_H
#define MESH_H

#pragma once

#include <painlessMesh.h>
#include <ArduinoJson.h>
#include <WebServer.h>

#include "config.h"
#include "custom_log.h"
#include "processCommand.h"
#include "mesh_http_content.h"

void mesh_setup();
void mesh_loop();
void send_change();
void newConnectionCallback(uint32_t nodeId);
void receivedCallback(uint32_t from, String &msg);
void sendData(String msg);
void set_Target_Pos_test();
void set_Target_Pos();
void homeing();
void get_current_pos_test();
void get_current_pos();
void get_target_pos_test();
void get_target_pos();
void get_state_test();
void get_state();
void handleinfo();
void handleNotFound();
void set_IP();
void set_PORT();
void discoverBridgeMdns();
bool saveConfig_mesh();
bool loadConfig_mesh();
void pollCurtainStatus();
void sendCurtainCommand(bool homing, bool request, int targetPos, const char *logPath);
void sendResponse(bool isRedirect, const String &content);

#endif // MESH_H
