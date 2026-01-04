#include "functions.h"

#define PRIMITIVE_CAT3(a, b, c) a##b##c
#define CAT3(a, b, c) PRIMITIVE_CAT3(a, b, c)
typedef CAT3(Neo, INFO_LED_ORDER, Feature) InfoLedColorFeature;

NeoPixelBus<InfoLedColorFeature, NeoEsp32Rmt0Ws2812xMethod> *strip_info = NULL;
float info_led_brightness = 0.3; // Default brightness (30% to avoid being too bright)

void serialWait()
{
	// Give user time to open serial monitor: print a heartbeat during the 10s wait
	const int waitMs = 10000;
	const int intervalMs = 500;
	int loops = waitMs / intervalMs;
	for (int i = 0; i < loops; ++i)
	{
		REMOTE_LOG_INFO("Waiting for serial monitor...");
		infoLedPulse(white, 1, intervalMs); // Pulse white while waiting for serial monitor
	}
}

void functions_setup()
{
	// Initialize filesystem (tries preferred if set, falls back)
	if (!fs_begin())
	{
		REMOTE_LOG_ERROR("Failed to initialize any filesystem");
		infoLedError();
		delay(500);
		fs_format();
		infoLedBusy();
		REMOTE_LOG_INFO("Filesystem formatted");
		delay(300);
	}
	else
	{
		REMOTE_LOG_INFO("Filesystem mounted:", fs_get_name());
		listDir("/", 0); // Log root directory contents
		log_file();
	}
}

void ChangeNeoPixels_info() // this set the number of leds of the strip based on web configuration
{
	if (strip_info != NULL)
	{
		delete strip_info; // delete the previous dynamically created strip
	}
	// Sanity check pin again before initializing RMT
	int infoPin = INFO_DATA_PIN;
	REMOTE_LOG_DEBUG("INFO_DATA_PIN=", infoPin);
	if (infoPin < 0 || infoPin > 47)
	{
		REMOTE_LOG_ERROR("ChangeNeoPixels_info: invalid INFO_DATA_PIN, aborting strip init");
		strip_info = NULL;
		return;
	}
	strip_info = new NeoPixelBus<InfoLedColorFeature, NeoEsp32Rmt0Ws2812xMethod>(1, INFO_DATA_PIN); // and recreate with new count
	strip_info->Begin();
}

// Helper function to apply brightness to a color
RgbColor applyBrightness(RgbColor color, float brightness)
{
	brightness = constrain(brightness, 0.0, 1.0);
	return RgbColor(
		(uint8_t)(color.R * brightness),
		(uint8_t)(color.G * brightness),
		(uint8_t)(color.B * brightness));
}

void setInfoLedBrightness(float brightness)
{
	info_led_brightness = constrain(brightness, 0.0, 1.0);
}

void blinkLed(uint8_t count, uint16_t interval)
{
	if (strip_info == NULL)
	{
		ChangeNeoPixels_info();
	}

	if (strip_info == NULL)
		return;

	RgbColor color = strip_info->GetPixelColor(0);
	for (uint8_t i = 0; i < count; i++)
	{
		strip_info->SetPixelColor(0, black);
		strip_info->Show();
		delay(interval);
		strip_info->SetPixelColor(0, color);
		strip_info->Show();
		delay(interval);
	}
}

void infoLight(RgbColor color)
{ // boot animation for leds count and wifi test
	if (strip_info == NULL)
	{
		ChangeNeoPixels_info();
	}

	if (strip_info == NULL)
		return;

	// Flash the strip in the selected color. White = booted, green = WLAN connected, red = WLAN could not connect
	RgbColor adjusted_color = applyBrightness(color, info_led_brightness);
	strip_info->SetPixelColor(0, adjusted_color);
	strip_info->Show();
}

void infoLedOff()
{
	if (strip_info == NULL)
	{
		ChangeNeoPixels_info();
	}

	if (strip_info == NULL)
		return;

	strip_info->SetPixelColor(0, black);
	strip_info->Show();
}

void infoLedFadeIn(RgbColor color, uint16_t duration)
{
	if (strip_info == NULL)
	{
		ChangeNeoPixels_info();
	}

	if (strip_info == NULL)
		return;

	uint8_t steps = 50;
	uint16_t stepDelay = duration / steps;

	for (uint8_t i = 0; i <= steps; i++)
	{
		float progress = (float)i / steps;
		RgbColor fade_color = applyBrightness(color, progress * info_led_brightness);
		strip_info->SetPixelColor(0, fade_color);
		strip_info->Show();
		delay(stepDelay);
	}
}

void infoLedFadeOut(uint16_t duration)
{
	if (strip_info == NULL)
	{
		ChangeNeoPixels_info();
	}

	if (strip_info == NULL)
		return;

	RgbColor current_color = strip_info->GetPixelColor(0);
	uint8_t steps = 50;
	uint16_t stepDelay = duration / steps;

	for (uint8_t i = steps; i > 0; i--)
	{
		float progress = (float)i / steps;
		RgbColor fade_color = RgbColor(
			(uint8_t)(current_color.R * progress),
			(uint8_t)(current_color.G * progress),
			(uint8_t)(current_color.B * progress));
		strip_info->SetPixelColor(0, fade_color);
		strip_info->Show();
		delay(stepDelay);
	}
	strip_info->SetPixelColor(0, black);
	strip_info->Show();
}

void infoLedPulse(RgbColor color, uint8_t pulses, uint16_t pulseDuration)
{
	if (strip_info == NULL)
	{
		ChangeNeoPixels_info();
	}

	if (strip_info == NULL)
		return;

	for (uint8_t p = 0; p < pulses; p++)
	{
		infoLedFadeIn(color, pulseDuration / 2);
		infoLedFadeOut(pulseDuration / 2);
		if (p < pulses - 1)
		{
			delay(pulseDuration / 4); // Short pause between pulses
		}
	}
}

// Status indication helpers
void infoLedIdle()
{
	if (strip_info == NULL)
	{
		ChangeNeoPixels_info();
	}
	RgbColor dim_blue = applyBrightness(blue, info_led_brightness * 0.3);
	strip_info->SetPixelColor(0, dim_blue);
	strip_info->Show();
}

void infoLedBusy()
{
	if (strip_info == NULL)
	{
		ChangeNeoPixels_info();
	}
	infoLedPulse(orange, 1, 1000); // Orange pulse
}

void infoLedSuccess()
{
	if (strip_info == NULL)
	{
		ChangeNeoPixels_info();
	}
	infoLedPulse(green, 2, 400); // Two quick green pulses
}

void infoLedError()
{
	if (strip_info == NULL)
	{
		ChangeNeoPixels_info();
	}
	infoLight(red);	  // Set to red first
	blinkLed(3, 100); // Three fast blinks
}

void factoryReset()
{
	infoLight(red); // Show red for factory reset warning
	delay(500);
	blinkLed(8, 100); // 8 fast blinks
	delay(300);
	infoLedBusy(); // Show formatting in progress
	// Emulate factory reset by removing all files from active FS
	fs_format();
	// WiFi.disconnect(false, true);
	infoLedPulse(magenta, 2, 400); // Magenta pulses before restart
	ESP.restart();
}

void resetESP()
{
	infoLight(orange); // Orange for restart
	blinkLed(3, 200);
	delay(500);
	fs_format();
	infoLedFadeOut(500); // Smooth fade out before restart
	delay(500);
	ESP.restart();
}

// Generic JSON file helpers
bool readJsonFile(const char *path, JsonDocument &doc)
{
	if (!fs_exists(path))
	{
		REMOTE_LOG_ERROR("readJsonFile: file not found", path);
		return false;
	}
	File file = fs_open(path, "r");
	if (!file)
	{
		REMOTE_LOG_ERROR("readJsonFile: failed to open", path);
		return false;
	}
	size_t size = file.size();
	if (size == 0)
	{
		REMOTE_LOG_ERROR("readJsonFile: empty file", path);
		file.close();
		return false;
	}
	// Read into string to avoid streaming issues
	String content = file.readString();
	file.close();
	// REMOTE_LOG_DEBUG("readJsonFile: content of file", path, String(content));
	DeserializationError err = deserializeJson(doc, content);
	if (err)
	{
		REMOTE_LOG_ERROR("readJsonFile: failed to parse", path);
		return false;
	}
	return true;
}

bool writeJsonFile(const char *path, JsonDocument &doc)
{
	File file = fs_open(path, "w");
	if (!file)
	{
		REMOTE_LOG_ERROR("failed to open for write", path);
		return false;
	}
	if (serializeJson(doc, file) == 0)
	{
		REMOTE_LOG_ERROR("failed to write json", path);
		file.close();
		return false;
	}
	file.close();
	return true;
}
