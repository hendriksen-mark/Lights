#include "i2c.h"
#include "mesh.h"
#include "ws2811.h"
#include "debug.h"
#include "functions.h"
#include "config.h"
#include <WiFi.h>

byte mac[] = { MAC_ADDR_0, MAC_ADDR_1, MAC_ADDR_2, MAC_ADDR_3, MAC_ADDR_4, MAC_ADDR_5 };

void ESP_Server_setup(); // Forward declaration

unsigned long lastWiFiCheck = 0;

void setup() {
  Serial.begin(115200);
  LOG_DEBUG("Start ESP32");
  functions_setup();
  ChangeNeoPixels_info();
  infoLight(white);
  blinkLed(1);

  ESP_Server_setup();

  i2c_setup();
  ws_setup();
  mesh_setup();
}

void loop() {
  // WiFi reconnection logic
  unsigned long currentMillis = millis();
  if (currentMillis - lastWiFiCheck >= WIFI_CHECK_INTERVAL) {
    lastWiFiCheck = currentMillis;
    if (WiFi.status() != WL_CONNECTED) {
      LOG_ERROR("WiFi disconnected, attempting reconnection...");
      WiFi.reconnect();
    }
  }

  i2c_loop();
  ws_loop();
  mesh_loop();
  yield(); // Prevent watchdog reset - allows ESP32 to handle WiFi/background tasks
}
