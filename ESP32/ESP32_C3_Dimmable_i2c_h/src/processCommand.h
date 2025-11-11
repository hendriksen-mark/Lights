#pragma once

#include <ArduinoJson.h>
#include "debug.h"
#include "server.h"


String sendHttpRequest(int button, String mac, IPAddress bridgeIp);