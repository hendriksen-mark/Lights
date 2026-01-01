#include "i2c.h"

struct state
{
	bool lightState;
	int bri, currentBri, lightadress;
};

state lights_i2c[LIGHT_COUNT_I2C];

// define adresses of i2c dimmable lights
//                               kamer	woonkamer	keuken	slaapkamer	gang    badkamer
//                               nummer 1   2   3	4		5			6		7
//                               array  0   1   2	3		4			5		6
int lightadress_i2c[LIGHT_COUNT_I2C] = {9,  10, 11,	12,		13,			14,		15};

unsigned long previousMillis = 0;

int transitiontime_i2c;

WebServer server_i2c(LIGHT_PORT_I2C);

void handleNotFound_i2c()
{
	String message;
	message.reserve(200); // Pre-allocate to reduce memory fragmentation
	message = "File Not Found\n\n";
	message += "URI: ";
	message += server_i2c.uri();
	message += "\nMethod: ";
	message += (server_i2c.method() == HTTP_GET) ? "GET" : "POST";
	message += "\nArguments: ";
	message += server_i2c.args();
	message += "\n";
	for (uint8_t i = 0; i < server_i2c.args(); i++)
	{
		message += " " + server_i2c.argName(i) + ": " + server_i2c.arg(i) + "\n";
	}
	REMOTE_LOG_DEBUG("from:", server_i2c.client().remoteIP().toString(), "not found:", server_i2c.uri());
	server_i2c.send(404, "text/plain", message);
}

void apply_scene_i2c(uint8_t new_scene, uint8_t light)
{
	if (new_scene == 0)
	{
		lights_i2c[light].bri = 144;
	}
	else if (new_scene == 1)
	{
		lights_i2c[light].bri = 254;
	}
	else if (new_scene == 2)
	{
		lights_i2c[light].bri = 1;
	}
}

void send_alert(uint8_t light)
{
	bool prev_light_state_i2c = lights_i2c[light].lightState;
	int prev_bri_i2c = lights_i2c[light].bri;
	int prev_transitiontime_i2c = transitiontime_i2c;

	lights_i2c[light].lightState = false;
	transitiontime_i2c = 1;
	lights_i2c[light].bri = 255;

	process_lightdata_i2c(light);

	delay(10);

	lights_i2c[light].lightState = true;

	process_lightdata_i2c(light);
	delay(10);

	lights_i2c[light].lightState = false;

	process_lightdata_i2c(light);
	delay(10);

	lights_i2c[light].lightState = prev_light_state_i2c;
	lights_i2c[light].bri = prev_bri_i2c;
	transitiontime_i2c = prev_transitiontime_i2c;

	process_lightdata_i2c(light);
}

void process_lightdata_i2c(uint8_t light)
{
	Wire.beginTransmission(lights_i2c[light].lightadress);
	Wire.write(lights_i2c[light].bri);
	Wire.write(lights_i2c[light].lightState);
	Wire.write(highByte(transitiontime_i2c));
	Wire.write(lowByte(transitiontime_i2c));
	int error_code = Wire.endTransmission(true);

	REMOTE_LOG_DEBUG("Light:", light);
	REMOTE_LOG_DEBUG("bri:", lights_i2c[light].bri);
	REMOTE_LOG_DEBUG("state:", lights_i2c[light].lightState);
	REMOTE_LOG_DEBUG("transitiontime:", transitiontime_i2c);
	switch (error_code)
	{
	case 0:
		REMOTE_LOG_DEBUG("wire code:", "success");
		break;
	case 1:
		REMOTE_LOG_DEBUG("wire code:", "data too long to fit in transmit buffer");
		infoLedError(); // Quick error indication
		delay(100);
		break;
	case 2:
		REMOTE_LOG_DEBUG("wire code:", "received NO ACK on transmit of address");
		infoLedError(); // Quick error indication
		delay(100);
		break;
	case 3:
		REMOTE_LOG_DEBUG("wire code:", "received NO ACK on transmit of data");
		infoLedError(); // Quick error indication
		delay(100);
		break;
	case 4:
		REMOTE_LOG_DEBUG("wire code:", "other error");
		break;
	case 5:
		REMOTE_LOG_DEBUG("wire code:", "timeout");
		break;
	default:
		REMOTE_LOG_DEBUG("wire code:", "unknown error");
		break;
	}
}

void lightEngine_i2c()
{
	for (int i = 0; i < LIGHT_COUNT_I2C; i++)
	{
		if (lights_i2c[i].lightState)
		{
			if (lights_i2c[i].bri != lights_i2c[i].currentBri)
			{
				process_lightdata_i2c(i);
				lights_i2c[i].currentBri = lights_i2c[i].bri;
			}
		}
		else
		{
			if (lights_i2c[i].currentBri != 0)
			{
				process_lightdata_i2c(i);
				lights_i2c[i].currentBri = 0;
			}
		}
	}
}

void request_lightdata(uint8_t light)
{
	int light_rec = Wire.requestFrom(lightadress_i2c[light], 2, 1);
	byte buff[2] = {0, 0};
	if (light_rec == 2)
	{
		size_t read = Wire.readBytes(buff, 2);
		if (read == 2)
		{
			lights_i2c[light].bri = buff[0];
			lights_i2c[light].lightState = buff[1];
			lights_i2c[light].currentBri = lights_i2c[light].bri;

			REMOTE_LOG_DEBUG("Light:", light);
			REMOTE_LOG_DEBUG("bri:", lights_i2c[light].bri);
			REMOTE_LOG_DEBUG("state:", lights_i2c[light].lightState);
		}
		else
		{
			REMOTE_LOG_ERROR("Light:", light, "partial read");
		}
	}
	else if (light_rec > 0)
	{
		// fewer bytes than expected
		size_t read = Wire.readBytes(buff, min((int)sizeof(buff), light_rec));
		(void)read;
		REMOTE_LOG_ERROR("Light:", light, "unexpected byte count:", light_rec);
	}
	else
	{
		REMOTE_LOG_ERROR("Light:", light, "no response");
	}
}

void saveState_i2c()
{
	REMOTE_LOG_DEBUG("save i2c state");
	JsonDocument json;
	for (uint8_t i = 0; i < LIGHT_COUNT_I2C; i++)
	{
		JsonObject light = json[String(i)].to<JsonObject>();
		light["on"] = lights_i2c[i].lightState;
		light["bri"] = lights_i2c[i].bri;
	}
	writeJsonFile(I2C_STATE_PATH, json);
}

void restoreState_i2c()
{
	REMOTE_LOG_DEBUG("restore i2c state");
	JsonDocument json;
	if (!readJsonFile(I2C_STATE_PATH, json))
	{
		saveState_i2c();
		return;
	}
	for (JsonPair state : json.as<JsonObject>())
	{
		const char *key = state.key().c_str();
		int lightId = atoi(key);
		JsonObject values = state.value();
		if (values["on"].is<bool>())
			lights_i2c[lightId].lightState = values["on"];
		if (values["bri"].is<int>())
			lights_i2c[lightId].bri = (int)values["bri"];
	}
}

void i2c_setup()
{
	Wire.begin();
	REMOTE_LOG_DEBUG("Setup I2C");
	infoLight(cyan);

	// Assign I2C addresses to each light in the struct
	for (int i = 0; i < LIGHT_COUNT_I2C; i++)
	{
		lights_i2c[i].lightadress = lightadress_i2c[i];
		request_lightdata(i);
	}

	// Restore saved I2C light state if available
	restoreState_i2c();

	server_i2c.on("/state", HTTP_PUT, []() { // HTTP PUT request used to set a new light state
		infoLight(yellow);	 // Yellow for I2C requests
		JsonDocument root;					 // Reduced from 1024 - more efficient for actual usage
		DeserializationError error = deserializeJson(root, server_i2c.arg("plain"));

		if (error)
		{
			server_i2c.send(404, "text/plain", "FAIL. " + server_i2c.arg("plain"));
		}
		else
		{
			for (JsonPair state : root.as<JsonObject>())
			{
				const char *key = state.key().c_str();
				int light = atoi(key) - 1;
				JsonObject values = state.value();
				transitiontime_i2c = 4;

				if (values["on"].is<int>() || values["on"].is<bool>())
				{
					if (values["on"])
					{
						lights_i2c[light].lightState = true;
					}
					else
					{
						lights_i2c[light].lightState = false;
					}
				}

				if (values["bri"].is<float>() || values["bri"].is<int>())
				{
					lights_i2c[light].bri = (int)values["bri"];
				}

				if (values["bri_inc"].is<float>() || values["bri_inc"].is<int>())
				{
					lights_i2c[light].bri += (int)values["bri_inc"];
					if (lights_i2c[light].bri > 255)
						lights_i2c[light].bri = 255;
					else if (lights_i2c[light].bri < 1)
						lights_i2c[light].bri = 1;
				}

				if (values["alert"].is<const char*>() && values["alert"] == "select")
				{
					send_alert(light);
				}

				if (values["transitiontime"].is<float>() || values["transitiontime"].is<int>())
				{
					transitiontime_i2c = (int)values["transitiontime"];
				}
				// process_lightdata_i2c(light, transitiontime);
			}
			String output;
			serializeJson(root, output);
			REMOTE_LOG_DEBUG("from:", server_i2c.client().remoteIP().toString(), "/state put", output);
			server_i2c.send(200, "text/plain", output);
			saveState_i2c();
		}
	});

	server_i2c.on("/state", HTTP_GET, []() { // HTTP GET request used to fetch current light state
		uint8_t light = server_i2c.arg("light").toInt() - 1;
		JsonDocument root; // Reduced from 1024 - only needs 2 values
		root["on"] = lights_i2c[light].lightState;
		root["bri"] = lights_i2c[light].bri;
		String output;
		output.reserve(50); // Pre-allocate to reduce memory fragmentation
		serializeJson(root, output);
		REMOTE_LOG_DEBUG("from:", server_i2c.client().remoteIP().toString(), "/state get", output);
		REMOTE_LOG_DEBUG("light :", light);
		server_i2c.send(200, "text/plain", output);
	});

	server_i2c.on("/detect", []() { // HTTP GET request used to discover the light type
		JsonDocument root; // Reduced from 1024 - optimized for actual content
		root["name"] = LIGHT_NAME_I2C;
		root["lights"] = LIGHT_COUNT_I2C;
		root["protocol"] = LIGHT_PROTOCOL_I2C;
		root["modelid"] = LIGHT_MODEL_I2C;
		root["type"] = LIGHT_TYPE_I2C;
		root["mac"] = get_mac_address();
		root["version"] = LIGHT_VERSION_I2C;
		String output;
		output.reserve(200); // Pre-allocate to reduce memory fragmentation
		serializeJson(root, output);
		REMOTE_LOG_DEBUG("from:", server_i2c.client().remoteIP().toString(), "/detect", output);
		server_i2c.send(200, "text/plain", output);
	});

	server_i2c.on("/", []()
				  {
					  transitiontime_i2c = 4;
					  for (int light = 0; light < LIGHT_COUNT_I2C; light++)
					  {
						  if (server_i2c.hasArg("scene"))
						  {
							  if (server_i2c.arg("bri") == "" && server_i2c.arg("hue") == "" && server_i2c.arg("ct") == "" && server_i2c.arg("sat") == "")
							  {
								  apply_scene_i2c(server_i2c.arg("scene").toInt(), light);
							  }
							  else
							  {
								  if (server_i2c.arg("bri") != "")
								  {
									  lights_i2c[light].bri = server_i2c.arg("bri").toInt();
								  }
							  }
						  }
						  else if (server_i2c.hasArg("on"))
						  {
							  if (server_i2c.arg("on") == "true")
							  {
								  lights_i2c[light].lightState = true;
							  }
							  else
							  {
								  lights_i2c[light].lightState = false;
							  }
						  }
						  else if (server_i2c.hasArg("alert"))
						  {
							  send_alert(light);
						  }
					  }

					  if (server_i2c.hasArg("reset"))
					  {
						  resetESP();
					  }

					  REMOTE_LOG_DEBUG("from:", server_i2c.client().remoteIP().toString(), "/", server_i2c.args(), "args");

					  server_i2c.send_P(200, "text/html", http_content_i2c);
				  });

	server_i2c.on("/reset", []() { // trigger manual reset
		server_i2c.send(200, "text/html", "reset");
		REMOTE_LOG_DEBUG("from:", server_i2c.client().remoteIP().toString(), "/reset");
		resetESP();
	});

	server_i2c.onNotFound(handleNotFound_i2c);

	server_i2c.begin();
}

void i2c_loop()
{
	server_i2c.handleClient();
	lightEngine_i2c();

	unsigned long currentMillis = millis();

	if (currentMillis - previousMillis >= LIGHT_INTERVAL)
	{
		previousMillis = currentMillis;
		for (int i = 0; i < LIGHT_COUNT_I2C; i++)
		{
			request_lightdata(i);
		}
	}
}
