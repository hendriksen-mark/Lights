#pragma once

#include <ArduinoJson.h>
#include <WiFi.h>

#include "config.h"
#include "custom_log.h"
#include "ethernet_server.h"

String sendHttpRequest(int button, String mac, IPAddress bridgeIp, int bridgePort);
