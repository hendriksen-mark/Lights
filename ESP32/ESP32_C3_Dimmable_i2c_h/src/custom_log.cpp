#include "custom_log.h"

// Send log message to remote clients (includes source file, line and function)
void sendRemoteLog(const char *level, const String &message, const char *file, int line, const char *function)
{
	if (getConnectedLogClientCount() > 0)
	{
		// Extract basename from file path
		const char *base = file;
		const char *p1 = strrchr(file, '/');
		const char *p2 = strrchr(file, '\\');
		if (p1 || p2)
		{
			const char *p = p1 > p2 ? p1 : p2;
			base = p + 1;
		}

		// Format: [LEVEL] filename L.<line> [function] : message
		// If function is the lambda operator() then omit it for readability
		String src = String(base) + " L." + String(line);
		if (function && strcmp(function, "operator()") != 0)
		{
			src += " ";
			src += String(function);
		}
		src += " : ";
		String formattedMessage = String("[") + level + "] " + src + message;
		logToRemoteClients(formattedMessage.c_str());
	}
}
// (Old specific overloads removed — header variadic template `buildLogMessage` is used)
