#if defined(ARDUINO_ESP32C3_M1_I_KIT)
#include "functions.h"
#include "driver.h"
#else
#include "include/functions.h"
#include "include/driver.h"
#endif

painlessMesh mesh;

uint32_t master = 0;

byte target;      // 0-100%
byte pref_target; // 0-100%
byte current;     // 0-100%
int state;        // 0 1 2
// int TOTALROND = 43200;//graden
// byte TOTALROND = 120;//rondjes
// byte TOTALROND = 180L;
long uitvoeren;
long totalstep;

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

// ISR: only set a flag to keep ISR quick and safe
void stopMotorISR()
{
  stopRequested = true;
}

void receivedCallback(uint32_t from, String &msg)
{

  JsonDocument root;
  DeserializationError error = deserializeJson(root, msg);
  if (error)
    return;

  if (root["master"].is<unsigned long>())
  {
    master = uint32_t(root["master"]);
  }

  if (root["device"] == DEVICE_NAME)
  {
    if (root["target"].is<int>())
    {
      // Don't call set_Target_Pos() here - just set flag
      pendingTarget = byte(root["target"]);
      newTargetReceived = true;
    }
    if (root["homing"].is<bool>() && root["homing"].as<bool>())
    {
      // Don't call homeing() here - just set flag
      homeRequested = true;
    }
    if (root["request"].is<bool>() && root["request"].as<bool>())
    {
      // Don't call send_change() here - just set flag
      statusRequested = true;
    }
  }
}

void setup()
{
  pinMode(ENABLE, OUTPUT);
  digitalWrite(ENABLE, LOW);

  totalstep = (TOTALROND * MOTOR_STEPS * MICROSTEPS) / 100L; // steps representing 1%

  Serial.begin(115200);
  // mesh.setDebugMsgTypes( ERROR | MESH_STATUS | COMMUNICATION );
  mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.initOTAReceive(DEVICE_NAME);

  configureRAdriver(SW_RX, SW_TX, R_SENSE, DRIVER_ADDRESS, R_CURRENT, 100);
  pinMode(HOME_SWITCH, INPUT);

  stepper.setPinsInverted(RA_INVERT_DIR, false, false);
  stepper.setMaxSpeed(MOTORSPEED * 8);   // 3200
  stepper.setAcceleration(MOTORACC * 8); // 3200
  stepper.setSpeed(MOTORSPEED * 8);

  attachInterrupt(digitalPinToInterrupt(HOME_SWITCH), stopMotorISR, FALLING);

  stepper.setCurrentPosition(0); // 384000);

  LOG_INFO("STARTUP HOMING");
  senddebug();
  gohome = false;
  stepper.runToNewPosition(TOTALROND * MOTOR_STEPS * MICROSTEPS);

  stepper.setMaxSpeed(MOTORSPEED * MICROSTEPS);   // 3200
  stepper.setAcceleration(MOTORACC * MICROSTEPS); // 3200
  stepper.setSpeed(MOTORSPEED * MICROSTEPS);
}

void loop()
{
  mesh.update();

  // Process mesh requests after mesh.update() but before motor control
  // This ensures stepper.run() isn't interrupted by callbacks
  if (newTargetReceived)
  {
    newTargetReceived = false;
    set_Target_Pos(pendingTarget);
  }

  if (homeRequested)
  {
    homeRequested = false;
    ishome = true; // Let existing home logic handle it
  }

  if (statusRequested)
  {
    statusRequested = false;
    send_change();
  }

  if (master == 0)
  {
    ask_master();
  }

  if (stopRequested)
  {
    unsigned long now = millis();
    if (now - lastHomeEvent >= HOMING_DEBOUNCE_MS)
    {
      lastHomeEvent = now;
      stopRequested = false;
      if (homingActive)
      {
        // If homing, interpret the switch as homing trigger
        stepper.stop();
        homingTimestamp = now;
        // advance to backoff stage
        if (homingStage == 1)
        {
          homingStage = 2;
        }
        else if (homingStage == 31)
        {
          // fine approach trigger -> finalize homing
          stepper.setCurrentPosition(TOTALROND * MOTOR_STEPS * MICROSTEPS);
          uitvoeren = TOTALROND * MOTOR_STEPS * MICROSTEPS;
          target = 100;
          pref_target = target;
          current = target;
          stepper.disableOutputs();
          digitalWrite(ENABLE, HIGH);
          homingActive = false;
          homingStage = 0;
        }
      }
      else
      {
        // normal stop
        stopMotor();
      }
    }
    else
    {
      stopRequested = false;
    }
  }

  // Non-blocking homing state machine
  if (homingActive)
  {
    // Apply state machine
    if (homingStage == 1)
    {
      // coarse approach running; keep calling run until either triggered or timeout
      if (millis() - homingStartTime > HOMING_TIMEOUT_MS)
      {
        // timeout: abort homing
        homingActive = false;
        homingStage = 0;
      }
      else
      {
        stepper.run();
      }
    }
    else if (homingStage == 2)
    {
      // we received trigger during coarse approach; wait until stepper stops
      if (stepper.distanceToGo() != 0)
      {
        stepper.run();
      }
      else
      {
        // start backoff move
        stepper.setMaxSpeed(MOTORSPEED * HOMING_FAST_MULT);
        stepper.setAcceleration(MOTORACC * HOMING_FAST_MULT);
        stepper.move(HOMING_BACKOFF_STEPS);
        homingStage = 21;
      }
    }
    else if (homingStage == 21)
    {
      // running backoff
      if (stepper.distanceToGo() != 0)
      {
        stepper.run();
      }
      else
      {
        // start fine approach slowly
        stepper.setMaxSpeed(MOTORSPEED * HOMING_SLOW_MULT);
        stepper.setAcceleration(MOTORACC * HOMING_SLOW_MULT);
        stepper.moveTo(-HOMING_FINE_STEPS);
        homingStage = 31;
      }
    }
    else if (homingStage == 31)
    {
      // fine approach; run until switch triggers (handled in ISR) or timeout
      if (millis() - homingTimestamp > HOMING_TIMEOUT_MS)
      {
        // fine approach timeout -> abort
        homingActive = false;
        homingStage = 0;
      }
      else
      {
        stepper.run();
      }
    }
  }

  if (millis() - prev_millis >= 10000 && gohome != false)
  {
    gohome = false;
  }

  while (ishome == true)
  {
    homeing();
    pref_target = target;
    ishome = false;
  }

  stepper.moveTo(uitvoeren);

  if (stepper.targetPosition() != stepper.currentPosition())
  {
    digitalWrite(ENABLE, LOW);
    stepper.run();
    ismoved = true;
    if (totalstep != 0)
    {
      current = (byte)(stepper.currentPosition() / totalstep);
    }
    else
    {
      current = 0;
    }
    // Update state: 1 = opening (increasing), 2 = closing (decreasing)
    if (stepper.targetPosition() > stepper.currentPosition())
    {
      state = 1; // opening
    }
    else
    {
      state = 2; // closing
    }
    // current = (stepper.currentPosition() / (TOTALROND * MOTOR_STEPS * MICROSTEPS)) * 100.000L;
  }
  else
  {
    state = 0; // stopped
    if (ismoved == true)
    {
      stable();
    }
    if (pref_target != target && stepper.currentPosition() == uitvoeren)
    {
      pref_target = target;
      current = target;
      senddebug();
    }
    if (digitalRead(ENABLE) != HIGH)
    {
      digitalWrite(ENABLE, HIGH);
    }
  }
}
