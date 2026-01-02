#ifndef ETHERNET_SERVER_H
#define ETHERNET_SERVER_H

#pragma once

#include <DNSServer.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>

#include "custom_log.h"
#include "functions.h"
#include "config.h"

// Forward declarations only - no W5500 includes in header
void ESP_Server_setup();
void ota_setup();
void ethernet_loop();
String get_mac_address();

#endif // ETHERNET_SERVER_H
