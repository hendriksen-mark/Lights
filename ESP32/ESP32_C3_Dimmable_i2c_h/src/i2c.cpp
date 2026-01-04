#include "i2c.h"

struct state
{
	bool lightState, reachable;
	int bri, lightadress;
};

state lights_i2c[LIGHT_COUNT_I2C];

// define adresses of i2c dimmable lights
//                               kamer	woonkamer	keuken	slaapkamer	gang    badkamer
//                               nummer 1   2   3	4		5			6		7
//                               array  0   1   2	3		4			5		6
int lightadress_i2c[LIGHT_COUNT_I2C] = {9,  10, 11,	12,		13,			14,		15};

unsigned long previousMillis = 0;

int transitiontime_i2c;

uint8_t scene_i2c, startup_i2c;

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
	REMOTE_LOG_DEBUG("from:", server_i2c.client().remoteIP().toString(), "not found:", server_i2c.uri(), server_i2c.args());
	server_i2c.send(404, "text/plain", message);
}

void apply_scene_i2c(uint8_t new_scene, uint8_t light)
{
	if (new_scene == 0)
	{
		lights_i2c[light].bri = 144;
		lights_i2c[light].lightState = true;
	}
	else if (new_scene == 1)
	{
		lights_i2c[light].bri = 254;
		lights_i2c[light].lightState = true;
	}
	else if (new_scene == 2)
	{
		lights_i2c[light].bri = 1;
		lights_i2c[light].lightState = true;
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

	JsonDocument json;
	json["on"] = lights_i2c[light].lightState;
	json["bri"] = lights_i2c[light].bri;
	json["transitiontime"] = transitiontime_i2c;
	String output;
	output.reserve(50); // Pre-allocate to reduce memory fragmentation
	serializeJson(json, output);

	REMOTE_LOG_DEBUG("Light:", light, "state sent:", output);
	switch (error_code)
	{
	case 0:
		REMOTE_LOG_DEBUG("wire code:", "success");
		lights_i2c[light].reachable = true;
		break;
	case 1:
		REMOTE_LOG_DEBUG("wire code:", "data too long to fit in transmit buffer");
		infoLedError(); // Quick error indication
		lights_i2c[light].reachable = false;
		break;
	case 2:
		REMOTE_LOG_DEBUG("wire code:", "received NO ACK on transmit of address");
		infoLedError(); // Quick error indication
		lights_i2c[light].reachable = false;
		break;
	case 3:
		REMOTE_LOG_DEBUG("wire code:", "received NO ACK on transmit of data");
		infoLedError(); // Quick error indication
		lights_i2c[light].reachable = false;
		break;
	case 4:
		REMOTE_LOG_DEBUG("wire code:", "other error");
		infoLedError(); // Quick error indication
		lights_i2c[light].reachable = false;
		break;
	case 5:
		REMOTE_LOG_DEBUG("wire code:", "timeout");
		infoLedError(); // Quick error indication
		lights_i2c[light].reachable = false;
		break;
	default:
		REMOTE_LOG_DEBUG("wire code:", "unknown error");
		infoLedError(); // Quick error indication
		lights_i2c[light].reachable = false;
		break;
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

			JsonDocument json;
			json["on"] = lights_i2c[light].lightState;
			json["bri"] = lights_i2c[light].bri;
			String output;
			output.reserve(50); // Pre-allocate to reduce memory fragmentation
			serializeJson(json, output);

			REMOTE_LOG_DEBUG("Light:", light, "state fetched:", output);
			lights_i2c[light].reachable = true;
		}
		else
		{
			REMOTE_LOG_ERROR("Light:", light, "partial read");
			lights_i2c[light].reachable = false;
		}
	}
	else if (light_rec > 0)
	{
		// fewer bytes than expected
		size_t read = Wire.readBytes(buff, min((int)sizeof(buff), light_rec));
		(void)read;
		REMOTE_LOG_ERROR("Light:", light, "unexpected byte count:", light_rec);
		lights_i2c[light].reachable = false;
	}
	else
	{
		REMOTE_LOG_ERROR("Light:", light, "no response");
		lights_i2c[light].reachable = false;
	}
}

bool saveState_i2c()
{
	REMOTE_LOG_DEBUG("save i2c state");
	JsonDocument json;
	for (uint8_t i = 0; i < LIGHT_COUNT_I2C; i++)
	{
		JsonObject light = json[String(i)].to<JsonObject>();
		light["on"] = lights_i2c[i].lightState;
		light["bri"] = lights_i2c[i].bri;
	}
	return writeJsonFile(I2C_STATE_PATH, json);
}

bool restoreState_i2c()
{
	REMOTE_LOG_DEBUG("restore i2c state");
	JsonDocument json;
	if (!readJsonFile(I2C_STATE_PATH, json))
	{
		REMOTE_LOG_INFO("Create new file with default values");
		return saveState_i2c();
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
	return true;
}

bool saveConfig_i2c()
{
	REMOTE_LOG_DEBUG("save i2c config");
	JsonDocument json;
	json["startup"] = startup_i2c;
	json["scene"] = scene_i2c;

	return writeJsonFile(I2C_CONFIG_PATH, json);
}

bool loadConfig_i2c()
{
	REMOTE_LOG_DEBUG("load i2c config");
	JsonDocument json;
	if (!readJsonFile(I2C_CONFIG_PATH, json))
	{
		REMOTE_LOG_DEBUG("Create new file with default values");
		return saveConfig_i2c();
	}
	startup_i2c = (uint8_t)json["startup"];
	scene_i2c = (uint8_t)json["scene"];
	return true;
}

void i2c_setup()
{
	Wire.begin();
	REMOTE_LOG_DEBUG("Setup I2C");
	infoLight(cyan);

	if (loadConfig_i2c())
	{
		REMOTE_LOG_DEBUG("I2C config loaded");
	}
	else
	{
		REMOTE_LOG_DEBUG("I2C config load failed, using defaults");
	}

	// Assign I2C addresses to each light in the struct
	for (int i = 0; i < LIGHT_COUNT_I2C; i++)
	{
		lights_i2c[i].lightadress = lightadress_i2c[i];
		request_lightdata(i);
	}

	switch (startup_i2c)
	{
	case 0:
		REMOTE_LOG_DEBUG("Startup: Restore previous state");
		if (restoreState_i2c())
		{
			REMOTE_LOG_DEBUG("I2C state restored");
		}
		else
		{
			REMOTE_LOG_DEBUG("I2C state restore failed, using defaults");
		}
		break;
	case 1:
		REMOTE_LOG_DEBUG("Startup: All lights ON");
		for (uint8_t i = 0; i < LIGHT_COUNT_I2C; i++)
		{
			lights_i2c[i].lightState = true;
		}
		break;
	default:
		REMOTE_LOG_DEBUG("Startup: Apply scene", String(scene_i2c));
		for (uint8_t i = 0; i < LIGHT_COUNT_I2C; i++)
		{
			apply_scene_i2c(scene_i2c, i);
		}
		break;
	}

	for (uint8_t i = 0; i < LIGHT_COUNT_I2C; i++)
	{
		process_lightdata_i2c(i);
	}

	server_i2c.on("/state", HTTP_PUT, []() { // HTTP PUT request used to set a new light state
		infoLight(yellow);					 // Yellow for I2C requests
		JsonDocument root;
		DeserializationError error = deserializeJson(root, server_i2c.arg("plain"));

		if (error)
		{
			server_i2c.send(404, "text/plain", "FAIL. " + server_i2c.arg("plain"));
		}
		else
		{
			// Check if this is single-light format (e.g., {"on":true,"bri":254})
			// or multi-light format (e.g., {"1":{"on":true},"2":{"bri":254}})
			bool isSingleLightFormat = false;
			JsonObject rootObj = root.as<JsonObject>();

			// Detect single-light format by checking if first key is a state property
			if (rootObj.size() > 0)
			{
				const char *firstKey = rootObj.begin()->key().c_str();
				if (strcmp(firstKey, "on") == 0 || strcmp(firstKey, "bri") == 0 ||
					strcmp(firstKey, "bri_inc") == 0 || strcmp(firstKey, "alert") == 0 ||
					strcmp(firstKey, "transitiontime") == 0)
				{
					isSingleLightFormat = true;
				}
			}

			if (isSingleLightFormat)
			{
				// Handle single light format - apply to ALL lights
				transitiontime_i2c = 4;

				if (rootObj["transitiontime"].is<float>() || rootObj["transitiontime"].is<int>())
				{
					transitiontime_i2c = (int)rootObj["transitiontime"];
				}

				// Apply changes to all lights
				for (int light = 0; light < LIGHT_COUNT_I2C; light++)
				{
					if (rootObj["on"].is<int>() || rootObj["on"].is<bool>())
					{
						lights_i2c[light].lightState = rootObj["on"] ? true : false;
					}

					if (rootObj["bri"].is<float>() || rootObj["bri"].is<int>())
					{
						lights_i2c[light].bri = (int)rootObj["bri"];
					}

					if (rootObj["bri_inc"].is<float>() || rootObj["bri_inc"].is<int>())
					{
						lights_i2c[light].bri += (int)rootObj["bri_inc"];
						if (lights_i2c[light].bri > 255)
							lights_i2c[light].bri = 255;
						else if (lights_i2c[light].bri < 1)
							lights_i2c[light].bri = 1;
					}

					if (rootObj["alert"].is<const char *>() && rootObj["alert"] == "select")
					{
						send_alert(light);
					}

					process_lightdata_i2c(light);
				}
			}
			else
			{
				// Handle multi-light format
				for (JsonPair state : rootObj)
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

					if (values["alert"].is<const char *>() && values["alert"] == "select")
					{
						send_alert(light);
					}

					if (values["transitiontime"].is<float>() || values["transitiontime"].is<int>())
					{
						transitiontime_i2c = (int)values["transitiontime"];
					}
					process_lightdata_i2c(light);
				}
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
		JsonDocument root;
		root["on"] = lights_i2c[light].lightState;
		root["bri"] = lights_i2c[light].bri;
		root["reachable"] = lights_i2c[light].reachable;
		String output;
		output.reserve(50); // Pre-allocate to reduce memory fragmentation
		serializeJson(root, output);
		REMOTE_LOG_DEBUG("from:", server_i2c.client().remoteIP().toString(), "/state get light:", light, output);
		server_i2c.send(200, "text/plain", output);
	});

	server_i2c.on("/detect", []() { // HTTP GET request used to discover the light type
		JsonDocument root;
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

	server_i2c.on("/config", HTTP_GET, []()
				  {
		JsonDocument cfg;
		cfg["name"] = LIGHT_NAME_I2C;
		cfg["lightscount"] = LIGHT_COUNT_I2C;
		cfg["pixelcount"] = 0; // I2C lights don't use pixels
		cfg["startup"] = startup_i2c;
		cfg["scene"] = scene_i2c;
		String out;
		serializeJson(cfg, out);
		REMOTE_LOG_DEBUG("from:", server_i2c.client().remoteIP().toString(), "/config", out);
		server_i2c.send(200, "application/json", out); });

	server_i2c.on("/", []()
				  {
		bool anyChange = false;
		bool toWrite[LIGHT_COUNT_I2C] = {false};

		if (server_i2c.hasArg("startup")) {
			startup_i2c = server_i2c.arg("startup").toInt();
			anyChange = true; // treat startup change as a config change
		}
		for (int light = 0; light < LIGHT_COUNT_I2C; light++) {
			if (server_i2c.hasArg("scene")) {
				if (server_i2c.arg("bri") == "" && server_i2c.arg("hue") == "" && server_i2c.arg("ct") == "" && server_i2c.arg("sat") == "") {
					scene_i2c = server_i2c.arg("scene").toInt();
					apply_scene_i2c(scene_i2c, light);
					anyChange = true;
					toWrite[light] = true;
				} else {
					if (server_i2c.arg("bri") != "") {
						lights_i2c[light].bri = server_i2c.arg("bri").toInt();
						anyChange = true;
						toWrite[light] = true;
					}
				}
			} else if (server_i2c.hasArg("on")) {
				bool newState = (server_i2c.arg("on") == "true");
				if (lights_i2c[light].lightState != newState) {
					lights_i2c[light].lightState = newState;
					anyChange = true;
					toWrite[light] = true;
				}
			} else if (server_i2c.hasArg("alert")) {
				send_alert(light);
				anyChange = true;
			}
		}

		if (anyChange) {
			transitiontime_i2c = 4;
			for (int light = 0; light < LIGHT_COUNT_I2C; light++) {
				if (toWrite[light]) process_lightdata_i2c(light);
			}
			// persist startup/scene changes when provided
			if (server_i2c.hasArg("startup") || server_i2c.hasArg("scene")) {
				saveConfig_i2c();
			}
			// save light state only if scene was applied or we actually wrote to any light
			bool didWrite = false;
			for (int i = 0; i < LIGHT_COUNT_I2C; i++) {
				if (toWrite[i]) { didWrite = true; break; }
			}
			if (server_i2c.hasArg("scene") || didWrite) {
				saveState_i2c();
			}
			String outputArgs;
			outputArgs.reserve(64);
			outputArgs = String("changed args:") + server_i2c.args();
			REMOTE_LOG_DEBUG("from:", server_i2c.client().remoteIP().toString(), "/ (changed)", "args:", outputArgs.c_str());
			server_i2c.sendHeader("Location", "/");
			server_i2c.send(303, "text/plain", "");
			return;
		}

		if (server_i2c.hasArg("reset")) {
			REMOTE_LOG_DEBUG("from:", server_i2c.client().remoteIP().toString(), "/ reset requested");
			server_i2c.sendHeader("Location", "/");
			server_i2c.send(303, "text/plain", "");
			resetESP();
			return;
		}

		if (server_i2c.args() > 0) {
			REMOTE_LOG_DEBUG("from:", server_i2c.client().remoteIP().toString(), "/ (no change)", "args:", server_i2c.args());
		}

		server_i2c.send_P(200, "text/html", http_content_i2c); });

	server_i2c.on("/reset", []() { // trigger manual reset
		server_i2c.send(200, "text/html", "reset");
		REMOTE_LOG_DEBUG("from:", server_i2c.client().remoteIP().toString(), "/reset");
		resetESP();
	});

	server_i2c.on("/get_state_save", []() { // display save file content for debugging
		JsonDocument json;
		if (readJsonFile(I2C_STATE_PATH, json))
		{
			String output;
			serializeJson(json, output);
			REMOTE_LOG_DEBUG("from:", server_i2c.client().remoteIP().toString(), "/get_state_save", output);
			server_i2c.send(200, "text/plain", output);
		}
		else
		{
			REMOTE_LOG_DEBUG("from:", server_i2c.client().remoteIP().toString(), "/get_state_save", "failed to read file");
			server_i2c.send(404, "text/plain", "Failed to read file");
		}
	});

	server_i2c.on("/get_config_save", []() { // display config file content for debugging
		JsonDocument json;
		if (readJsonFile(I2C_CONFIG_PATH, json))
		{
			String output;
			serializeJson(json, output);
			REMOTE_LOG_DEBUG("from:", server_i2c.client().remoteIP().toString(), "/get_config_save", output);
			server_i2c.send(200, "text/plain", output);
		}
		else
		{
			REMOTE_LOG_DEBUG("from:", server_i2c.client().remoteIP().toString(), "/get_config_save", "failed to read file");
			server_i2c.send(404, "text/plain", "Failed to read file");
		}
	});

	server_i2c.onNotFound(handleNotFound_i2c);

	server_i2c.begin();
}

void i2c_loop()
{
	server_i2c.handleClient();

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
