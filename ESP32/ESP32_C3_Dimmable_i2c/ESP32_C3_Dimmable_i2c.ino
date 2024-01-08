//#define wifi
#define ethernet

#ifdef wifi
#include <WiFi.h>
#include <WiFiUdp.h>
#include <WebServer.h>
#include <WiFiManager.h>
#endif

#ifdef ethernet
//#include <EthernetWebServer.h>  //https://github.com/khoih-prog/EthernetWebServer
#include <WebServer_ESP32_W5500.h> //https://github.com/khoih-prog/WebServer_ESP32_W5500
//#include <Ethernet.h>
#define INT_GPIO 3
#endif

#include <LittleFS.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <DebugLog.h>
#include <HTTPUpdateServer.h>
HTTPUpdateServer httpUpdateServer;

#include <NeoPixelBus.h>

RgbColor red = RgbColor(255, 0, 0);
RgbColor green = RgbColor(0, 255, 0);
RgbColor white = RgbColor(255);
RgbColor black = RgbColor(0);

#define INFO_DATA_PIN 12

//#define DEBUG_PIN 64
//bool current_debug = false, previous_debug = false;
int debug_light, light_rec, rec, debug_code;
//byte mac[6];

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

/*void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255) {
  // calculate duty, 4095 from 2 ^ 12 - 1
  uint32_t duty = (4095 / valueMax) * min(value, valueMax);

  // write duty to LEDC
  ledcWrite(channel, duty);
  }*/


void factoryReset() {
  LittleFS.format();
  //WiFi.disconnect(false, true);
  blinkLed(8, 100);
  ESP.restart();
}

/*void process_lightdata(uint8_t light, float transitiontime) {
  transitiontime *= 33;
  if (light_state[light]) {
    step_level[light] = (bri[light] - current_bri[light]) / transitiontime;
  } else {
    step_level[light] = current_bri[light] / transitiontime;
  }
  }*/


/*void lightEngine() {
  for (int i = 0; i < LIGHTS_COUNT; i++) {
    if (light_state[i]) {
      if (bri[i] != current_bri[i]) {
        in_transition = true;
        current_bri[i] += step_level[i];
        if ((step_level[i] > 0.0 && current_bri[i] > bri[i]) || (step_level[i] < 0.0 && current_bri[i] < bri[i])) {
          current_bri[i] = bri[i];
        }
        ledcWrite(LEDC_CHANNEL_0, (int)(current_bri[i] * 16));
       // ledcAnalogWrite(LEDC_CHANNEL_0, (int)(current_bri[i]));
      }
    } else {
      if (current_bri[i] != 0 ) {
        in_transition = true;
        current_bri[i] -= step_level[i];
        if (current_bri[i] < 0) {
          current_bri[i] = 0;
        }
        //ledcAnalogWrite(LEDC_CHANNEL_0, (int)(current_bri[i]));
        ledcWrite(LEDC_CHANNEL_0, (int)(current_bri[i] * 16));

      }
    }
  }
  if (in_transition) {
    delay(2);
    in_transition = false;
  }
  }*/

void infoLight(RgbColor color) { // boot animation for leds count and wifi test
  // Flash the strip in the selected color. White = booted, green = WLAN connected, red = WLAN could not connect
  strip_info->SetPixelColor(0, color);
  strip_info->Show();
}

void setup() {
  Serial.begin(115200);
  LOG_SET_LEVEL(DebugLogLevel::LVL_TRACE);
  LOG_DEBUG(F("Start ESP32"));
  EEPROM.begin(512);
  //ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_12_BIT);
  //ledcAttachPin(LED_PIN, LEDC_CHANNEL_0);
  ChangeNeoPixels_info();
  infoLight(white);
  blinkLed(1);
  //ledcAnalogWrite(LEDC_CHANNEL_0, 50);


  /*for (uint8_t light = 0; light < LIGHTS_COUNT; light++) {
    apply_scene(EEPROM.read(2), light);
    step_level[light] = bri[light] / 150.0;
    }

    if (EEPROM.read(1) == 1 || (EEPROM.read(1) == 0 && EEPROM.read(0) == 1)) {
    for (int i = 0; i < LIGHTS_COUNT; i++) {
      light_state[i] = true;
    }
    for (int j = 0; j < 200; j++) {
      lightEngine();
    }
    }*/
#ifdef wifi
  WiFi.mode(WIFI_STA);
  wm.setDebugOutput(false);
  wm.setConfigPortalTimeout(120);
#endif

#ifdef ethernet
  LOG_DEBUG(F("start W5500"));
  ESP32_W5500_onEvent();

  // start the ethernet connection and the server:
  // Use DHCP dynamic IP and random mac
  //bool begin(int MISO_GPIO, int MOSI_GPIO, int SCLK_GPIO, int CS_GPIO, int INT_GPIO, int SPI_CLOCK_MHZ,
  //           int SPI_HOST, uint8_t *W6100_Mac = W6100_Default_Mac);
  ETH.begin( MISO_GPIO, MOSI_GPIO, SCK_GPIO, CS_GPIO, INT_GPIO, SPI_CLOCK_MHZ, ETH_SPI_HOST, mac);
  //ETH.begin( MISO_GPIO, MOSI_GPIO, SCK_GPIO, CS_GPIO, INT_GPIO, SPI_CLOCK_MHZ, ETH_SPI_HOST, mac[millis() % NUMBER_OF_MAC] );

  // Static IP, leave without this line to get IP via DHCP
  //bool config(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns1 = 0, IPAddress dns2 = 0);
  //ETH.config(myIP, myGW, mySN, myDNS);

  //ESP32_W5500_waitForConnect();
  //Ethernet.begin(mac);
  infoLight(white); // play white anymation
  while (!ESP32_W5500_isConnected()) { // connection to wifi still not ready
    LOG_DEBUG(F("W5500_isConnected: "), ESP32_W5500_isConnected());
    infoLight(red); // play red animation
    delay(500);
  }
  // Show that we are connected
  LOG_DEBUG(F("W5500_isConnected: "), ESP32_W5500_isConnected());
  infoLight(green); // connected, play green animation
#endif

#ifdef wifi
  bool res;
  res = wm.autoConnect(light_name);

  if (!res) {
    ESP.restart();
  }
  WiFi.macAddress(mac);

  infoLight(white); // play white anymation
  while (WiFi.status() != WL_CONNECTED) { // connection to wifi still not ready
    infoLight(red); // play red animation
    delay(500);
  }
  // Show that we are connected
  infoLight(green); // connected, play green animation

#endif

  /*if (! light_state[0])  {
    // Show that we are connected
    ledcAnalogWrite(LEDC_CHANNEL_0, 50);
    delay(500);
    ledcAnalogWrite(LEDC_CHANNEL_0, 0);
    }*/

  i2c_setup();
  ws_setup();
}

void loop() {
  i2c_loop();
  ws_loop();

}
