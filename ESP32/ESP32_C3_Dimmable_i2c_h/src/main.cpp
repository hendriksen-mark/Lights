#include "i2c.h"
#include "mesh.h"
#include "ws2811.h"
#include "debug.h"
#include "functions.h"

byte mac[] = { 0xDA, 0xAD, 0xEB, 0xFF, 0xEF, 0xDE };

void ESP_Server_setup(); // Forward declaration

void setup() {
  Serial.begin(115200);
  LOG_SET_LEVEL(DebugLevel);
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
  i2c_loop();
  ws_loop();
  mesh_loop();
}
