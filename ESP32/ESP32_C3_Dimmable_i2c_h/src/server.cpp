#include "server.h"

extern byte mac[];

void ESP_Server_setup(){
  LOG_SET_LEVEL(DebugLevel);
  LOG_DEBUG("start W5500");
  ESP32_W5500_onEvent();

  ETH.begin( MISO_GPIO, MOSI_GPIO, SCK_GPIO, CS_GPIO, INT_GPIO, SPI_CLOCK_MHZ, ETH_SPI_HOST, mac);

  infoLight(white); // play white anymation
  while (!ESP32_W5500_isConnected()) { // connection to wifi still not ready
    LOG_DEBUG("W5500_isConnected: ", ESP32_W5500_isConnected());
    infoLight(red); // play red animation
    delay(500);
  }
  // Show that we are connected
  LOG_DEBUG("W5500_isConnected: ", ESP32_W5500_isConnected(), " IP Address: ", ETH.localIP());
  infoLight(green); // connected, play green animation
}