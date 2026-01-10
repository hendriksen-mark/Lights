#include <Wire.h>
#include <Arduino.h>

#include <config.h>

struct LightState
{
    bool on = false;
    uint8_t bri = 0;
    uint8_t pin = PIN;
    uint8_t adress = ADRESS;
    int16_t stepLevel = 0;  // amount to change per step (scaled by 16)
    int16_t currentBri = 0; // current brightness (scaled by 16)
};

static LightState light;

static bool dataAvailable = false;
static uint16_t transitionTime = 0;
static uint8_t recdata[4];

// Non-blocking step timing
static unsigned long lastStepMillis = 0;

static void applyAnalogWrite()
{
    int out = (light.currentBri + 8) >> 4; // divide by 16 with rounding
    out = constrain(out, 0, 255);
    analogWrite(light.pin, out);
}

static void computeStepLevel()
{
    uint16_t steps = transitionTime << 4; // multiply by 16
    if (steps == 0)
    {
        if (light.on)
        {
            light.currentBri = (int16_t)light.bri << 4;
        }
        else
        {
            light.currentBri = 0;
        }
        light.stepLevel = 0;
        applyAnalogWrite();
        return;
    }
    int16_t target = light.on ? ((int16_t)light.bri << 4) : 0;
    int16_t delta = target - light.currentBri;
    int16_t s = (int16_t)steps;
    int16_t step = delta / s; // may be zero if delta < s
    if (step == 0 && delta != 0)
    {
        // ensure we make progress by at least one LSB in fixed point
        step = (delta > 0) ? 1 : -1;
    }
    light.stepLevel = step;
}

static void lightEngine()
{
    unsigned long now = millis();
    if (now - lastStepMillis < STEP_MS)
        return;
    lastStepMillis = now;

    int16_t target = light.on ? ((int16_t)light.bri << 4) : 0;
    
    if (light.currentBri != target)
    {
        light.currentBri += light.stepLevel;
        
        if ((light.stepLevel > 0 && light.currentBri > target) ||
            (light.stepLevel < 0 && light.currentBri < target))
        {
            light.currentBri = target;
        }
        
        if (light.currentBri < 0)
            light.currentBri = 0;
            
        applyAnalogWrite();
    }
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
    Wire.write(light.bri);
    Wire.write(light.on ? 1 : 0);
}

static void processData()
{
    light.bri = recdata[0];
    light.on = (recdata[1] != 0);
    transitionTime = ((uint16_t)recdata[2] << 8) | recdata[3];
    computeStepLevel();
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
