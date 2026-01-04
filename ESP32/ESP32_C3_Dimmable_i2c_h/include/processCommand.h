#ifndef PROCESS_COMMAND_H
#define PROCESS_COMMAND_H

#pragma once

#include <ArduinoJson.h>
#include <WiFi.h>

#include "config.h"
#include "custom_log.h"
#include "ethernet_server.h"

String sendHttpRequest(int button, String mac, IPAddress bridgeIp, int bridgePort);

#endif // PROCESS_COMMAND_H
