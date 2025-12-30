#pragma once

#include <ArduinoJson.h>
#include <WiFi.h>

#include "config.h"
#include "debug.h"
#include "ethernet_server.h"

String sendHttpRequest(int button, String mac, IPAddress bridgeIp, int bridgePort);
