#ifndef PROCESS_COMMAND_H
#define PROCESS_COMMAND_H

#pragma once

#include <ArduinoJson.h>
#include <HTTPClient.h>

#include "config.h"
#include "custom_log.h"
#include "ethernet_server.h"

void sendHttpRequest(int button, String mac, IPAddress bridgeIp, int bridgePort);

#endif // PROCESS_COMMAND_H
