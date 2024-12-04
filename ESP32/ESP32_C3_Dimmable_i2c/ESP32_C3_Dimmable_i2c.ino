//#include <EthernetWebServer.h>  //https://github.com/khoih-prog/EthernetWebServer
#include <WebServer_ESP32_W5500.h> //https://github.com/khoih-prog/WebServer_ESP32_W5500
//#include <Ethernet.h>
#define INT_GPIO 3

#include <LittleFS.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
//#define DEBUGLOG_ENABLE_FILE_LOGGER
#include <DebugLog.h>
#include <HTTPUpdateServer.h>
HTTPUpdateServer httpUpdateServer;

#include <NeoPixelBus.h>

RgbColor red = RgbColor(255, 0, 0);
RgbColor green = RgbColor(0, 255, 0);
RgbColor white = RgbColor(255);
RgbColor black = RgbColor(0);

#define INFO_DATA_PIN 12

byte mac[] = { 0xDA, 0xAD, 0xEB, 0xFF, 0xEF, 0xDE };

NeoPixelBus<NeoRgbFeature, NeoEsp32Rmt0Ws2812xMethod>* strip_info = NULL;

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

void factoryReset() {
  LittleFS.format();
  //WiFi.disconnect(false, true);
  blinkLed(8, 100);
  ESP.restart();
}

void infoLight(RgbColor color) { // boot animation for leds count and wifi test
  // Flash the strip in the selected color. White = booted, green = WLAN connected, red = WLAN could not connect
  strip_info->SetPixelColor(0, color);
  strip_info->Show();
}

void setup() {
  Serial.begin(115200);
  LOG_SET_LEVEL(DebugLogLevel::LVL_TRACE);
  //LOG_FILE_SET_LEVEL(DebugLogLevel::LVL_TRACE);
  if (!LittleFS.begin()) {
    LOG_DEBUG("Failed to mount file system");
    //Serial.println("Failed to mount file system");
    LittleFS.format();
  } else {
    //LOG_ATTACH_FS_AUTO(LittleFS, "/log.txt", FILE_WRITE);
  }
  LOG_DEBUG("Start ESP32");
  EEPROM.begin(512);
  ChangeNeoPixels_info();
  infoLight(white);
  blinkLed(1);

  LOG_DEBUG("start W5500");
  ESP32_W5500_onEvent();

  ETH.begin( MISO_GPIO, MOSI_GPIO, SCK_GPIO, CS_GPIO, INT_GPIO, SPI_CLOCK_MHZ, ETH_SPI_HOST, mac);

  infoLight(white); // play white anymation
  while (!ESP32_W5500_isConnected()) { // connection to wifi still not ready
    LOG_DEBUG("W5500_isConnected:", ESP32_W5500_isConnected());
    infoLight(red); // play red animation
    delay(500);
  }
  // Show that we are connected
  LOG_DEBUG("W5500_isConnected:", ESP32_W5500_isConnected(), "IP Address:", ETH.localIP());
  infoLight(green); // connected, play green animation

  i2c_setup();
  ws_setup();
  mesh_setup();
}

void loop() {
  i2c_loop();
  ws_loop();
  mesh_loop();
}
