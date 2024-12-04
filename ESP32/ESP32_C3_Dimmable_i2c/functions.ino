#include <NeoPixelBus.h>

#define INFO_DATA_PIN 12

RgbColor red = RgbColor(255, 0, 0);
RgbColor green = RgbColor(0, 255, 0);
RgbColor white = RgbColor(255);
RgbColor black = RgbColor(0);

NeoPixelBus<NeoRgbFeature, NeoEsp32Rmt0Ws2812xMethod>* strip_info = NULL;

void infoLight(RgbColor color) { // boot animation for leds count and wifi test
  // Flash the strip in the selected color. White = booted, green = WLAN connected, red = WLAN could not connect
  strip_info->SetPixelColor(0, color);
  strip_info->Show();
}

void ChangeNeoPixels_info() // this set the number of leds of the strip based on web configuration
{
  if (strip_info != NULL) {
    delete strip_info; // delete the previous dynamically created strip
  }
  strip_info = new NeoPixelBus<NeoRgbFeature, NeoEsp32Rmt0Ws2812xMethod>(1, INFO_DATA_PIN); // and recreate with new count
  strip_info->Begin();
}

void blinkLed(uint8_t count, uint16_t interval = 200) {
  RgbColor color = strip_info->GetPixelColor(0);
  for (uint8_t i = 0; i < count; i++) {
    strip_info->SetPixelColor(0, black);
    strip_info->Show();
    delay(interval);
    strip_info->SetPixelColor(0, color);
    strip_info->Show();
    delay(interval);
  }
}