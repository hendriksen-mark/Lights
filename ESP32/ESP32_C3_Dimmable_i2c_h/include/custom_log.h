#ifndef CUSTOM_LOG_H
#define CUSTOM_LOG_H

#pragma once

#include "log_server.h"

#define LOG_PREAMBLE (isNTPTimeValid() ? getFormattedDateTime() : ("+" + String(millis() / 1000) + "s")), LOG_SHORT_FILENAME, LOG_MACRO_APPEND_STR(L.__LINE__), __func__, ":"

#include <Arduino.h>
#include <DebugLog.h>

// Helper function to build log messages
void sendRemoteLog(const char *level, const String &message, const char *file, int line, const char *function);

// Macros that use the original DebugLog and also send to remote clients (include file:line:function)
#define REMOTE_LOG_INFO(...)                                                                   \
	do                                                                                         \
	{                                                                                          \
		LOG_INFO(__VA_ARGS__);                                                                 \
		sendRemoteLog("INFO", buildLogMessage(__VA_ARGS__), __FILE__, __LINE__, __FUNCTION__); \
		LOG_FILE_FLUSH();                                                                      \
	} while (0)
#define REMOTE_LOG_DEBUG(...)                                                                   \
	do                                                                                          \
	{                                                                                           \
		LOG_DEBUG(__VA_ARGS__);                                                                 \
		sendRemoteLog("DEBUG", buildLogMessage(__VA_ARGS__), __FILE__, __LINE__, __FUNCTION__); \
		LOG_FILE_FLUSH();                                                                       \
	} while (0)
#define REMOTE_LOG_WARN(...)                                                                   \
	do                                                                                         \
	{                                                                                          \
		LOG_WARN(__VA_ARGS__);                                                                 \
		sendRemoteLog("WARN", buildLogMessage(__VA_ARGS__), __FILE__, __LINE__, __FUNCTION__); \
		LOG_FILE_FLUSH();                                                                      \
	} while (0)
#define REMOTE_LOG_ERROR(...)                                                                   \
	do                                                                                          \
	{                                                                                           \
		LOG_ERROR(__VA_ARGS__);                                                                 \
		sendRemoteLog("ERROR", buildLogMessage(__VA_ARGS__), __FILE__, __LINE__, __FUNCTION__); \
		LOG_FILE_FLUSH();                                                                       \
	} while (0)

// Variadic template to build messages from arbitrary arguments, inserting a single space between items
template <typename... Args>
String buildLogMessage(Args &&...args)
{
	String s;
	int idx = 0;
	int dummy[] = {0, ((idx++ > 0 ? ((s += " "), 0) : 0), s += String(std::forward<Args>(args)), 0)...};
	(void)dummy;
	return s;
}

#endif // CUSTOM_LOG_H
