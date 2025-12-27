#include "functions.h"

// Define the color constants
RgbColor red = RgbColor(255, 0, 0);
RgbColor green = RgbColor(0, 255, 0);
RgbColor white = RgbColor(255);
RgbColor black = RgbColor(0);

NeoPixelBus<NeoRgbFeature, NeoEsp32Rmt0Ws2812xMethod> *strip_info = NULL;
float info_led_brightness = 0.3; // Default brightness (30% to avoid being too bright)

void functions_setup()
{
	if (!LittleFS.begin())
	{
		LOG_ERROR("Failed to mount file system");
		infoLedError(); // Show error indication
		delay(500);
		LittleFS.format();
		infoLedBusy(); // Show formatting in progress
		delay(300);
	}
	else
	{
		// LOG_ATTACH_FS_AUTO(LittleFS, "/log.txt", FILE_WRITE);
	}
}

void ChangeNeoPixels_info() // this set the number of leds of the strip based on web configuration
{
	if (strip_info != NULL)
	{
		delete strip_info; // delete the previous dynamically created strip
	}
	strip_info = new NeoPixelBus<NeoRgbFeature, NeoEsp32Rmt0Ws2812xMethod>(1, INFO_DATA_PIN); // and recreate with new count
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
	strip_info->SetPixelColor(0, black);
	strip_info->Show();
}

void infoLedFadeIn(RgbColor color, uint16_t duration)
{
	if (strip_info == NULL)
	{
		ChangeNeoPixels_info();
	}

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
	RgbColor dim_blue = applyBrightness(RgbColor(0, 0, 255), info_led_brightness * 0.3);
	strip_info->SetPixelColor(0, dim_blue);
	strip_info->Show();
}

void infoLedBusy()
{
	if (strip_info == NULL)
	{
		ChangeNeoPixels_info();
	}
	infoLedPulse(RgbColor(255, 165, 0), 1, 1000); // Orange pulse
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
	LittleFS.format();
	// WiFi.disconnect(false, true);
	infoLedPulse(RgbColor(255, 0, 255), 2, 400); // Magenta pulses before restart
	ESP.restart();
}

void resetESP()
{
	infoLight(RgbColor(255, 165, 0)); // Orange for restart
	blinkLed(3, 200);
	delay(500);
	infoLedFadeOut(500); // Smooth fade out before restart
	delay(500);
	ESP.restart();
}

// Generic JSON file helpers
bool readJsonFile(const char* path, JsonDocument &doc)
{
	if (!LittleFS.exists(path))
	{
		LOG_DEBUG("readJsonFile: file not found", path);
		return false;
	}
	File file = LittleFS.open(path, "r");
	if (!file)
	{
		LOG_DEBUG("readJsonFile: failed to open", path);
		return false;
	}
	size_t size = file.size();
	if (size == 0)
	{
		LOG_DEBUG("readJsonFile: empty file", path);
		file.close();
		return false;
	}
	// Read into string to avoid streaming issues
	String content = file.readString();
	file.close();
	DeserializationError err = deserializeJson(doc, content);
	if (err)
	{
		LOG_DEBUG("readJsonFile: failed to parse", path);
		return false;
	}
	return true;
}

bool writeJsonFile(const char* path, JsonDocument &doc)
{
	File file = LittleFS.open(path, "w");
	if (!file)
	{
		LOG_DEBUG("failed to open for write", path);
		return false;
	}
	if (serializeJson(doc, file) == 0)
	{
		LOG_DEBUG("failed to write json", path);
		file.close();
		return false;
	}
	file.close();
	return true;
}
