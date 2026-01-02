#pragma once
#include <Arduino.h>
#include <WebServer.h>
#include <LittleFS.h>

#include "custom_log.h"

void setup_file(WebServer& server_instance);
void handleFSInfo();
String formatBytes(size_t bytes);
void handleFileUpload();
void handleFileList();
void handleDeletePage();
void handleDeleteFile();
void handleDownloadPage();
void handleFileDownload();
