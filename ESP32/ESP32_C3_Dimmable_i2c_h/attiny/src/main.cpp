#include <Wire.h>
#include <Arduino.h>

#include <config.h>

// Small, focused refactor: safer types, bounds checks, and non-blocking transition.

struct LightState
{
    bool on = false; // target on/off state
    uint8_t bri = 0; // target brightness (0-255)
    uint8_t pin = PIN;
    uint8_t adress = ADRESS;
    float stepLevel = 0.0f;  // amount to change per step
    float currentBri = 0.0f; // current brightness
};

static LightState light;

static bool dataAvailable = false;
static float transitionTime = 0.0; // in steps (will be scaled)
static uint8_t recdata[4];

// Non-blocking step timing
static const unsigned long STEP_MS = 6;
static unsigned long lastStepMillis = 0;

static void applyAnalogWrite()
{
    int out = (int)round(light.currentBri);
    out = constrain(out, 0, 255);
    analogWrite(light.pin, out);
}

static void computeStepLevel()
{
    // original code scaled transition time by 16
    float steps = transitionTime * 16.0;
    if (steps <= 0.0)
    {
        // No stepping — apply target immediately to avoid stuck transitions
        if (light.on)
        {
            light.currentBri = light.bri;
        }
        else
        {
            light.currentBri = 0.0f;
        }
        light.stepLevel = 0.0f;
        applyAnalogWrite();
        return;
    }
    if (light.on)
    {
        light.stepLevel = ((float)light.bri - light.currentBri) / steps;
    }
    else
    {
        light.stepLevel = light.currentBri / steps;
    }
}

static void lightEngine()
{
    unsigned long now = millis();
    if (now - lastStepMillis < STEP_MS)
        return; // not time for next step
    lastStepMillis = now;

    bool transitioned = false;

    if (light.on)
    {
        if ((int)light.bri != (int)round(light.currentBri))
        {
            light.currentBri += light.stepLevel;
            if ((light.stepLevel > 0.0f && light.currentBri > light.bri) ||
                (light.stepLevel < 0.0f && light.currentBri < light.bri))
            {
                light.currentBri = light.bri;
            }
            applyAnalogWrite();
            transitioned = true;
        }
    }
    else
    {
        if ((int)round(light.currentBri) != 0)
        {
            light.currentBri -= light.stepLevel;
            if (light.currentBri < 0.0f)
                light.currentBri = 0.0f;
            applyAnalogWrite();
            transitioned = true;
        }
    }

    (void)transitioned; // placeholder if future logic needs it
}

// I2C receive: guard against malformed messages
void receiveEvent(int howMany)
{
    if (howMany <= 0)
        return;
    int count = min(howMany, (int)sizeof(recdata));
    for (int i = 0; i < count; ++i)
    {
        recdata[i] = Wire.read();
    }
    // if extra bytes arrive, discard them
    for (int i = count; i < howMany; ++i)
        Wire.read();
    // clear any remaining bytes in recdata if message was shorter than 4
    for (int i = count; i < (int)sizeof(recdata); ++i)
        recdata[i] = 0;
    dataAvailable = true;
}

void requestEvent()
{
    uint8_t cur = (uint8_t)constrain((int)round(light.currentBri), 0, 255);
    Wire.write(cur);
    Wire.write(light.on ? 1 : 0);
}

static void processData()
{
    light.bri = recdata[0];
    light.on = (recdata[1] != 0);
    transitionTime = ((uint16_t)recdata[2] << 8) | recdata[3];
    // If transitionTime is zero, apply the new state immediately
    if (transitionTime == 0.0f)
    {
        if (light.on)
        {
            light.currentBri = light.bri;
        }
        else
        {
            light.currentBri = 0.0f;
        }
        light.stepLevel = 0.0f;
        applyAnalogWrite();
    }
    else
    {
        computeStepLevel();
    }
    dataAvailable = false;
}

void setup()
{
    Wire.begin(light.adress);
    Wire.onReceive(receiveEvent);
    Wire.onRequest(requestEvent);

    pinMode(light.pin, OUTPUT);
    applyAnalogWrite();
    lastStepMillis = millis();
}

void loop()
{
    if (dataAvailable)
    {
        processData();
    }
    lightEngine();
}
