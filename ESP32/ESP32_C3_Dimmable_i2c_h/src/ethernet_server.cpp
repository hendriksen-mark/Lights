#include "ethernet_server.h"
#include <WebServer_ESP32_W5500.h>
#include "log_server.h"

void ESP_Server_setup()
{
	REMOTE_LOG_DEBUG("start W5500");
	ESP32_W5500_onEvent();

	ETH.begin(MISO, MOSI, SCK, SS, W5500_INT_GPIO, SPI_CLOCK_MHZ, ETH_SPI_HOST);

	infoLight(white); // Initializing
	while (!ESP32_W5500_isConnected())
	{ // connection to ethernet still not ready
		REMOTE_LOG_DEBUG("W5500_isConnected: ", ESP32_W5500_isConnected());
		infoLedPulse(red, 1, 500); // Pulse red while connecting
	}
	// Show that we are connected
	REMOTE_LOG_DEBUG("W5500_isConnected: ", ESP32_W5500_isConnected(), " IP Address: ", ETH.localIP(), " MAC: ", ETH.macAddress());

	// Setup mDNS
	if (MDNS.begin(DEVICE_NAME))
	{
		REMOTE_LOG_DEBUG("mDNS responder started: ", DEVICE_NAME, ".local");
		MDNS.addService("http", "tcp", LIGHT_PORT_WS);
	}

	infoLedSuccess(); // Show success with green pulses
}

void ota_setup()
{
	ArduinoOTA
		.onStart([]() {
			String type;
			if (ArduinoOTA.getCommand() == U_FLASH)
				type = "sketch";
			else // U_SPIFFS
				type = "filesystem";

			REMOTE_LOG_DEBUG("Start updating " + type);
		})
		.onEnd([]() {
			REMOTE_LOG_DEBUG("\nEnd");
		})
		.onProgress([](unsigned int progress, unsigned int total) {
			REMOTE_LOG_INFO("Progress: " + String((progress / (total / 100))) + "%");
		})
		.onError([](ota_error_t error) {
			REMOTE_LOG_ERROR("Error[%u]: ", error);
			switch (error)
			{
			case OTA_AUTH_ERROR:
				REMOTE_LOG_ERROR("Auth Failed");
				break;
			case OTA_BEGIN_ERROR:
				REMOTE_LOG_ERROR("Begin Failed");
				break;
			case OTA_CONNECT_ERROR:
				REMOTE_LOG_ERROR("Connect Failed");
				break;
			case OTA_RECEIVE_ERROR:
				REMOTE_LOG_ERROR("Receive Failed");
				break;
			case OTA_END_ERROR:
				REMOTE_LOG_ERROR("End Failed");
				break;
			default:
				break;
			}
		})
		.setHostname(DEVICE_NAME)
		.begin();
}

void ethernet_loop()
{
	ArduinoOTA.handle();

	// Handle incoming log client connections and client communication
	handleNewLogConnections();
	handleLogClientCommunication();
	cleanupLogClients();
}

String get_mac_address()
{
	return ETH.macAddress();
}
