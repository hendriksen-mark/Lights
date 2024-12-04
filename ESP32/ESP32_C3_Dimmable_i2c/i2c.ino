#include <Wire.h>

#define light_name_i2c "Dimmable Hue Light ESP32"  //default light name
#define LIGHT_VERSION_i2c 2.1

//define pins
#define LIGHTS_COUNT_i2c 7
//                            kamer    woonkamer      keuken    slaapkamer      gang    badkamer
//                            nummer   1    2   3     4         5               6       7
//                            array    0    1   2     3         4               5       6
int lightadress_i2c[LIGHTS_COUNT_i2c] {9,   10, 11,   12,       13,             14,     15};

unsigned long previousMillis = 0;
#define LIGHT_interval 60000

bool light_state_i2c[LIGHTS_COUNT_i2c], in_transition_i2c, alert = false;
int transitiontime_i2c, bri_i2c[LIGHTS_COUNT_i2c], error_code, debug_light, light_rec, rec, debug_code;
float step_level_i2c[LIGHTS_COUNT_i2c], current_bri_i2c[LIGHTS_COUNT_i2c];

byte hb, lb;

WebServer server_i2c(80);

void handleNotFound_i2c() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server_i2c.uri();
  message += "\nMethod: ";
  message += (server_i2c.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server_i2c.args();
  message += "\n";
  for (uint8_t i = 0; i < server_i2c.args(); i++) {
    message += " " + server_i2c.argName(i) + ": " + server_i2c.arg(i) + "\n";
  }
  server_i2c.send(404, "text/plain", message);
}


void apply_scene_i2c(uint8_t new_scene,  uint8_t light) {
  if ( new_scene == 0) {
    bri_i2c[light] = 144;
  } else if ( new_scene == 1) {
    bri_i2c[light] = 254;
  } else if ( new_scene == 2) {
    bri_i2c[light] = 1;
  }
}

void send_alert(uint8_t light) {
  bool prev_light_state_i2c = light_state_i2c[light];
  int prev_bri_i2c = bri_i2c[light];
  int prev_transitiontime_i2c = transitiontime_i2c;

  light_state_i2c[light] = false;
  transitiontime_i2c = 1;
  bri_i2c[light] = 255;

  process_lightdata_i2c(light);

  delay(10);

  light_state_i2c[light] = true;

  process_lightdata_i2c(light);
  delay(10);

  light_state_i2c[light] = false;

  process_lightdata_i2c(light);
  delay(10);

  light_state_i2c[light] = prev_light_state_i2c;
  bri_i2c[light] = prev_bri_i2c;
  transitiontime_i2c = prev_transitiontime_i2c;

  process_lightdata_i2c(light);

}

void process_lightdata_i2c(uint8_t light) {
  Wire.beginTransmission(lightadress_i2c[light]);
  Wire.write(bri_i2c[light]);
  Wire.write(light_state_i2c[light]);
  Wire.write(highByte(transitiontime_i2c));
  Wire.write(lowByte(transitiontime_i2c));
  error_code = Wire.endTransmission(true);

  if (error_code) {
    debug_light = light;
    debug_code = error_code;
  } else {
    if (debug_light == light && error_code == 0) {
      debug_light = 0x7F;
      debug_code = 0x7F;
    }
  }
  LOG_DEBUG("Light:", light);
  LOG_DEBUG("bri:", bri_i2c[light]);
  LOG_DEBUG("state:", light_state_i2c[light]);
  LOG_DEBUG("transitiontime:", transitiontime_i2c);
  if (error_code == 0) LOG_DEBUG("wire code:", "success");
  if (error_code == 1) LOG_DEBUG("wire code:", "data too long to fit in transmit buffer");
  if (error_code == 2) LOG_DEBUG("wire code:", "received NO ACK on transmit of address");
  if (error_code == 3) LOG_DEBUG("wire code:", "received NO ACK on transmit of data");
  if (error_code == 4) LOG_DEBUG("wire code:", "other error");
  if (error_code == 5) LOG_DEBUG("wire code:", "timeout");
}

void lightEngine_i2c() {
  for (int i = 0; i < LIGHTS_COUNT_i2c; i++) {
    if (light_state_i2c[i]) {
      if (bri_i2c[i] != current_bri_i2c[i]) {
        process_lightdata_i2c(i);
        current_bri_i2c[i] = bri_i2c[i];
      }
    } else {
      if (current_bri_i2c[i] != 0) {
        process_lightdata_i2c(i);
        current_bri_i2c[i] = 0;
      }
    }
  }
}

void request_lightdata(uint8_t light) {
  light_rec = Wire.requestFrom(lightadress_i2c[light], 2, 1);
  byte buff[2];
  Wire.readBytes(buff, 2);
  if (light_rec > 0) {
    bri_i2c[light] = buff[0];
    light_state_i2c[light] = buff[1];
    //current_bri_i2c[light] = bri_i2c[light];

    rec = light_rec;

    if (debug_light == light) {
      debug_light = 0x7F;
      debug_code = 0x7F;
    }

    LOG_DEBUG("Light:", light);
    LOG_DEBUG("bri:", bri_i2c[light]);
    LOG_DEBUG("state:", light_state_i2c[light]);
  } else {
    rec = 0;
    debug_light = light;
    LOG_ERROR("Light:", light, "no response");
  }
}

void i2c_setup() {
  Wire.begin();
  LOG_DEBUG("Setup I2C");

  for (int i = 0; i < LIGHTS_COUNT_i2c; i++) {
    request_lightdata(i);
  }

  server_i2c.on("/state", HTTP_PUT, []() { // HTTP PUT request used to set a new light state
    DynamicJsonDocument root(1024);
    DeserializationError error = deserializeJson(root, server_i2c.arg("plain"));

    if (error) {
      server_i2c.send(404, "text/plain", "FAIL. " + server_i2c.arg("plain"));
    } else {
      for (JsonPair state : root.as<JsonObject>()) {
        const char* key = state.key().c_str();
        int light = atoi(key) - 1;
        JsonObject values = state.value();
        transitiontime_i2c = 4;

        if (values.containsKey("on")) {
          if (values["on"]) {
            light_state_i2c[light] = true;
            if (EEPROM.read(1) == 0 && EEPROM.read(0) == 0) {
              EEPROM.write(0, 1);
            }
          } else {
            light_state_i2c[light] = false;
            if (EEPROM.read(1) == 0 && EEPROM.read(0) == 1) {
              EEPROM.write(0, 0);
            }
          }
        }

        if (values.containsKey("bri")) {
          bri_i2c[light] = values["bri"];
        }

        if (values.containsKey("bri_inc")) {
          bri_i2c[light] += (int) values["bri_inc"];
          if (bri_i2c[light] > 255) bri_i2c[light] = 255;
          else if (bri_i2c[light] < 1) bri_i2c[light] = 1;
        }

        if (values.containsKey("alert") && values["alert"] == "select") {
          send_alert(light);
        }

        if (values.containsKey("transitiontime")) {
          transitiontime_i2c = values["transitiontime"];
        }
        //process_lightdata_i2c(light, transitiontime);
      }
      String output;
      serializeJson(root, output);
      LOG_DEBUG("/state put", output);
      server_i2c.send(200, "text/plain", output);
    }
  });

  server_i2c.on("/state", HTTP_GET, []() { // HTTP GET request used to fetch current light state
    uint8_t light = server_i2c.arg("light").toInt() - 1;
    DynamicJsonDocument root(1024);
    root["on"] = light_state_i2c[light];
    root["bri"] = bri_i2c[light];
    String output;
    serializeJson(root, output);
    LOG_DEBUG("/state get", output);
    LOG_DEBUG("light :", light);
    server_i2c.send(200, "text/plain", output);
  });


  server_i2c.on("/detect", []() { // HTTP GET request used to discover the light type
    char macString[32] = {0};
    sprintf(macString, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    DynamicJsonDocument root(1024);
    root["name"] = light_name_i2c;
    root["lights"] = LIGHTS_COUNT_i2c;
    root["protocol"] = "native_multi";
    root["modelid"] = "LWB010";
    root["type"] = "dimmable_light";
    root["mac"] = String(macString);
    root["version"] = LIGHT_VERSION_i2c;
    String output;
    serializeJson(root, output);
    server_i2c.send(200, "text/plain", output);
  });

  server_i2c.on("/", []() {
    transitiontime_i2c = 4;
    if (server_i2c.hasArg("startup")) {
      if (  EEPROM.read(1) != server_i2c.arg("startup").toInt()) {
        EEPROM.write(1, server_i2c.arg("startup").toInt());
        EEPROM.commit();
      }
    }

    for (int light = 0; light < LIGHTS_COUNT_i2c; light++) {
      if (server_i2c.hasArg("scene")) {
        if (server_i2c.arg("bri") == "" && server_i2c.arg("hue") == "" && server_i2c.arg("ct") == "" && server_i2c.arg("sat") == "") {
          if (  EEPROM.read(2) != server_i2c.arg("scene").toInt()) {
            EEPROM.write(2, server_i2c.arg("scene").toInt());
            EEPROM.commit();
          }
          apply_scene_i2c(server_i2c.arg("scene").toInt(), light);
        } else {
          if (server_i2c.arg("bri") != "") {
            bri_i2c[light] = server_i2c.arg("bri").toInt();
          }
        }
      } else if (server_i2c.hasArg("on")) {
        if (server_i2c.arg("on") == "true") {
          light_state_i2c[light] = true; {
            if (EEPROM.read(1) == 0 && EEPROM.read(0) == 0) {
              EEPROM.write(0, 1);
            }
          }
        } else {
          light_state_i2c[light] = false;
          if (EEPROM.read(1) == 0 && EEPROM.read(0) == 1) {
            EEPROM.write(0, 0);
          }
        }
        EEPROM.commit();
      } else if (server_i2c.hasArg("alert")) {
        send_alert(light);
      }
      if (light_state_i2c[light]) {
        step_level_i2c[light] = ((float)bri_i2c[light] - current_bri_i2c[light]) / transitiontime_i2c;
      } else {
        step_level_i2c[light] = current_bri_i2c[light] / transitiontime_i2c;
      }
    }
    if (server_i2c.hasArg("reset")) {
      ESP.restart();
    }


    String http_content = "<!doctype html>";
    http_content += "<html>";
    http_content += "<head>";
    http_content += "<meta charset=\"utf-8\">";
    http_content += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
    http_content += "<title>Light Setup</title>";
    http_content += "<link rel=\"stylesheet\" href=\"https://unpkg.com/purecss@0.6.2/build/pure-min.css\">";
    http_content += "</head>";
    http_content += "<body>";
    http_content += "<fieldset>";
    http_content += "<h3>Light Setup</h3>";
    http_content += "<form class=\"pure-form pure-form-aligned\" action=\"/\" method=\"post\">";
    http_content += "<div class=\"pure-control-group\">";
    http_content += "<label for=\"power\"><strong>Power</strong></label>";
    http_content += "<a class=\"pure-button"; if (light_state_i2c[0]) http_content += "  pure-button-primary"; http_content += "\" href=\"/?on=true\">ON</a>";
    http_content += "<a class=\"pure-button"; if (!light_state_i2c[0]) http_content += "  pure-button-primary"; http_content += "\" href=\"/?on=false\">OFF</a>";
    http_content += "</div>";
    http_content += "<div class=\"pure-control-group\">";
    http_content += "<label for=\"startup\">Startup</label>";
    http_content += "<select onchange=\"this.form.submit()\" id=\"startup\" name=\"startup\">";
    http_content += "<option "; if (EEPROM.read(1) == 0) http_content += "selected=\"selected\""; http_content += " value=\"0\">Last state</option>";
    http_content += "<option "; if (EEPROM.read(1) == 1) http_content += "selected=\"selected\""; http_content += " value=\"1\">On</option>";
    http_content += "<option "; if (EEPROM.read(1) == 2) http_content += "selected=\"selected\""; http_content += " value=\"2\">Off</option>";
    http_content += "</select>";
    http_content += "</div>";
    http_content += "<div class=\"pure-control-group\">";
    http_content += "<label for=\"scene\">Scene</label>";
    http_content += "<select onchange = \"this.form.submit()\" id=\"scene\" name=\"scene\">";
    http_content += "<option "; if (EEPROM.read(2) == 0) http_content += "selected=\"selected\""; http_content += " value=\"0\">Relax</option>";
    http_content += "<option "; if (EEPROM.read(2) == 1) http_content += "selected=\"selected\""; http_content += " value=\"1\">Bright</option>";
    http_content += "<option "; if (EEPROM.read(2) == 2) http_content += "selected=\"selected\""; http_content += " value=\"2\">Nightly</option>";
    http_content += "</select>";
    http_content += "</div>";
    http_content += "<br>";
    http_content += "<div class=\"pure-control-group\">";
    http_content += "<label for=\"state\"><strong>State</strong></label>";
    http_content += "</div>";
    http_content += "<div class=\"pure-control-group\">";
    http_content += "<label for=\"bri\">Bri</label>";
    http_content += "<input id=\"bri\" name=\"bri\" type=\"text\" placeholder=\"" + (String)bri_i2c[0] + "\">";
    http_content += "</div>";

    http_content += "<div class=\"pure-controls\">";
    http_content += "<span class=\"pure-form-message\"><a href=\"/?alert=1\">alert</a> or <a href=\"/?reset=1\">reset</a></span>";
    http_content += "<label for=\"cb\" class=\"pure-checkbox\">";
    http_content += "</label>";
    http_content += "<button type=\"submit\" class=\"pure-button pure-button-primary\">Save</button>";
    http_content += "</div>";
    http_content += "</fieldset>";
    http_content += "</form>";
    http_content += "</body>";
    http_content += "</html>";


    server_i2c.send(200, "text/html", http_content);

  });

  server_i2c.on("/reset", []() { // trigger manual reset
    server_i2c.send(200, "text/html", "reset");
    delay(1000);
    ESP.restart();
  });


  server_i2c.on("/factory", []() { // trigger manual reset
    server_i2c.send(200, "text/html", "factory reset");
    factoryReset();
  });

  server_i2c.onNotFound(handleNotFound_i2c);

  server_i2c.begin();
}

void i2c_loop() {
  server_i2c.handleClient();
  lightEngine_i2c();
  //i2c_http_loop();

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= LIGHT_interval) {
    previousMillis = currentMillis;
    for (int i = 0; i < LIGHTS_COUNT_i2c; i++) {
      request_lightdata(i);
    }
  }
}
