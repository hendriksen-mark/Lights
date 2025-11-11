#pragma once

#include <WebServer_ESP32_W5500.h> //https://github.com/khoih-prog/WebServer_ESP32_W5500
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include "debug.h"
#include "functions.h"

void ESP_Server_setup();