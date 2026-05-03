#if defined(ESP_PLATFORM)
#include "globals.h"
#else
#include "include/globals.h"
#endif

painlessMesh mesh;

// Initialize WS2812B RGB LED using NeoPixelBus
NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt0Ws2812xMethod> statusLED((uint16_t)LED_COUNT, (uint8_t)LED_PIN);

uint32_t master = 0;

byte target;      // 0-100%
byte pref_target; // 0-100%
byte current;     // 0-100%
int state;        // 0 1 2
// int TOTALROND = 43200;//graden
// byte TOTALROND = 120;//rondjes
// byte TOTALROND = 180L;
long uitvoeren;

// Create a new instance of the AccelStepper class:
AccelStepper stepper = AccelStepper(stepper.DRIVER, STEP, DIR);

volatile bool ishome = false;
bool ismoved = false;
volatile bool gohome = false;
unsigned long prev_millis;
volatile bool stopRequested = false;

// Flags for non-blocking mesh response
bool statusRequested = false;
bool newTargetReceived = false;
bool homeRequested = false;
byte pendingTarget = 0;
volatile unsigned long lastHomeEvent = 0;
volatile bool homingActive = false;
volatile uint8_t homingStage = 0;
long homingTarget = 0;
volatile unsigned long homingStartTime = 0;
volatile unsigned long homingTimestamp = 0;