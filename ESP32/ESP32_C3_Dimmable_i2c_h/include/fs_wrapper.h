#ifndef FS_WRAPPER_H
#define FS_WRAPPER_H

#pragma once

#include <Arduino.h>
#include <FS.h>

#include "custom_log.h"

#if defined(USE_LITTLEFS)
#include <LittleFS.h>
#endif

#if defined(USE_SD)
#include <SPI.h>
#include <SD.h>
#endif

enum FsType
{
    FS_NONE = 0,
    FS_LITTLEFS,
    FS_SD
};

// Set preferred filesystem for initialization (runtime preference)
void fs_set_preferred(FsType t);
FsType fs_get_preferred();
FsType fs_get_current();
String fs_get_name();

// Initialize selected filesystem. If both are available, will try preferred then fallback.
bool fs_begin();

// Basic FS operations
bool fs_exists(const char *path);
bool fs_exists(const String &path);
File fs_open(const char *path, const char *mode);
File fs_open(const String &path, const char *mode);
bool fs_remove(const char *path);
bool fs_remove(const String &path);
bool fs_mkdir(const char *path);
bool fs_mkdir(const String &path);
bool fs_format();
size_t fs_usedBytes();
size_t fs_totalBytes();
File fs_open_root();
void listDir(const char *dirname, uint8_t levels);

void log_file();

#endif // FS_WRAPPER_H
