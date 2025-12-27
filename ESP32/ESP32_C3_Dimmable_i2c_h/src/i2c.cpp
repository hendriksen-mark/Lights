#include "i2c.h"

extern byte mac[];

struct state
{
	bool lightState;
	int bri, lightadress;
	float stepLevel, currentBri;
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

	LOG_DEBUG("Light:", light);
	LOG_DEBUG("bri:", lights_i2c[light].bri);
	LOG_DEBUG("state:", lights_i2c[light].lightState);
	LOG_DEBUG("transitiontime:", transitiontime_i2c);
	switch (error_code)
	{
	case 0:
		LOG_DEBUG("wire code:", "success");
		break;
	case 1:
		LOG_DEBUG("wire code:", "data too long to fit in transmit buffer");
		infoLedError(); // Quick error indication
		delay(100);
		break;
	case 2:
		LOG_DEBUG("wire code:", "received NO ACK on transmit of address");
		infoLedError(); // Quick error indication
		delay(100);
		break;
	case 3:
		LOG_DEBUG("wire code:", "received NO ACK on transmit of data");
		infoLedError(); // Quick error indication
		delay(100);
		break;
	case 4:
		LOG_DEBUG("wire code:", "other error");
		break;
	case 5:
		LOG_DEBUG("wire code:", "timeout");
		break;
	default:
		LOG_DEBUG("wire code:", "unknown error");
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
	byte buff[2];
	Wire.readBytes(buff, 2);
	if (light_rec > 0)
	{
		lights_i2c[light].bri = buff[0];
		lights_i2c[light].lightState = buff[1];
		// lights_i2c[light].currentBri = lights_i2c[light].bri;

		LOG_DEBUG("Light:", light);
		LOG_DEBUG("bri:", lights_i2c[light].bri);
		LOG_DEBUG("state:", lights_i2c[light].lightState);
	}
	else
	{
		LOG_ERROR("Light:", light, "no response");
	}
}

void i2c_setup()
{
	Wire.begin();
	LOG_DEBUG("Setup I2C");

	// Assign I2C addresses to each light in the struct
	for (int i = 0; i < LIGHT_COUNT_I2C; i++)
	{
		lights_i2c[i].lightadress = lightadress_i2c[i];
		request_lightdata(i);
	}

	server_i2c.on("/state", HTTP_PUT, []() { // HTTP PUT request used to set a new light state
		infoLight(RgbColor(255, 255, 0));	 // Yellow for I2C requests
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

				if (!values["on"].isNull())
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

				if (!values["bri"].isNull())
				{
					lights_i2c[light].bri = values["bri"];
				}

				if (!values["bri_inc"].isNull())
				{
					lights_i2c[light].bri += (int)values["bri_inc"];
					if (lights_i2c[light].bri > 255)
						lights_i2c[light].bri = 255;
					else if (lights_i2c[light].bri < 1)
						lights_i2c[light].bri = 1;
				}

				if (values["alert"].isNull() && values["alert"] == "select")
				{
					send_alert(light);
				}

				if (!values["transitiontime"].isNull())
				{
					transitiontime_i2c = values["transitiontime"];
				}
				// process_lightdata_i2c(light, transitiontime);
			}
			String output;
			serializeJson(root, output);
			LOG_DEBUG("/state put", output);
			server_i2c.send(200, "text/plain", output);
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
		LOG_DEBUG("/state get", output);
		LOG_DEBUG("light :", light);
		server_i2c.send(200, "text/plain", output);
	});

	server_i2c.on("/detect", []() { // HTTP GET request used to discover the light type
		char macString[32] = {0};
		sprintf(macString, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		JsonDocument root; // Reduced from 1024 - optimized for actual content
		root["name"] = LIGHT_NAME_I2C;
		root["lights"] = LIGHT_COUNT_I2C;
		root["protocol"] = LIGHT_PROTOCOL_I2C;
		root["modelid"] = LIGHT_MODEL_I2C;
		root["type"] = LIGHT_TYPE_I2C;
		root["mac"] = String(macString);
		root["version"] = LIGHT_VERSION_I2C;
		String output;
		output.reserve(200); // Pre-allocate to reduce memory fragmentation
		serializeJson(root, output);
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
						  if (lights_i2c[light].lightState)
						  {
							  lights_i2c[light].stepLevel = ((float)lights_i2c[light].bri - lights_i2c[light].currentBri) / transitiontime_i2c;
						  }
						  else
						  {
							  lights_i2c[light].stepLevel = lights_i2c[light].currentBri / transitiontime_i2c;
						  }
					  }
					  if (server_i2c.hasArg("reset"))
					  {
						  resetESP();
					  }

					  const char http_content[] PROGMEM = R"=====(<!doctype html><html><head><meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1.0"><title>Light Setup</title><style>body{font-family:Arial,sans-serif;max-width:600px;margin:20px auto;padding:20px}label{display:inline-block;width:100px}input,select{margin:5px 0;padding:5px}.btn{background:#4CAF50;color:white;padding:10px 20px;border:none;cursor:pointer;margin:5px}.btn:hover{background:#45a049}.off{background:#f44336}.off:hover{background:#da190b}</style></head><body><h2>Light Setup</h2><form method="post"><div><label>Power:</label><a class="btn" href="/?on=true">ON</a><a class="btn off" href="/?on=false">OFF</a></div><div><label>Startup:</label><select name="startup" onchange="this.form.submit()"><option value="0">Last state</option><option value="1">On</option><option value="2">Off</option></select></div><div><label>Scene:</label><select name="scene" onchange="this.form.submit()"><option value="0">Relax</option><option value="1">Bright</option><option value="2">Nightly</option></select></div><div><label>Brightness:</label><input name="bri" type="number" min="1" max="254"></div><div><button type="submit" class="btn">Save</button></div><div><a href="/?alert=1">Alert</a> | <a href="/?reset=1">Reset</a></div></form></body></html>)=====";

					  server_i2c.send_P(200, "text/html", http_content); });

	server_i2c.on("/reset", []() { // trigger manual reset
		server_i2c.send(200, "text/html", "reset");
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
