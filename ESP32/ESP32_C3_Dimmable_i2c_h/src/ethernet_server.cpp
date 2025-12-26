#include "ethernet_server.h"
#include <WebServer_ESP32_W5500.h>

extern byte mac[];

void ESP_Server_setup(){
	LOG_DEBUG("start W5500");
	ESP32_W5500_onEvent();

	ETH.begin( MISO, MOSI, SCK, SS, W5500_INT_GPIO, SPI_CLOCK_MHZ, ETH_SPI_HOST, mac);

	infoLight(white); // play white anymation
	while (!ESP32_W5500_isConnected()) { // connection to wifi still not ready
		LOG_DEBUG("W5500_isConnected: ", ESP32_W5500_isConnected());
		infoLight(red); // play red animation
		delay(500);
	}
	// Show that we are connected
	LOG_DEBUG("W5500_isConnected: ", ESP32_W5500_isConnected(), " IP Address: ", ETH.localIP());
	
	// Setup mDNS
	if (MDNS.begin(DEVICE_NAME)) {
		LOG_DEBUG("mDNS responder started: ", DEVICE_NAME, ".local");
		MDNS.addService("http", "tcp", LIGHT_PORT_WS);
	}
	
	infoLight(green); // connected, play green animation
}
