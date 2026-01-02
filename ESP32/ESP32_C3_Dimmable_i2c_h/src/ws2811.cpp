#include "ws2811.h"

HTTPUpdateServer httpUpdateServer;

struct state
{
  uint8_t colors[3], bri = 100, sat = 254, colorMode = 2;
  bool lightState;
  int ct = 200, hue;
  float stepLevel[3], currentColors[3], x, y;
  uint16_t dividedLights = 0;
};

state lights[MAX_RUNTIME_LIGHTS];
uint16_t lightsCapacity = 0; // currently allocated capacity
bool inTransition, entertainmentRun;
byte packetBuffer[46];
unsigned long lastEPMillis;
unsigned long lastTransitionMillis = 0;
unsigned long transitionFrameMs = TRANSITION_FRAME_MS_DEFAULT;

// settings
char lightName[LIGHT_NAME_MAX_LENGTH] = LIGHT_NAME_WS2811;
uint8_t effect, scene, startup;
uint8_t rgb_multiplier[] = {100, 100, 100}; // light multiplier in percentage /R, G, B/

uint8_t lightsCount = LIGHT_COUNT_WS;
uint16_t pixelCount = PIXEL_COUNT_WS;
uint8_t transitionLeds = TRANSITION_LEDS_WS; // pixelCount must be divisible by this value

WebServer server_ws(LIGHT_PORT_WS);
WiFiUDP Udp;

NeoPixelBus<NeoRgbFeature, NeoEsp32Rmt1Ws2812xMethod> *strip = NULL;

void handleNotFound_ws()
{ // default webserver response for unknow requests
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
  for (uint8_t i = 0; i < server_ws.args(); i++)
  {
    message += " " + server_ws.argName(i) + ": " + server_ws.arg(i) + "\n";
  }
  REMOTE_LOG_DEBUG("from:", server_ws.client().remoteIP().toString(), "not found:", server_ws.uri(), "args:", server_ws.args());
  server_ws.send(404, "text/plain", message);
}

void apply_scene_ws(uint8_t new_scene)
{ // these are internal scenes store in light firmware that can be applied on boot and manually from light web interface
  for (uint8_t light = 0; light < lightsCount; light++)
  {
    if (new_scene == 1)
    {
      lights[light].bri = 254;
      lights[light].ct = 346;
      lights[light].colorMode = 2;
      convertCt(lights[light], rgb_multiplier);
    }
    else if (new_scene == 2)
    {
      lights[light].bri = 254;
      lights[light].ct = 233;
      lights[light].colorMode = 2;
      convertCt(lights[light], rgb_multiplier);
    }
    else if (new_scene == 3)
    {
      lights[light].bri = 254;
      lights[light].ct = 156;
      lights[light].colorMode = 2;
      convertCt(lights[light], rgb_multiplier);
    }
    else if (new_scene == 4)
    {
      lights[light].bri = 77;
      lights[light].ct = 367;
      lights[light].colorMode = 2;
      convertCt(lights[light], rgb_multiplier);
    }
    else if (new_scene == 5)
    {
      lights[light].bri = 254;
      lights[light].ct = 447;
      lights[light].colorMode = 2;
      convertCt(lights[light], rgb_multiplier);
    }
    else if (new_scene == 6)
    {
      lights[light].bri = 1;
      lights[light].x = 0.561;
      lights[light].y = 0.4042;
      lights[light].colorMode = 1;
      convertXy(lights[light], rgb_multiplier);
    }
    else if (new_scene == 7)
    {
      lights[light].bri = 203;
      lights[light].x = 0.380328;
      lights[light].y = 0.39986;
      lights[light].colorMode = 1;
      convertXy(lights[light], rgb_multiplier);
    }
    else if (new_scene == 8)
    {
      lights[light].bri = 112;
      lights[light].x = 0.359168;
      lights[light].y = 0.28807;
      lights[light].colorMode = 1;
      convertXy(lights[light], rgb_multiplier);
    }
    else if (new_scene == 9)
    {
      lights[light].bri = 142;
      lights[light].x = 0.267102;
      lights[light].y = 0.23755;
      lights[light].colorMode = 1;
      convertXy(lights[light], rgb_multiplier);
    }
    else if (new_scene == 10)
    {
      lights[light].bri = 216;
      lights[light].x = 0.393209;
      lights[light].y = 0.29961;
      lights[light].colorMode = 1;
      convertXy(lights[light], rgb_multiplier);
    }
    else
    {
      lights[light].bri = 144;
      lights[light].ct = 447;
      lights[light].colorMode = 2;
      convertCt(lights[light], rgb_multiplier);
    }
  }
}

void processLightdata(uint8_t light, float transitiontime)
{                                           // calculate the step level of every RGB channel for a smooth transition in requested transition time
  transitiontime *= 14 - (pixelCount / 70); // every extra led add a small delay that need to be counted for transition time match

  REMOTE_LOG_DEBUG("Light:", light);
	REMOTE_LOG_DEBUG("bri:", lights[light].bri);
	REMOTE_LOG_DEBUG("state:", lights[light].lightState);
  REMOTE_LOG_DEBUG("colorMode:", lights[light].colorMode);

  if (lights[light].colorMode == 1 && lights[light].lightState == true)
  {
    REMOTE_LOG_DEBUG("x:", lights[light].x);
    REMOTE_LOG_DEBUG("y:", lights[light].y);
    convertXy(lights[light], rgb_multiplier);
  }
  else if (lights[light].colorMode == 2 && lights[light].lightState == true)
  {
    REMOTE_LOG_DEBUG("ct:", lights[light].ct);
    convertCt(lights[light], rgb_multiplier);
  }
  else if (lights[light].colorMode == 3 && lights[light].lightState == true)
  {
    REMOTE_LOG_DEBUG("hue:", lights[light].hue);
    REMOTE_LOG_DEBUG("sat:", lights[light].sat);
    convertHue(lights[light]);
  }
  for (uint8_t i = 0; i < 3; i++)
  {
    if (lights[light].lightState)
    {
      lights[light].stepLevel[i] = ((float)lights[light].colors[i] - lights[light].currentColors[i]) / transitiontime;
    }
    else
    {
      lights[light].stepLevel[i] = lights[light].currentColors[i] / transitiontime;
    }
  }
  REMOTE_LOG_DEBUG("colors R:", lights[light].colors[0], "G:", lights[light].colors[1], "B:", lights[light].colors[2]);
	REMOTE_LOG_DEBUG("transitiontime:", transitiontime);
}

RgbColor blending(const float left[3], const float right[3], uint8_t pixel)
{ // return RgbColor based on neighbour leds
  uint8_t result[3];
  for (uint8_t i = 0; i < 3; i++)
  {
    float percent = (float)pixel / (float)(transitionLeds + 1);
    result[i] = (left[i] * (1.0f - percent) + right[i] * percent);
  }
  return RgbColor((uint8_t)result[0], (uint8_t)result[1], (uint8_t)result[2]);
}

void candleEffect()
{
  for (uint8_t light = 0; light < lightsCount; light++)
  {
    lights[light].colors[0] = random(170, 254);
    lights[light].colors[1] = random(37, 62);
    lights[light].colors[2] = 0;
    for (uint8_t i = 0; i < 3; i++)
    {
      lights[light].stepLevel[i] = ((float)lights[light].colors[i] - lights[light].currentColors[i]) / random(5, 15);
    }
  }
}

void firePlaceEffect()
{
  for (uint8_t light = 0; light < lightsCount; light++)
  {
    lights[light].colors[0] = random(100, 254);
    lights[light].colors[1] = random(10, 35);
    lights[light].colors[2] = 0;
    for (uint8_t i = 0; i < 3; i++)
    {
      lights[light].stepLevel[i] = ((float)lights[light].colors[i] - lights[light].currentColors[i]) / random(5, 15);
    }
  }
}

RgbColor convFloat(float color[3])
{ // return RgbColor from float
  return RgbColor((uint8_t)color[0], (uint8_t)color[1], (uint8_t)color[2]);
}

void lightEngine()
{ // core function executed in loop()
  unsigned long now = millis();
  // If a transition is active, only process stepping once per frame interval
  if (inTransition && (now - lastTransitionMillis) < transitionFrameMs)
  {
    return;
  }
  for (int light = 0; light < lightsCount; light++)
  { // loop with every virtual light
    if (lights[light].lightState)
    { // if light in on
      if (lights[light].colors[0] != lights[light].currentColors[0] || lights[light].colors[1] != lights[light].currentColors[1] || lights[light].colors[2] != lights[light].currentColors[2])
      { // if not all RGB channels of the light are at desired level
        if (!inTransition)
          lastTransitionMillis = millis();
        inTransition = true;
        for (uint8_t k = 0; k < 3; k++)
        { // loop with every RGB channel of the light
          if (lights[light].colors[k] != lights[light].currentColors[k])
            lights[light].currentColors[k] += lights[light].stepLevel[k]; // move RGB channel on step closer to desired level
          if ((lights[light].stepLevel[k] > 0.0 && lights[light].currentColors[k] > lights[light].colors[k]) || (lights[light].stepLevel[k] < 0.0 && lights[light].currentColors[k] < lights[light].colors[k]))
            lights[light].currentColors[k] = lights[light].colors[k]; // if the current level go below desired level apply directly the desired level.
        }
        if (lightsCount > 1)
        { // if are more then 1 virtual light we need to apply transition leds (set in the web interface)
          if (light == 0)
          {                                                               // if is the first light we must not have transition leds at the beginning
            for (int pixel = 0; pixel < lights[0].dividedLights; pixel++) // loop with all leds of the light (declared in web interface)
            {
              if (pixel < lights[0].dividedLights - transitionLeds / 2)
              { // apply raw color if we are outside transition leds
                strip->SetPixelColor(pixel, convFloat(lights[light].currentColors));
              }
              else
              {
                strip->SetPixelColor(pixel, blending(lights[0].currentColors, lights[1].currentColors, pixel + 1 - (lights[0].dividedLights - transitionLeds / 2))); // calculate the transition led color
              }
            }
          }
          else
          {                                                                   // is not the first virtual light
            for (int pixel = 0; pixel < lights[light].dividedLights; pixel++) // loop with all leds of the light
            {
              long pixelSum = 0;
              for (int value = 0; value < light; value++)
              {
                if (value + 1 == light)
                {
                  pixelSum += lights[value].dividedLights - transitionLeds;
                }
                else
                {
                  pixelSum += lights[value].dividedLights;
                }
              }

              if (pixel < transitionLeds / 2)
              { // beginning transition leds
                strip->SetPixelColor(pixel + pixelSum + transitionLeds, blending(lights[light - 1].currentColors, lights[light].currentColors, pixel + 1));
              }
              else if (pixel > lights[light].dividedLights - transitionLeds / 2 - 1)
              { // end of transition leds
                // Serial.println(String(pixel));
                strip->SetPixelColor(pixel + pixelSum + transitionLeds, blending(lights[light].currentColors, lights[light + 1].currentColors, pixel + transitionLeds / 2 - lights[light].dividedLights));
              }
              else
              { // outside transition leds (apply raw color)
                strip->SetPixelColor(pixel + pixelSum + transitionLeds, convFloat(lights[light].currentColors));
              }
            }
          }
        }
        else
        { // strip has only one virtual light so apply raw color to entire strip
          strip->ClearTo(convFloat(lights[light].currentColors), 0, pixelCount - 1);
        }
        strip->Show(); // show what was calculated previously
        lastTransitionMillis = millis();
      }
    }
    else
    { // if light in off, calculate the dimming effect only
      if (lights[light].currentColors[0] != 0 || lights[light].currentColors[1] != 0 || lights[light].currentColors[2] != 0)
      { // proceed forward only in case not all RGB channels are zero
        if (!inTransition)
          lastTransitionMillis = millis();
        inTransition = true;
        for (uint8_t k = 0; k < 3; k++)
        { // loop with every RGB channel
          if (lights[light].currentColors[k] != 0)
            lights[light].currentColors[k] -= lights[light].stepLevel[k]; // remove one step level
          if (lights[light].currentColors[k] < 0)
            lights[light].currentColors[k] = 0; // save condition, if level go below zero set it to zero
        }
        if (lightsCount > 1)
        { // if the strip has more than one light
          if (light == 0)
          {                                                               // if is the first light of the strip
            for (int pixel = 0; pixel < lights[0].dividedLights; pixel++) // loop with every led of the virtual light
            {
              if (pixel < lights[0].dividedLights - transitionLeds / 2)
              { // leds until transition zone apply raw color
                strip->SetPixelColor(pixel, convFloat(lights[light].currentColors));
              }
              else
              { // leds in transition zone apply the transition color
                strip->SetPixelColor(pixel, blending(lights[0].currentColors, lights[1].currentColors, pixel + 1 - (lights[0].dividedLights - transitionLeds / 2)));
              }
            }
          }
          else
          {                                                                   // is not the first light
            for (int pixel = 0; pixel < lights[light].dividedLights; pixel++) // loop with every led
            {
              long pixelSum = 0;
              for (int value = 0; value < light; value++)
              {
                if (value + 1 == light)
                {
                  pixelSum += lights[value].dividedLights - transitionLeds;
                }
                else
                {
                  pixelSum += lights[value].dividedLights;
                }
              }

              if (pixel < transitionLeds / 2)
              { // leds in beginning of transition zone must apply blending
                strip->SetPixelColor(pixel + pixelSum + transitionLeds, blending(lights[light - 1].currentColors, lights[light].currentColors, pixel + 1));
              }
              else if (pixel > lights[light].dividedLights - transitionLeds / 2 - 1)
              { // leds in the end of transition zone must apply blending
                // Serial.println(String(pixel));
                strip->SetPixelColor(pixel + pixelSum + transitionLeds, blending(lights[light].currentColors, lights[light + 1].currentColors, pixel + transitionLeds / 2 - lights[light].dividedLights));
              }
              else
              { // leds outside transition zone apply raw color
                strip->SetPixelColor(pixel + pixelSum + transitionLeds, convFloat(lights[light].currentColors));
              }
            }
          }
        }
        else
        { // is just one virtual light declared, apply raw color to all leds
          strip->ClearTo(convFloat(lights[light].currentColors), 0, pixelCount - 1);
        }
        strip->Show();
        lastTransitionMillis = millis();
      }
    }
  }
  if (inTransition)
  { // non-blocking wait for a nice transition effect
    if (millis() - lastTransitionMillis >= transitionFrameMs)
    {
      inTransition = false; // clear transition flag after minimal pause
    }
  }
  else
  {
    if (effect == 1)
    { // candle effect
      candleEffect();
    }
    else if (effect == 2)
    { // fireplace effect
      firePlaceEffect();
    }
  }
}

void saveState_ws()
{ // save the lights state using generic helper
  REMOTE_LOG_DEBUG("save ws state");
  JsonDocument json;
  for (uint8_t i = 0; i < lightsCount; i++)
  {
    JsonObject light = json[String(i)].to<JsonObject>();
    light["on"] = lights[i].lightState;
    light["bri"] = lights[i].bri;
    if (lights[i].colorMode == 1)
    {
      light["x"] = lights[i].x;
      light["y"] = lights[i].y;
    }
    else if (lights[i].colorMode == 2)
    {
      light["ct"] = lights[i].ct;
    }
    else if (lights[i].colorMode == 3)
    {
      light["hue"] = lights[i].hue;
      light["sat"] = lights[i].sat;
    }
  }
  writeJsonFile(WS_STATE_PATH, json);
}

void restoreState_ws()
{ // restore the lights state using generic helper
  REMOTE_LOG_DEBUG("restore ws state");
  JsonDocument json;
  if (!readJsonFile(WS_STATE_PATH, json))
  {
    saveState_ws();
    return;
  }

  for (JsonPair state : json.as<JsonObject>())
  {
    const char *key = state.key().c_str();
    int lightId = atoi(key);
    JsonObject values = state.value();
    lights[lightId].lightState = values["on"];
    lights[lightId].bri = (uint8_t)values["bri"];
    if (values["x"].is<float>())
    {
      lights[lightId].x = values["x"];
      lights[lightId].y = values["y"];
      lights[lightId].colorMode = 1;
    }
    else if (values["ct"].is<int>())
    {
      lights[lightId].ct = values["ct"];
      lights[lightId].colorMode = 2;
    }
    else
    {
      if (values["hue"].is<int>())
      {
        lights[lightId].hue = values["hue"];
        lights[lightId].colorMode = 3;
      }
      if (values["sat"].is<int>())
      {
        lights[lightId].sat = (uint8_t)values["sat"];
        lights[lightId].colorMode = 3;
      }
    }
  }
}

bool saveConfig_ws()
{ // save config using generic helper
  REMOTE_LOG_DEBUG("save ws config");
  JsonDocument json;
  json["name"] = lightName;
  json["startup"] = startup;
  json["scene"] = scene;
  json["lightsCount"] = lightsCount;
  for (uint16_t i = 0; i < lightsCount; i++)
  {
    int divided = 0;
    if (lights != nullptr)
    {
      divided = (int)lights[i].dividedLights;
    }
    else if (lightsCount > 0)
    {
      divided = pixelCount / lightsCount; // reasonable default when lights not yet allocated
    }
    json["dividedLight_" + String(i)] = divided;
  }
  json["pixelCount"] = pixelCount;
  json["transLeds"] = transitionLeds;
  json["transMs"] = (int)transitionFrameMs;
  json["rpct"] = rgb_multiplier[0];
  json["gpct"] = rgb_multiplier[1];
  json["bpct"] = rgb_multiplier[2];
  return writeJsonFile(WS_CONFIG_PATH, json);
}

bool loadConfig_ws()
{ // load the configuration using generic helper
  REMOTE_LOG_DEBUG("load ws config");
  JsonDocument json;
  if (!readJsonFile(WS_CONFIG_PATH, json))
  {
    REMOTE_LOG_DEBUG("Create new file with default values");
    return saveConfig_ws();
  }

  strcpy(lightName, json["name"]);
  startup = (uint8_t)json["startup"];
  scene = (uint8_t)json["scene"];
  lightsCount = (uint16_t)json["lightsCount"];
  for (uint16_t i = 0; i < lightsCount; i++)
  {
    lights[i].dividedLights = (uint16_t)json["dividedLight_" + String(i)];
  }
  pixelCount = (uint16_t)json["pixelCount"];
  transitionLeds = (uint8_t)json["transLeds"];
  if (json["transMs"].is<int>())
  {
    int t = (int)json["transMs"];
    if (t < 1)
      t = 1;
    else if (t > 1000)
      t = 1000;
    transitionFrameMs = (unsigned long)t;
  }
  if (json["rpct"].is<int>())
    rgb_multiplier[0] = (uint8_t)json["rpct"];
  if (json["gpct"].is<int>())
    rgb_multiplier[1] = (uint8_t)json["gpct"];
  if (json["bpct"].is<int>())
    rgb_multiplier[2] = (uint8_t)json["bpct"];
  return true;
}

void ChangeNeoPixels(uint16_t newCount) // this set the number of leds of the strip based on web configuration
{
  if (strip != NULL)
  {
    delete strip; // delete the previous dynamically created strip
  }
  // Sanity check DATA_PIN before initializing NeoPixelBus / RMT
  int dataPin = DATA_PIN;
  REMOTE_LOG_DEBUG("DATA_PIN=", dataPin);
  if (dataPin < 0 || dataPin > 47)
  {
    REMOTE_LOG_ERROR("ChangeNeoPixels: invalid DATA_PIN, aborting strip init");
    strip = NULL;
    return;
  }
  strip = new NeoPixelBus<NeoRgbFeature, NeoEsp32Rmt1Ws2812xMethod>(newCount, DATA_PIN); // and recreate with new count
  strip->Begin();
}

void ws_setup()
{
  REMOTE_LOG_DEBUG("Setup WS2811");
  infoLight(yellow);

  if (!loadConfig_ws())
  {
    REMOTE_LOG_DEBUG("Failed to load config");
  }
  else
  {
    REMOTE_LOG_DEBUG("Config loaded");
  }

  if (lights[0].dividedLights == 0)
  {
    for (uint8_t light = 0; light < lightsCount; light++)
    {
      lights[light].dividedLights = pixelCount / lightsCount;
    }
    saveConfig_ws();
  }

  ChangeNeoPixels(pixelCount);

  switch (startup)
  {
  case 0:
    REMOTE_LOG_DEBUG("Startup: Restore previous state");
    for (uint8_t i = 0; i < lightsCount; i++)
    {
      lights[i].lightState = true;
    }
    break;
  case 1:
    REMOTE_LOG_DEBUG("Startup: All lights ON");
    restoreState_ws();
    break;
  default:
    REMOTE_LOG_DEBUG("Startup: Apply scene", String(scene));
    apply_scene_ws(scene);
    break;
  }

  for (uint8_t i = 0; i < lightsCount; i++)
  {
    processLightdata(i);
  }

  if (lights[0].lightState)
  {
    for (uint8_t i = 0; i < 200; i++)
    {
      lightEngine();
    }
  }

  Udp.begin(2100); // start entertainment UDP server

  server_ws.on("/state", HTTP_PUT, []() { // HTTP PUT request used to set a new light state
    infoLight(cyan);     // Cyan for WS2811 requests
    JsonDocument root;
    DeserializationError error = deserializeJson(root, server_ws.arg("plain"));

    if (error)
    {
      server_ws.send(404, "text/plain", "FAIL. " + server_ws.arg("plain"));
    }
    else
    {
      for (JsonPair state : root.as<JsonObject>())
      {
        const char *key = state.key().c_str();
        int light = atoi(key) - 1;
        JsonObject values = state.value();
        int transitiontime = 4;

        if (values["effect"].is<const char *>())
        {
          if (values["effect"] == "no_effect")
          {
            effect = 0;
          }
          else if (values["effect"] == "candle")
          {
            effect = 1;
          }
          else if (values["effect"] == "fire")
          {
            effect = 2;
          }
        }

        if (values["xy"].is<JsonArray>())
        {
          lights[light].x = values["xy"][0];
          lights[light].y = values["xy"][1];
          lights[light].colorMode = 1;
        }
        else if (values["ct"].is<int>())
        {
          lights[light].ct = values["ct"];
          lights[light].colorMode = 2;
        }
        else
        {
          if (values["hue"].is<int>())
          {
            lights[light].hue = values["hue"];
            lights[light].colorMode = 3;
          }
          if (values["sat"].is<int>())
          {
            lights[light].sat = values["sat"];
            lights[light].colorMode = 3;
          }
        }

        if (values["on"].is<int>() || values["on"].is<bool>())
        {
          if (values["on"])
          {
            lights[light].lightState = true;
          }
          else
          {
            lights[light].lightState = false;
          }
        }

        if (values["bri"].is<float>() || values["bri"].is<int>())
        {
          lights[light].bri = (int)values["bri"];
        }

        if (values["bri_inc"].is<int>())
        {
          if (values["bri_inc"] > 0)
          {
            if (lights[light].bri + (int)values["bri_inc"] > 254)
            {
              lights[light].bri = 254;
            }
            else
            {
              lights[light].bri += (int)values["bri_inc"];
            }
          }
          else
          {
            if (lights[light].bri - (int)values["bri_inc"] < 1)
            {
              lights[light].bri = 1;
            }
            else
            {
              lights[light].bri += (int)values["bri_inc"];
            }
          }
        }

        if (values["transitiontime"].is<int>())
        {
          transitiontime = values["transitiontime"];
        }

        if (values["alert"].is<const char *>() && values["alert"] == "select")
        {
          if (lights[light].lightState)
          {
            lights[light].currentColors[0] = 0;
            lights[light].currentColors[1] = 0;
            lights[light].currentColors[2] = 0;
          }
          else
          {
            lights[light].currentColors[1] = 126;
            lights[light].currentColors[2] = 126;
          }
        }
        processLightdata(light, transitiontime);
      }
      String output;
      serializeJson(root, output);
      REMOTE_LOG_DEBUG("from:", server_ws.client().remoteIP().toString(), "/state put", output);
      server_ws.send(200, "text/plain", output);
      saveState_ws();
    }
  });

  server_ws.on("/state", HTTP_GET, []() { // HTTP GET request used to fetch current light state
    uint8_t light = server_ws.arg("light").toInt() - 1;
    JsonDocument root;
    root["on"] = lights[light].lightState;
    root["bri"] = lights[light].bri;
    JsonArray xy = root["xy"].to<JsonArray>();
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
    REMOTE_LOG_DEBUG("from:", server_ws.client().remoteIP().toString(), "/state get light:", light, output);
    server_ws.send(200, "text/plain", output);
  });

  server_ws.on("/detect", []() { // HTTP GET request used to discover the light type
    JsonDocument root;
    root["name"] = lightName;
    root["lights"] = lightsCount;
    root["protocol"] = LIGHT_PROTOCOL_WS;
    root["modelid"] = LIGHT_MODEL_WS;
    root["type"] = LIGHT_TYPE_WS;
    root["mac"] = get_mac_address();
    root["version"] = LIGHT_VERSION;
    String output;
    serializeJson(root, output);
		REMOTE_LOG_DEBUG("from:", server_ws.client().remoteIP().toString(), "/detect", output);
    server_ws.send(200, "text/plain", output);
  });

  server_ws.on("/config", []() { // used by light web interface to get current configuration
    JsonDocument root;
    root["name"] = lightName;
    root["scene"] = scene;
    root["startup"] = startup;
    root["lightscount"] = lightsCount;
    for (uint8_t i = 0; i < lightsCount; i++)
    {
      root["dividedLight_" + String(i)] = (int)lights[i].dividedLights;
    }
    root["pixelcount"] = pixelCount;
    root["transitionleds"] = transitionLeds;
    root["transms"] = transitionFrameMs;
    root["rpct"] = rgb_multiplier[0];
    root["gpct"] = rgb_multiplier[1];
    root["bpct"] = rgb_multiplier[2];
    String output;
    serializeJson(root, output);
    REMOTE_LOG_DEBUG("from:", server_ws.client().remoteIP().toString(), "/config", output);
    server_ws.send(200, "text/plain", output);
  });

  server_ws.on("/", []() { // light http web interface
    if (server_ws.arg("section").toInt() == 1)
    {
      server_ws.arg("name").toCharArray(lightName, LIGHT_NAME_MAX_LENGTH);
      startup = server_ws.arg("startup").toInt();
      scene = server_ws.arg("scene").toInt();
      {
        uint16_t requested = server_ws.arg("lightscount").toInt();
        if (requested == 0)
          requested = 1;
        if (requested != lightsCount)
        {
          if (requested > MAX_RUNTIME_LIGHTS)
          {
            server_ws.send(400, "text/plain", "Requested lights count exceeds maximum of " + String(MAX_RUNTIME_LIGHTS));
            return;
          }
          lightsCount = requested;
        }
      }
      pixelCount = server_ws.arg("pixelcount").toInt();
      transitionLeds = server_ws.arg("transitionleds").toInt();
      if (server_ws.arg("transms").length())
      {
        int t = server_ws.arg("transms").toInt();
        if (t < 1)
          t = 1;
        else if (t > 1000)
          t = 1000;
        transitionFrameMs = (unsigned long)t;
      }
      rgb_multiplier[0] = server_ws.arg("rpct").toInt();
      rgb_multiplier[1] = server_ws.arg("gpct").toInt();
      rgb_multiplier[2] = server_ws.arg("bpct").toInt();
      for (uint16_t i = 0; i < lightsCount; i++)
      {
        lights[i].dividedLights = server_ws.arg("dividedLight_" + String(i)).toInt();
      }
      saveConfig_ws();
    }
    if (server_ws.args())
    {
      resetESP();
    }
    REMOTE_LOG_DEBUG("from:", server_ws.client().remoteIP().toString(), "/", "args:", server_ws.args());
    server_ws.send_P(200, "text/html", htmlContent_ws);

  });

  server_ws.on("/reset", []() { // trigger manual reset
    server_ws.send(200, "text/html", "reset");
		REMOTE_LOG_DEBUG("from:", server_ws.client().remoteIP().toString(), "/reset");
    resetESP();
  });

  server_ws.onNotFound(handleNotFound_ws);

  server_ws.begin();

  httpUpdateServer.setup(&server_ws); // start http server
}

RgbColor blendingEntert(const float left[3], const float right[3], float pixel)
{
  uint8_t result[3];
  for (uint8_t i = 0; i < 3; i++)
  {
    float percent = (float)pixel / (float)(transitionLeds + 1);
    result[i] = (left[i] * (1.0f - percent) + right[i] * percent) / 2;
  }
  return RgbColor((uint8_t)result[0], (uint8_t)result[1], (uint8_t)result[2]);
}

void entertainment()
{                                         // entertainment function
  uint8_t packetSize = Udp.parsePacket(); // check if UDP received some bytes
  if (packetSize)
  {                          // if nr of bytes is more than zero
    entertainmentRun = true; // announce entertainment is running
    lastEPMillis = millis(); // update variable with last received package timestamp
    // Visual indicator for entertainment mode - brief pulse every few packets
    static unsigned long lastEntertainmentBlink = 0;
    if (millis() - lastEntertainmentBlink > 500)
    {                                   // Pulse every 500ms during entertainment
      infoLight(purple); // Purple for entertainment mode
      lastEntertainmentBlink = millis();
    }
    Udp.read(packetBuffer, packetSize);
    for (uint8_t i = 0; i < packetSize / 4; i++)
    { // loop with every light. There are 4 bytes for every light (light number, red, green, blue)
      lights[packetBuffer[i * 4]].currentColors[0] = packetBuffer[i * 4 + 1] * rgb_multiplier[0] / 100;
      lights[packetBuffer[i * 4]].currentColors[1] = packetBuffer[i * 4 + 2] * rgb_multiplier[1] / 100;
      lights[packetBuffer[i * 4]].currentColors[2] = packetBuffer[i * 4 + 3] * rgb_multiplier[2] / 100;
    }
    for (uint8_t light = 0; light < lightsCount; light++)
    {
      if (lightsCount > 1)
      {
        if (light == 0)
        {
          for (int pixel = 0; pixel < lights[0].dividedLights; pixel++)
          {
            if (pixel < lights[0].dividedLights - transitionLeds / 2)
            {
              strip->SetPixelColor(pixel, convFloat(lights[light].currentColors));
            }
            else
            {
              strip->SetPixelColor(pixel, blendingEntert(lights[0].currentColors, lights[1].currentColors, pixel + 1 - (lights[0].dividedLights - transitionLeds / 2)));
            }
          }
        }
        else
        {
          for (int pixel = 0; pixel < lights[light].dividedLights; pixel++)
          {
            long pixelSum = 0;
            for (int value = 0; value < light; value++)
            {
              if (value + 1 == light)
              {
                pixelSum += lights[value].dividedLights - transitionLeds;
              }
              else
              {
                pixelSum += lights[value].dividedLights;
              }
            }
            if (pixel < transitionLeds / 2)
            {
              strip->SetPixelColor(pixel + pixelSum + transitionLeds, blendingEntert(lights[light - 1].currentColors, lights[light].currentColors, pixel + 1));
            }
            else if (pixel > lights[light].dividedLights - transitionLeds / 2 - 1)
            {
              // Serial.println(String(pixel));
              strip->SetPixelColor(pixel + pixelSum + transitionLeds, blendingEntert(lights[light].currentColors, lights[light + 1].currentColors, pixel + transitionLeds / 2 - lights[light].dividedLights));
            }
            else
            {
              strip->SetPixelColor(pixel + pixelSum + transitionLeds, convFloat(lights[light].currentColors));
            }
          }
        }
      }
      else
      {
        strip->ClearTo(convFloat(lights[light].currentColors), 0, pixelCount - 1);
      }
    }
    strip->Show();
    lastTransitionMillis = millis();
  }
}

void ws_loop()
{
  server_ws.handleClient();
  if (!entertainmentRun)
  {
    lightEngine(); // process lights data set on http server
  }
  else
  {
    if ((millis() - lastEPMillis) >= ENTERTAINMENT_TIMEOUT)
    { // entertainment stream stop (timeout)
      entertainmentRun = false;
      infoLedIdle(); // Return to idle when entertainment stops
      for (uint8_t i = 0; i < lightsCount; i++)
      {
        processLightdata(i); // return to original colors with 0.4 sec transition
      }
    }
  }
  entertainment(); // process entertainment data on UDP server
}
