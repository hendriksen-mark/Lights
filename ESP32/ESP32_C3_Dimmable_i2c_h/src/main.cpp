#include "i2c.h"
#include "mesh.h"
#include "ws2811.h"
#include "debug.h"
#include "functions.h"
#include "config.h"
#include <WiFi.h>

void ESP_Server_setup(); // Forward declaration

void setup()
{
  Serial.begin(115200);
  LOG_INFO("Start ESP32");
  functions_setup();
  ChangeNeoPixels_info();
  infoLedFadeIn(white, 500); // Smooth startup indication
  delay(200);

  ESP_Server_setup();

  i2c_setup();
  ws_setup();
  mesh_setup();

  // Setup complete - enter idle state
  infoLedIdle();
}

void loop()
{
  i2c_loop();
  ws_loop();
  mesh_loop();
  yield(); // Prevent watchdog reset - allows ESP32 to handle WiFi/background tasks
}
