#include "ws2811.h"

HTTPUpdateServer httpUpdateServer;

extern byte mac[];

struct state {
  uint8_t colors[3], bri = 100, sat = 254, colorMode = 2;
  bool lightState;
  int ct = 200, hue;
  float stepLevel[3], currentColors[3], x, y;
};

state lights[10];
bool inTransition, entertainmentRun, mosftetState, useDhcp = true;
byte packetBuffer[46];
unsigned long lastEPMillis;

//settings
char lightName[LIGHT_NAME_MAX_LENGTH] = LIGHT_NAME_WS2811;
uint8_t effect, scene, startup, onPin = LIGHT_ONPIN_WS, offPin = LIGHT_OFFPIN_WS;
bool hwSwitch = false;
uint8_t rgb_multiplier[] = {100, 100, 100}; // light multiplier in percentage /R, G, B/

uint16_t dividedLightsArray[30];

uint8_t lightsCount = LIGHT_COUNT_WS;
uint16_t pixelCount = PIXEL_COUNT_WS;
uint8_t transitionLeds = TRANSITION_LEDS_WS; // pixelCount must be divisible by this value

WebServer server_ws(LIGHT_PORT_WS);
WiFiUDP Udp;

NeoPixelBus<NeoRgbFeature, NeoEsp32Rmt1Ws2812xMethod>* strip = NULL;

void handleNotFound_ws() { // default webserver response for unknow requests
  String message;
  message.reserve(200); // Pre-allocate to reduce memory fragmentation
  message = "File Not Found\n\n";
  message += "URI: ";
  message += server_ws.uri();
  message += "\nMethod: ";
  message += (server_ws.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server_ws.args();
  message += "\n";
  for (uint8_t i = 0; i < server_ws.args(); i++) {
    message += " " + server_ws.argName(i) + ": " + server_ws.arg(i) + "\n";
  }
  server_ws.send(404, "text/plain", message);
}

void apply_scene_ws(uint8_t new_scene) { // these are internal scenes store in light firmware that can be applied on boot and manually from light web interface
  for (uint8_t light = 0; light < lightsCount; light++) {
    if ( new_scene == 1) {
      lights[light].bri = 254; lights[light].ct = 346; lights[light].colorMode = 2; convertCt(light);
    } else if ( new_scene == 2) {
      lights[light].bri = 254; lights[light].ct = 233; lights[light].colorMode = 2; convertCt(light);
    }  else if ( new_scene == 3) {
      lights[light].bri = 254; lights[light].ct = 156; lights[light].colorMode = 2; convertCt(light);
    }  else if ( new_scene == 4) {
      lights[light].bri = 77; lights[light].ct = 367; lights[light].colorMode = 2; convertCt(light);
    }  else if ( new_scene == 5) {
      lights[light].bri = 254; lights[light].ct = 447; lights[light].colorMode = 2; convertCt(light);
    }  else if ( new_scene == 6) {
      lights[light].bri = 1; lights[light].x = 0.561; lights[light].y = 0.4042; lights[light].colorMode = 1; convertXy(light);
    }  else if ( new_scene == 7) {
      lights[light].bri = 203; lights[light].x = 0.380328; lights[light].y = 0.39986; lights[light].colorMode = 1; convertXy(light);
    }  else if ( new_scene == 8) {
      lights[light].bri = 112; lights[light].x = 0.359168; lights[light].y = 0.28807; lights[light].colorMode = 1; convertXy(light);
    }  else if ( new_scene == 9) {
      lights[light].bri = 142; lights[light].x = 0.267102; lights[light].y = 0.23755; lights[light].colorMode = 1; convertXy(light);
    }  else if ( new_scene == 10) {
      lights[light].bri = 216; lights[light].x = 0.393209; lights[light].y = 0.29961; lights[light].colorMode = 1; convertXy(light);
    } else {
      lights[light].bri = 144; lights[light].ct = 447; lights[light].colorMode = 2; convertCt(light);
    }
  }
}

void processLightdata(uint8_t light, float transitiontime) { // calculate the step level of every RGB channel for a smooth transition in requested transition time
  transitiontime *= 14 - (pixelCount / 70); //every extra led add a small delay that need to be counted for transition time match
  if (lights[light].colorMode == 1 && lights[light].lightState == true) {
    convertXy(light);
  } else if (lights[light].colorMode == 2 && lights[light].lightState == true) {
    convertCt(light);
  } else if (lights[light].colorMode == 3 && lights[light].lightState == true) {
    convertHue(light);
  }
  for (uint8_t i = 0; i < 3; i++) {
    if (lights[light].lightState) {
      lights[light].stepLevel[i] = ((float)lights[light].colors[i] - lights[light].currentColors[i]) / transitiontime;
    } else {
      lights[light].stepLevel[i] = lights[light].currentColors[i] / transitiontime;
    }
  }
}

RgbColor blending(float left[3], float right[3], uint8_t pixel) { // return RgbColor based on neighbour leds
  uint8_t result[3];
  for (uint8_t i = 0; i < 3; i++) {
    float percent = (float) pixel / (float) (transitionLeds + 1);
    result[i] = (left[i] * (1.0f - percent) + right[i] * percent);
  }
  return RgbColor((uint8_t)result[0], (uint8_t)result[1], (uint8_t)result[2]);
}

void candleEffect() {
  for (uint8_t light = 0; light < lightsCount; light++) {
    lights[light].colors[0] = random(170, 254);
    lights[light].colors[1] = random(37, 62);
    lights[light].colors[2] = 0;
    for (uint8_t i = 0; i < 3; i++) {
      lights[light].stepLevel[i] = ((float)lights[light].colors[i] - lights[light].currentColors[i]) / random(5, 15);
    }
  }
}

void firePlaceEffect() {
    for (uint8_t light = 0; light < lightsCount; light++) {
    lights[light].colors[0] = random(100, 254);
    lights[light].colors[1] = random(10, 35);
    lights[light].colors[2] = 0;
    for (uint8_t i = 0; i < 3; i++) {
      lights[light].stepLevel[i] = ((float)lights[light].colors[i] - lights[light].currentColors[i]) / random(5, 15);
    }
  }
}

RgbColor convFloat(float color[3]) { // return RgbColor from float
  return RgbColor((uint8_t)color[0], (uint8_t)color[1], (uint8_t)color[2]);
}

void cutPower() {
  bool any_on = false;
  for (int light = 0; light < lightsCount; light++) {
    if (lights[light].lightState) {
      any_on = true;
    }
  }
  if (!any_on && !inTransition && mosftetState) {
    //digitalWrite(POWER_MOSFET_PIN, LOW);
    mosftetState = false;
  } else if (any_on && !mosftetState) {
    //digitalWrite(POWER_MOSFET_PIN, HIGH);
    mosftetState = true;
  }
}

void lightEngine() {  // core function executed in loop()
  for (int light = 0; light < lightsCount; light++) { // loop with every virtual light
    if (lights[light].lightState) { // if light in on
      if (lights[light].colors[0] != lights[light].currentColors[0] || lights[light].colors[1] != lights[light].currentColors[1] || lights[light].colors[2] != lights[light].currentColors[2]) { // if not all RGB channels of the light are at desired level
        inTransition = true;
        for (uint8_t k = 0; k < 3; k++) { // loop with every RGB channel of the light
          if (lights[light].colors[k] != lights[light].currentColors[k]) lights[light].currentColors[k] += lights[light].stepLevel[k]; // move RGB channel on step closer to desired level
          if ((lights[light].stepLevel[k] > 0.0 && lights[light].currentColors[k] > lights[light].colors[k]) || (lights[light].stepLevel[k] < 0.0 && lights[light].currentColors[k] < lights[light].colors[k])) lights[light].currentColors[k] = lights[light].colors[k]; // if the current level go below desired level apply directly the desired level.
        }
        if (lightsCount > 1) { // if are more then 1 virtual light we need to apply transition leds (set in the web interface)
          if (light == 0) { // if is the first light we must not have transition leds at the beginning
            for (int pixel = 0; pixel < dividedLightsArray[0]; pixel++) // loop with all leds of the light (declared in web interface)
            {
              if (pixel < dividedLightsArray[0] - transitionLeds / 2) { // apply raw color if we are outside transition leds
                strip->SetPixelColor(pixel, convFloat(lights[light].currentColors));
              } else {
                strip->SetPixelColor(pixel, blending(lights[0].currentColors, lights[1].currentColors, pixel + 1 - (dividedLightsArray[0] - transitionLeds / 2 ))); // calculate the transition led color
              }
            }
          }
          else { // is not the first virtual light
            for (int pixel = 0; pixel < dividedLightsArray[light]; pixel++) // loop with all leds of the light
            {
              long pixelSum;
              for (int value = 0; value < light; value++)
              {
                if (value + 1 == light) {
                  pixelSum += dividedLightsArray[value] - transitionLeds;
                }
                else {
                  pixelSum += dividedLightsArray[value];
                }
              }

              if (pixel < transitionLeds / 2) { // beginning transition leds
                strip->SetPixelColor(pixel + pixelSum + transitionLeds, blending( lights[light - 1].currentColors, lights[light].currentColors, pixel + 1));
              }
              else if (pixel > dividedLightsArray[light] - transitionLeds / 2 - 1) {  // end of transition leds
                //Serial.println(String(pixel));
                strip->SetPixelColor(pixel + pixelSum + transitionLeds, blending( lights[light].currentColors, lights[light + 1].currentColors, pixel + transitionLeds / 2 - dividedLightsArray[light]));
              }
              else  { // outside transition leds (apply raw color)
                strip->SetPixelColor(pixel + pixelSum + transitionLeds, convFloat(lights[light].currentColors));
              }
              pixelSum = 0;
            }
          }
        } else { // strip has only one virtual light so apply raw color to entire strip
          strip->ClearTo(convFloat(lights[light].currentColors), 0, pixelCount - 1);
        }
        strip->Show(); //show what was calculated previously
      }
    } else { // if light in off, calculate the dimming effect only
      if (lights[light].currentColors[0] != 0 || lights[light].currentColors[1] != 0 || lights[light].currentColors[2] != 0) { // proceed forward only in case not all RGB channels are zero
        inTransition = true;
        for (uint8_t k = 0; k < 3; k++) { //loop with every RGB channel
          if (lights[light].currentColors[k] != 0) lights[light].currentColors[k] -= lights[light].stepLevel[k]; // remove one step level
          if (lights[light].currentColors[k] < 0) lights[light].currentColors[k] = 0; // save condition, if level go below zero set it to zero
        }
        if (lightsCount > 1) { // if the strip has more than one light
          if (light == 0) { // if is the first light of the strip
            for (int pixel = 0; pixel < dividedLightsArray[0]; pixel++) // loop with every led of the virtual light
            {
              if (pixel < dividedLightsArray[0] - transitionLeds / 2) { // leds until transition zone apply raw color
                strip->SetPixelColor(pixel, convFloat(lights[light].currentColors));
              } else { // leds in transition zone apply the transition color
                strip->SetPixelColor(pixel, blending(lights[0].currentColors, lights[1].currentColors, pixel + 1 - (dividedLightsArray[0] - transitionLeds / 2 )));
              }
            }
          }
          else { // is not the first light
            for (int pixel = 0; pixel < dividedLightsArray[light]; pixel++) // loop with every led
            {
              long pixelSum;
              for (int value = 0; value < light; value++)
              {
                if (value + 1 == light) {
                  pixelSum += dividedLightsArray[value] - transitionLeds;
                }
                else {
                  pixelSum += dividedLightsArray[value];
                }
              }

              if (pixel < transitionLeds / 2) { // leds in beginning of transition zone must apply blending
                strip->SetPixelColor(pixel + pixelSum + transitionLeds, blending( lights[light - 1].currentColors, lights[light].currentColors, pixel + 1));
              }
              else if (pixel > dividedLightsArray[light] - transitionLeds / 2 - 1) { // leds in the end of transition zone must apply blending
                //Serial.println(String(pixel));
                strip->SetPixelColor(pixel + pixelSum + transitionLeds, blending( lights[light].currentColors, lights[light + 1].currentColors, pixel + transitionLeds / 2 - dividedLightsArray[light]));
              }
              else  { // leds outside transition zone apply raw color
                strip->SetPixelColor(pixel + pixelSum + transitionLeds, convFloat(lights[light].currentColors));
              }
              pixelSum = 0;
            }
          }
        } else { // is just one virtual light declared, apply raw color to all leds
          strip->ClearTo(convFloat(lights[light].currentColors), 0, pixelCount - 1);
        }
        strip->Show();
      }
    }
  }
  cutPower(); // if all lights are off GPIO12 can cut the power to the strip using a powerful P-Channel MOSFET
  if (inTransition) { // wait 6ms for a nice transition effect
    delay(6);
    inTransition = false; // set inTransition bash to false (will be set bach to true on next level execution if desired state is not reached)
  } else {
    if (effect == 1) { // candle effect
        candleEffect();
      } else if (effect == 2) { // fireplace effect
        firePlaceEffect();
    }
    if (hwSwitch == true) { // if you want to use some GPIO's for on/off and brightness controll
      if (digitalRead(onPin) == LOW) { // on button pressed
        int i = 0;
        while (digitalRead(onPin) == LOW && i < 30) { // count how log is the button pressed
          delay(20);
          i++;
        }
        for (int light = 0; light < lightsCount; light++) {
          if (i < 30) { // there was a short press
            lights[light].lightState = true;
          }
          else { // there was a long press
            if (lights[light].bri < 198) {
              lights[light].bri += 56;
            } else {
              lights[light].bri = 254;
            }
            processLightdata(light);
          }
        }
      } else if (digitalRead(offPin) == LOW) { // off button pressed
        int i = 0;
        while (digitalRead(offPin) == LOW && i < 30) {
          delay(20);
          i++;
        }
        for (int light = 0; light < lightsCount; light++) {
          if (i < 30) {
            // there was a short press
            lights[light].lightState = false;
          }
          else {
            // there was a long press
            if (lights[light].bri > 57) {
              lights[light].bri -= 56;
            } else {
              lights[light].bri = 1;
            }
            processLightdata(light);
          }
        }
      }
    }
  }
}

void saveState() { // save the lights state on LittleFS partition in JSON format
  LOG_DEBUG("save state");
  DynamicJsonDocument json(1024);
  for (uint8_t i = 0; i < lightsCount; i++) {
    JsonObject light = json.createNestedObject((String)i);
    light["on"] = lights[i].lightState;
    light["bri"] = lights[i].bri;
    if (lights[i].colorMode == 1) {
      light["x"] = lights[i].x;
      light["y"] = lights[i].y;
    } else if (lights[i].colorMode == 2) {
      light["ct"] = lights[i].ct;
    } else if (lights[i].colorMode == 3) {
      light["hue"] = lights[i].hue;
      light["sat"] = lights[i].sat;
    }
  }
  File stateFile = LittleFS.open("/state.json", "w");
  serializeJson(json, stateFile);

}

void restoreState() { // restore the lights state from LittleFS partition
  LOG_DEBUG("restore state");
  File stateFile = LittleFS.open("/state.json", "r");
  if (!stateFile) {
    saveState();
    return;
  }

  DynamicJsonDocument json(1024);
  DeserializationError error = deserializeJson(json, stateFile.readString());
  if (error) {
    LOG_DEBUG("Failed to parse config file");
    return;
  }
  for (JsonPair state : json.as<JsonObject>()) {
    const char* key = state.key().c_str();
    int lightId = atoi(key);
    JsonObject values = state.value();
    lights[lightId].lightState = values["on"];
    lights[lightId].bri = (uint8_t)values["bri"];
    if (values.containsKey("x")) {
      lights[lightId].x = values["x"];
      lights[lightId].y = values["y"];
      lights[lightId].colorMode = 1;
    } else if (values.containsKey("ct")) {
      lights[lightId].ct = values["ct"];
      lights[lightId].colorMode = 2;
    } else {
      if (values.containsKey("hue")) {
        lights[lightId].hue = values["hue"];
        lights[lightId].colorMode = 3;
      }
      if (values.containsKey("sat")) {
        lights[lightId].sat = (uint8_t) values["sat"];
        lights[lightId].colorMode = 3;
      }
    }
  }
}

bool saveConfig() { // save config in LittleFS partition in JSON file
  DynamicJsonDocument json(1024);
  json["name"] = lightName;
  json["startup"] = startup;
  json["scene"] = scene;
  json["on"] = onPin;
  json["off"] = offPin;
  json["hw"] = hwSwitch;
  json["dhcp"] = useDhcp;
  json["lightsCount"] = lightsCount;
  for (uint16_t i = 0; i < lightsCount; i++) {
    json["dividedLight_" + String(i)] = dividedLightsArray[i];
  }
  json["pixelCount"] = pixelCount;
  json["transLeds"] = transitionLeds;
  json["rpct"] = rgb_multiplier[0];
  json["gpct"] = rgb_multiplier[1];
  json["bpct"] = rgb_multiplier[2];
  File configFile = LittleFS.open("/config.json", "w");
  if (!configFile) {
    LOG_DEBUG("Failed to open config file for writing");
    return false;
  }

  serializeJson(json, configFile);
  return true;
}

bool loadConfig() { // load the configuration from LittleFS partition
  LOG_DEBUG("loadConfig file");
  File configFile = LittleFS.open("/config.json", "r");
  if (!configFile) {
    LOG_DEBUG("Create new file with default values");
    return saveConfig();
  }

  size_t size = configFile.size();
  if (size > 1024) {
    LOG_DEBUG("Config file size is too large");
    return false;
  }

  DynamicJsonDocument json(1024);
  DeserializationError error = deserializeJson(json, configFile.readString());
  if (error) {
    LOG_DEBUG("Failed to parse config file");
    return false;
  }

  strcpy(lightName, json["name"]);
  startup = (uint8_t) json["startup"];
  scene  = (uint8_t) json["scene"];
  onPin = (uint8_t) json["on"];
  offPin = (uint8_t) json["off"];
  hwSwitch = json["hw"];
  lightsCount = (uint16_t) json["lightsCount"];
  for (uint16_t i = 0; i < lightsCount; i++) {
    dividedLightsArray[i] = (uint16_t) json["dividedLight_" + String(i)];
  }
  pixelCount = (uint16_t) json["pixelCount"];
  transitionLeds = (uint8_t) json["transLeds"];
  if (json.containsKey("rpct")) {
    rgb_multiplier[0] = (uint8_t) json["rpct"];
    rgb_multiplier[1] = (uint8_t) json["gpct"];
    rgb_multiplier[2] = (uint8_t) json["bpct"];
  }
  useDhcp = json["dhcp"];
  return true;
}

void ChangeNeoPixels(uint16_t newCount) // this set the number of leds of the strip based on web configuration
{
  if (strip != NULL) {
    delete strip; // delete the previous dynamically created strip
  }
  strip = new NeoPixelBus<NeoRgbFeature, NeoEsp32Rmt1Ws2812xMethod>(newCount, DATA_PIN); // and recreate with new count
  strip->Begin();
}

void ws_setup() {
  LOG_DEBUG("Setup WS2811");
  //pinMode(POWER_MOSFET_PIN, OUTPUT);
  blinkLed(2);
  //digitalWrite(POWER_MOSFET_PIN, HIGH); mosftetState = true; // reuired if HIGH logic power the strip, otherwise must be commented.

  /*if (!LittleFS.begin()) {
    LOG_DEBUG("Failed to mount file system");
    //Serial.println("Failed to mount file system");
    LittleFS.format();
    }*/

  if (!loadConfig()) {
    LOG_DEBUG("Failed to load config");
  } else {
    LOG_DEBUG("Config loaded");
  }

  dividedLightsArray[lightsCount];
  if (dividedLightsArray[0] == 0) {
    for (uint8_t light = 0; light < lightsCount; light++) {
      dividedLightsArray[light] = pixelCount / lightsCount;
    }
    saveConfig();
  }

  ChangeNeoPixels(pixelCount);

  if (startup == 1) {
    for (uint8_t i = 0; i < lightsCount; i++) {
      lights[i].lightState = true;
    }
  }
  if (startup == 0) {
    restoreState();
  } else {
    apply_scene_ws(scene);
  }
  for (uint8_t i = 0; i < lightsCount; i++) {
    processLightdata(i);
  }
  if (lights[0].lightState) {
    for (uint8_t i = 0; i < 200; i++) {
      lightEngine();
    }
  }

  Udp.begin(2100); // start entertainment UDP server

  server_ws.on("/state", HTTP_PUT, []() { // HTTP PUT request used to set a new light state
    bool stateSave = false;
    DynamicJsonDocument root(1024);
    DeserializationError error = deserializeJson(root, server_ws.arg("plain"));

    if (error) {
      server_ws.send(404, "text/plain", "FAIL. " + server_ws.arg("plain"));
    } else {
      for (JsonPair state : root.as<JsonObject>()) {
        const char* key = state.key().c_str();
        int light = atoi(key) - 1;
        JsonObject values = state.value();
        int transitiontime = 4;

        if (values.containsKey("effect")) {
          if (values["effect"] == "no_effect") {
            effect = 0;
          } else if (values["effect"] == "candle") {
            effect = 1;
          } else if (values["effect"] == "fire") {
            effect = 2;
          }
        }

        if (values.containsKey("xy")) {
          lights[light].x = values["xy"][0];
          lights[light].y = values["xy"][1];
          lights[light].colorMode = 1;
        } else if (values.containsKey("ct")) {
          lights[light].ct = values["ct"];
          lights[light].colorMode = 2;
        } else {
          if (values.containsKey("hue")) {
            lights[light].hue = values["hue"];
            lights[light].colorMode = 3;
          }
          if (values.containsKey("sat")) {
            lights[light].sat = values["sat"];
            lights[light].colorMode = 3;
          }
        }

        if (values.containsKey("on")) {
          if (values["on"]) {
            lights[light].lightState = true;
          } else {
            lights[light].lightState = false;
          }
          if (startup == 0) {
            stateSave = true;
          }
        }

        if (values.containsKey("bri")) {
          lights[light].bri = values["bri"];
        }

        if (values.containsKey("bri_inc")) {
          if (values["bri_inc"] > 0) {
            if (lights[light].bri + (int) values["bri_inc"] > 254) {
              lights[light].bri = 254;
            } else {
              lights[light].bri += (int) values["bri_inc"];
            }
          } else {
            if (lights[light].bri - (int) values["bri_inc"] < 1) {
              lights[light].bri = 1;
            } else {
              lights[light].bri += (int) values["bri_inc"];
            }
          }
        }

        if (values.containsKey("transitiontime")) {
          transitiontime = values["transitiontime"];
        }

        if (values.containsKey("alert") && values["alert"] == "select") {
          if (lights[light].lightState) {
            lights[light].currentColors[0] = 0; lights[light].currentColors[1] = 0; lights[light].currentColors[2] = 0;
          } else {
            lights[light].currentColors[1] = 126; lights[light].currentColors[2] = 126;
          }
        }
        processLightdata(light, transitiontime);
      }
      String output;
      serializeJson(root, output);
      server_ws.send(200, "text/plain", output);
      if (stateSave) {
        saveState();
      }
    }
  });

  server_ws.on("/state", HTTP_GET, []() { // HTTP GET request used to fetch current light state
    uint8_t light = server_ws.arg("light").toInt() - 1;
    DynamicJsonDocument root(1024);
    root["on"] = lights[light].lightState;
    root["bri"] = lights[light].bri;
    JsonArray xy = root.createNestedArray("xy");
    xy.add(lights[light].x);
    xy.add(lights[light].y);
    root["ct"] = lights[light].ct;
    root["hue"] = lights[light].hue;
    root["sat"] = lights[light].sat;
    if (lights[light].colorMode == 1)
      root["colormode"] = "xy";
    else if (lights[light].colorMode == 2)
      root["colormode"] = "ct";
    else if (lights[light].colorMode == 3)
      root["colormode"] = "hs";
    String output;
    serializeJson(root, output);
    server_ws.send(200, "text/plain", output);
  });

  server_ws.on("/detect", []() { // HTTP GET request used to discover the light type
    char macString[32] = {0};
    sprintf(macString, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    DynamicJsonDocument root(1024);
    root["name"] = lightName;
    root["lights"] = lightsCount;
    root["protocol"] = LIGHT_PROTOCOL_WS;
    root["modelid"] = LIGHT_MODEL_WS;
    root["type"] = LIGHT_TYPE_WS;
    root["mac"] = String(macString);
    root["version"] = LIGHT_VERSION;
    String output;
    serializeJson(root, output);
    server_ws.send(200, "text/plain", output);
  });

  server_ws.on("/config", []() { // used by light web interface to get current configuration
    DynamicJsonDocument root(1024);
    root["name"] = lightName;
    root["scene"] = scene;
    root["startup"] = startup;
    root["hw"] = hwSwitch;
    root["on"] = onPin;
    root["off"] = offPin;
    root["hwswitch"] = (int)hwSwitch;
    root["lightscount"] = lightsCount;
    for (uint8_t i = 0; i < lightsCount; i++) {
      root["dividedLight_" + String(i)] = (int)dividedLightsArray[i];
    }
    root["pixelcount"] = pixelCount;
    root["transitionleds"] = transitionLeds;
    root["rpct"] = rgb_multiplier[0];
    root["gpct"] = rgb_multiplier[1];
    root["bpct"] = rgb_multiplier[2];
    root["disdhcp"] = (int)!useDhcp;
    String output;
    serializeJson(root, output);
    server_ws.send(200, "text/plain", output);
  });

  server_ws.on("/", []() { // light http web interface
    if (server_ws.arg("section").toInt() == 1) {
      server_ws.arg("name").toCharArray(lightName, LIGHT_NAME_MAX_LENGTH);
      startup = server_ws.arg("startup").toInt();
      scene = server_ws.arg("scene").toInt();
      lightsCount = server_ws.arg("lightscount").toInt();
      pixelCount = server_ws.arg("pixelcount").toInt();
      transitionLeds = server_ws.arg("transitionleds").toInt();
      rgb_multiplier[0] = server_ws.arg("rpct").toInt();
      rgb_multiplier[1] = server_ws.arg("gpct").toInt();
      rgb_multiplier[2] = server_ws.arg("bpct").toInt();
      for (uint16_t i = 0; i < lightsCount; i++) {
        dividedLightsArray[i] = server_ws.arg("dividedLight_" + String(i)).toInt();
      }
      hwSwitch = server_ws.hasArg("hwswitch") ? server_ws.arg("hwswitch").toInt() : 0;
      if (server_ws.hasArg("hwswitch")) {
        onPin = server_ws.arg("on").toInt();
        offPin = server_ws.arg("off").toInt();
      }
      saveConfig();
    } else if (server_ws.arg("section").toInt() == 2) {
      useDhcp = (!server_ws.hasArg("disdhcp")) ? 1 : server_ws.arg("disdhcp").toInt();
      saveConfig();
    }

    const char htmlContent[] PROGMEM = R"=====(<!DOCTYPE html> <html> <head> <meta charset="UTF-8"> <meta name="viewport" content="width=device-width, initial-scale=1"> <title>DiyHue</title> <link rel="icon" type="image/png" href="https://raw.githubusercontent.com/diyhue/Lights/master/HTML/diyhue.png" sizes="32x32"> <link href="https://fonts.googleapis.com/icon?family=Material+Icons" rel="stylesheet"> <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/materialize/1.0.0/css/materialize.min.css"> <link rel="stylesheet" href="https://github.com/diyhue/Lights/raw/refs/heads/master/HTML/nouislider.css" /> </head> <body> <div class="wrapper"> <nav class="nav-extended row" style="background-color: #26a69a !important;"> <div class="nav-wrapper col s12"> <a href="#" class="brand-logo">DiyHue</a> <ul id="nav-mobile" class="right hide-on-med-and-down" style="position: relative;z-index: 10;"> <li><a target="_blank" href="https://github.com/diyhue"><i class="material-icons left">language</i>GitHub</a></li> <li><a target="_blank" href="https://diyhue.readthedocs.io/en/latest/"><i class="material-icons left">description</i>Documentation</a></li> <li><a target="_blank" href="https://diyhue.slack.com/"><i class="material-icons left">question_answer</i>Slack channel</a></li> </ul> </div> <div class="nav-content"> <ul class="tabs tabs-transparent"> <li class="tab" title="#home"><a class="active" href="#home">Home</a></li> <li class="tab" title="#preferences"><a href="#preferences">Preferences</a></li> <li class="tab" title="#network"><a href="#network">Network settings</a></li> <li class="tab" title="/update"><a href="/update">Updater</a></li> </ul> </div> </nav> <ul class="sidenav" id="mobile-demo"> <li><a target="_blank" href="https://github.com/diyhue">GitHub</a></li> <li><a target="_blank" href="https://diyhue.readthedocs.io/en/latest/">Documentation</a></li> <li><a target="_blank" href="https://diyhue.slack.com/">Slack channel</a></li> </ul> <div class="container"> <div class="section"> <div id="home" class="col s12"> <form> <input type="hidden" name="section" value="1"> <div class="row"> <div class="col s10"> <label for="power">Power</label> <div id="power" class="switch section"> <label> Off <input type="checkbox" name="pow" id="pow" value="1"> <span class="lever"></span> On </label> </div> </div> </div> <div class="row"> <div class="col s12 m10"> <label for="bri">Brightness</label> <input type="text" id="bri" class="js-range-slider" name="bri" value="" /> </div> </div> <div class="row"> <div class="col s12"> <label for="hue">Color</label> <div> <canvas id="hue" width="320px" height="320px" style="border:1px solid #d3d3d3;"></canvas> </div> </div> </div> <div class="row"> <div class="col s12"> <label for="ct">Color Temp</label> <div> <canvas id="ct" width="320px" height="50px" style="border:1px solid #d3d3d3;"></canvas> </div> </div> </div> </form> </div> <div id="preferences" class="col s12"> <form method="POST" action="/"> <input type="hidden" name="section" value="1"> <div class="row"> <div class="col s12"> <label for="name">Light Name</label> <input type="text" id="name" name="name"> </div> </div> <div class="row"> <div class="col s12 m6"> <label for="startup">Default Power:</label> <select name="startup" id="startup"> <option value="0">Last State</option> <option value="1">On</option> <option value="2">Off</option> </select> </div> </div> <div class="row"> <div class="col s12 m6"> <label for="scene">Default Scene:</label> <select name="scene" id="scene"> <option value="0">Relax</option> <option value="1">Read</option> <option value="2">Concentrate</option> <option value="3">Energize</option> <option value="4">Bright</option> <option value="5">Dimmed</option> <option value="6">Nightlight</option> <option value="7">Savanna sunset</option> <option value="8">Tropical twilight</option> <option value="9">Arctic aurora</option> <option value="10">Spring blossom</option> </select> </div> </div> <div class="row"> <div class="col s4 m3"> <label for="pixelcount" class="col-form-label">Pixel count</label> <input type="number" id="pixelcount" name="pixelcount"> </div> </div> <div class="row"> <div class="col s4 m3"> <label for="lightscount" class="col-form-label">Lights count</label> <input type="number" id="lightscount" name="lightscount"> </div> </div> <label class="form-label">Light division</label> </br> <label>Available Pixels:</label> <label class="availablepixels"><b>null</b></label> <div class="row dividedLights"> </div> <div class="row"> <div class="col s4 m3"> <label for="transitionleds">Transition leds:</label> <select name="transitionleds" id="transitionleds"> <option value="0">0</option> <option value="2">2</option> <option value="4">4</option> <option value="6">6</option> <option value="8">8</option> <option value="10">10</option> </select> </div> </div> <div class="row"> <div class="col s4 m3"> <label for="rpct" class="form-label">Red multiplier</label> <input type="number" id="rpct" class="js-range-slider" data-skin="round" name="rpct" value="" /> </div> <div class="col s4 m3"> <label for="gpct" class="form-label">Green multiplier</label> <input type="number" id="gpct" class="js-range-slider" data-skin="round" name="gpct" value="" /> </div> <div class="col s4 m3"> <label for="bpct" class="form-label">Blue multiplier</label> <input type="number" id="bpct" class="js-range-slider" data-skin="round" name="bpct" value="" /> </div> </div> <div class="row"> <label class="control-label col s10">HW buttons:</label> <div class="col s10"> <div class="switch section"> <label> Disable <input type="checkbox" name="hwswitch" id="hwswitch" value="1"> <span class="lever"></span> Enable </label> </div> </div> </div> <div class="switchable"> <div class="row"> <div class="col s4 m3"> <label for="on">On Pin</label> <input type="number" id="on" name="on"> </div> <div class="col s4 m3"> <label for="off">Off Pin</label> <input type="number" id="off" name="off"> </div> </div> </div> <div class="row"> <div class="col s10"> <button type="submit" class="waves-effect waves-light btn teal">Save</button> </div> </div> </form> </div> <div id="network" class="col s12"> <form method="POST" action="/"> <input type="hidden" name="section" value="2"> <div class="row"> <div class="col s12"> <label class="control-label">Manual IP assignment:</label> <div class="switch section"> <label> Disable <input type="checkbox" name="disdhcp" id="disdhcp" value="0"> <span class="lever"></span> Enable </label> </div> </div> </div> <div class="switchable"> <div class="row"> <div class="col s12 m3"> <label for="addr">Ip</label> <input type="text" id="addr" name="addr"> </div> <div class="col s12 m3"> <label for="sm">Submask</label> <input type="text" id="sm" name="sm"> </div> <div class="col s12 m3"> <label for="gw">Gateway</label> <input type="text" id="gw" name="gw"> </div> </div> </div> <div class="row"> <div class="col s10"> <button type="submit" class="waves-effect waves-light btn teal">Save</button> </div> </div> </form> </div> </div> </div> </div> <script src="https://cdnjs.cloudflare.com/ajax/libs/jquery/3.5.1/jquery.min.js"></script> <script src="https://cdnjs.cloudflare.com/ajax/libs/materialize/1.0.0/js/materialize.min.js"></script> <script src="https://github.com/diyhue/Lights/raw/refs/heads/master/HTML/nouislider.js"></script> <script src="https://github.com/diyhue/Lights/raw/refs/heads/master/HTML/diyhue.js"></script> </body> </html>)=====";

    server_ws.send_P(200, "text/html", htmlContent);
    if (server_ws.args()) {
      resetESP();
    }

  });

  server_ws.on("/reset", []() { // trigger manual reset
    server_ws.send(200, "text/html", "reset");
    resetESP();
  });

  server_ws.onNotFound(handleNotFound_ws);

  server_ws.begin();

  httpUpdateServer.setup(&server_ws); // start http server

}

RgbColor blendingEntert(float left[3], float right[3], float pixel) {
  uint8_t result[3];
  for (uint8_t i = 0; i < 3; i++) {
    float percent = (float) pixel / (float) (transitionLeds + 1);
    result[i] = (left[i] * (1.0f - percent) + right[i] * percent) / 2;
  }
  return RgbColor((uint8_t)result[0], (uint8_t)result[1], (uint8_t)result[2]);
}

void entertainment() { // entertainment function
  uint8_t packetSize = Udp.parsePacket(); // check if UDP received some bytes
  if (packetSize) { // if nr of bytes is more than zero
    if (!entertainmentRun) { // announce entertainment is running
      entertainmentRun = true;
    }
    lastEPMillis = millis(); // update variable with last received package timestamp
    Udp.read(packetBuffer, packetSize);
    for (uint8_t i = 0; i < packetSize / 4; i++) { // loop with every light. There are 4 bytes for every light (light number, red, green, blue)
      lights[packetBuffer[i * 4]].currentColors[0] = packetBuffer[i * 4 + 1] * rgb_multiplier[0] / 100;
      lights[packetBuffer[i * 4]].currentColors[1] = packetBuffer[i * 4 + 2] * rgb_multiplier[1] / 100;
      lights[packetBuffer[i * 4]].currentColors[2] = packetBuffer[i * 4 + 3] * rgb_multiplier[2] / 100;
    }
    for (uint8_t light = 0; light < lightsCount; light++) {
      if (lightsCount > 1) {
        if (light == 0) {
          for (int pixel = 0; pixel < dividedLightsArray[0]; pixel++)
          {
            if (pixel < dividedLightsArray[0] - transitionLeds / 2) {
              strip->SetPixelColor(pixel, convFloat(lights[light].currentColors));
            } else {
              strip->SetPixelColor(pixel, blendingEntert(lights[0].currentColors, lights[1].currentColors, pixel + 1 - (dividedLightsArray[0] - transitionLeds / 2 )));
            }
          }
        }
        else {
          for (int pixel = 0; pixel < dividedLightsArray[light]; pixel++)
          {
            long pixelSum;
            for (int value = 0; value < light; value++)
            {
              if (value + 1 == light) {
                pixelSum += dividedLightsArray[value] - transitionLeds;
              }
              else {
                pixelSum += dividedLightsArray[value];
              }
            }
            if (pixel < transitionLeds / 2) {
              strip->SetPixelColor(pixel + pixelSum + transitionLeds, blendingEntert( lights[light - 1].currentColors, lights[light].currentColors, pixel + 1));
            }
            else if (pixel > dividedLightsArray[light] - transitionLeds / 2 - 1) {
              //Serial.println(String(pixel));
              strip->SetPixelColor(pixel + pixelSum + transitionLeds, blendingEntert( lights[light].currentColors, lights[light + 1].currentColors, pixel + transitionLeds / 2 - dividedLightsArray[light]));
            }
            else  {
              strip->SetPixelColor(pixel + pixelSum + transitionLeds, convFloat(lights[light].currentColors));
            }
            pixelSum = 0;
          }
        }
      } else {
        strip->ClearTo(convFloat(lights[light].currentColors), 0, pixelCount - 1);
      }
    }
    strip->Show();
  }
}

void ws_loop() {
  server_ws.handleClient();
  if (!entertainmentRun) {
    lightEngine(); // process lights data set on http server
  } else {
    if ((millis() - lastEPMillis) >= ENTERTAINMENT_TIMEOUT) { // entertainment stream stop (timeout)
      entertainmentRun = false;
      for (uint8_t i = 0; i < lightsCount; i++) {
        processLightdata(i); //return to original colors with 0.4 sec transition
      }
    }
  }
  entertainment(); // process entertainment data on UDP server
}
