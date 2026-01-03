#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include "fs_wrapper.h"

#include "custom_log.h"
#include "config.h"

void setup_file(WebServer& server_instance);
void handleFSInfo();
String formatBytes(size_t bytes);
void handleFileUpload();
void handleFileList();
void handleDeletePage();
void handleDeleteFile();
void handleDownloadPage();
void handleFileDownload();

#endif // FILE_MANAGER_H
