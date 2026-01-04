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

static LightState lights[1];

static bool dataAvailable = false;
static float transitionTime = 0.0; // in steps (will be scaled)
static uint8_t recdata[4];

// Non-blocking step timing
static const unsigned long STEP_MS = 6;
static unsigned long lastStepMillis = 0;

static void applyAnalogWrite()
{
    int out = (int)round(lights[0].currentBri);
    out = constrain(out, 0, 255);
    analogWrite(lights[0].pin, out);
}

static void computeStepLevel()
{
    // original code scaled transition time by 16
    float steps = transitionTime * 16.0;
    if (steps <= 0.0)
    {
        // No stepping — apply target immediately to avoid stuck transitions
        if (lights[0].on)
        {
            lights[0].currentBri = lights[0].bri;
        }
        else
        {
            lights[0].currentBri = 0.0f;
        }
        lights[0].stepLevel = 0.0f;
        applyAnalogWrite();
        return;
    }
    if (lights[0].on)
    {
        lights[0].stepLevel = ((float)lights[0].bri - lights[0].currentBri) / steps;
    }
    else
    {
        lights[0].stepLevel = lights[0].currentBri / steps;
    }
}

static void lightEngine()
{
    unsigned long now = millis();
    if (now - lastStepMillis < STEP_MS)
        return; // not time for next step
    lastStepMillis = now;

    bool transitioned = false;

    if (lights[0].on)
    {
        if ((int)lights[0].bri != (int)round(lights[0].currentBri))
        {
            lights[0].currentBri += lights[0].stepLevel;
            if ((lights[0].stepLevel > 0.0f && lights[0].currentBri > lights[0].bri) ||
                (lights[0].stepLevel < 0.0f && lights[0].currentBri < lights[0].bri))
            {
                lights[0].currentBri = lights[0].bri;
            }
            applyAnalogWrite();
            transitioned = true;
        }
    }
    else
    {
        if ((int)round(lights[0].currentBri) != 0)
        {
            lights[0].currentBri -= lights[0].stepLevel;
            if (lights[0].currentBri < 0.0f)
                lights[0].currentBri = 0.0f;
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
    uint8_t cur = (uint8_t)constrain((int)round(lights[0].currentBri), 0, 255);
    Wire.write(cur);
    Wire.write(lights[0].on ? 1 : 0);
}

static void processData()
{
    lights[0].bri = recdata[0];
    lights[0].on = (recdata[1] != 0);
    transitionTime = ((uint16_t)recdata[2] << 8) | recdata[3];
    // If transitionTime is zero, apply the new state immediately
    if (transitionTime == 0.0f)
    {
        if (lights[0].on)
        {
            lights[0].currentBri = lights[0].bri;
        }
        else
        {
            lights[0].currentBri = 0.0f;
        }
        lights[0].stepLevel = 0.0f;
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
    Wire.begin(lights[0].adress);
    Wire.onReceive(receiveEvent);
    Wire.onRequest(requestEvent);

    pinMode(lights[0].pin, OUTPUT);
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
