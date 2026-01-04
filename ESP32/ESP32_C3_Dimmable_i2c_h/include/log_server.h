#ifndef LOG_SERVER_H
#define LOG_SERVER_H

#pragma once

#include <Arduino.h>
#include <WiFi.h>

#include "custom_log.h"
#include "ntp_client.h"
#include "ethernet_server.h"
#include "config.h"

// Log server and client management
extern WiFiServer logServer;
extern WiFiClient logClients[MAX_LOG_CONNECTIONS];
extern unsigned long logClientLastActivity[MAX_LOG_CONNECTIONS];
extern bool logClientConnected[MAX_LOG_CONNECTIONS];

// Log statistics
extern unsigned long totalLogMessages;
extern unsigned long totalLogBytesSent;

// Function declarations
void initializeLogServer();
void handleNewLogConnections();
void sendToAllLogClients(const String &logMessage);
void handleLogClientCommunication();
void cleanupLogClients();
int getConnectedLogClientCount();
void logToRemoteClients(const char *message);

#endif // LOG_SERVER_H
