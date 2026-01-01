#include "i2c.h"
#include "mesh.h"
#include "ws2811.h"
#include "log_server.h"
#include "custom_log.h"
#include "ntp_client.h"
#include "functions.h"
#include "config.h"

void setup()
{
  Serial.begin(115200);
  REMOTE_LOG_INFO("Start ESP32");
  ChangeNeoPixels_info();
  infoLedFadeIn(white, 500); // Smooth startup indication
  functions_setup();
  delay(200);

  ESP_Server_setup();
  initializeLogServer();
  initializeNTP();
  ota_setup();
  i2c_setup();
  ws_setup();
  mesh_setup();

  // Setup complete - enter idle state
  infoLedIdle();
}

void loop()
{
  ethernet_loop();
  i2c_loop();
  ws_loop();
  mesh_loop();
  yield(); // Prevent watchdog reset - allows ESP32 to handle WiFi/background tasks
}
