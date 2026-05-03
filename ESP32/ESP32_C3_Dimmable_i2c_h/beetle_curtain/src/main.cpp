#if defined(ESP_PLATFORM)
#include "functions.h"
#include "driver.h"
#else
#include "include/functions.h"
#include "include/driver.h"
#endif

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
  {
    LOG_ERROR("deserializeJson() failed: ", error.f_str());
    return;
  }
  LOG_DEBUG("Received message from ", from, ": ", msg);

  if (root["master"].is<unsigned long>())
  {
    master = uint32_t(root["master"]);
  }

  if (root["device"] == DEVICE_NAME)
  {
    bool homingCmd = root["homing"].is<bool>() && root["homing"].as<bool>();

    if (root["target"].is<int>() && !homingCmd)
    {
      // Don't call set_Target_Pos() here - just set flag
      pendingTarget = byte(root["target"]);
      newTargetReceived = true;
    }
    if (homingCmd)
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
  // Initialize RGB LED
  statusLED.Begin();
  setLedColorRGB(0, 0, 255); // Blue during startup

  Serial.begin(115200);
  serialWait();

  mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | COMMUNICATION );
  mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.initOTAReceive(DEVICE_NAME);

  configureDualDrivers();
  
  pinMode(HOME_SWITCH, INPUT);

  stepper.setEnablePin(ENABLE);
  stepper.setPinsInverted(INVERT_DIR, INVERT_STEP, INVERT_ENABLE);
  stepper.disableOutputs();
  stepper.setMaxSpeed(HOME_MOTOR_SPEED * MICROSTEPS);   // 200*16=3200
  stepper.setAcceleration(HOME_MOTOR_ACC * MICROSTEPS); // 200*16=3200
  stepper.setSpeed(HOME_MOTOR_SPEED * MICROSTEPS);      // 200*16=3200

  attachInterrupt(digitalPinToInterrupt(HOME_SWITCH), stopMotorISR, FALLING);

  stepper.setCurrentPosition(0); // 384000);

  // Allow mesh to connect before starting motors (reduces WiFi interference)
  LOG_INFO("Waiting for mesh connection...");
  setLedColorRGB(255, 0, 255); // Magenta during mesh connection
  unsigned long meshWaitStart = millis();
  while (millis() - meshWaitStart < 5000) // Wait up to 5 seconds
  {
    mesh.update();
    delay(10);
  }

  LOG_INFO("STARTUP HOMING");
  setLedColorRGB(255, 255, 0); // Yellow during homing
  senddebug();
  gohome = false;

  // Non-blocking startup homing that respects interrupt
  stepper.enableOutputs();
  stepper.moveTo(TOTALROND * MOTOR_STEPS * MICROSTEPS); // 123*200*16=39321600 steps for 123 revs
  while (stepper.distanceToGo() != 0)
  {
    mesh.update(); // Keep mesh connected during startup homing
    
    if (stopRequested)
    {
      stepper.moveTo(stepper.currentPosition());
      stopRequested = false;

      // Home switch reached: move right to real home offset, then establish 0%
      stepper.setMaxSpeed(MOTORSPEED * HOMING_FAST_MULT);
      stepper.setAcceleration(MOTORACC * HOMING_FAST_MULT);
      stepper.move(-HOMING_ZERO_OFFSET_STEPS);
      while (stepper.distanceToGo() != 0)
      {
        mesh.update();
        stepper.run();
      }

      stepper.setCurrentPosition(0);
      uitvoeren = 0;
      target = 0;
      pref_target = 0;
      current = 0;
      state = 2; // idle
      restoreNormalMotionProfile();
      LOG_INFO("STARTUP HOMING COMPLETE");
      break;
    }
    stepper.run();
  }
  stepper.disableOutputs();

  restoreNormalMotionProfile();
  
  // Startup complete - set LED to green
  setLedError(LED_OK);

  LOG_INFO("Setup complete - system ready");
}

void loop()
{
  mesh.update();

  // Check mesh connection status and update LED
  if (master == 0)
  {
    setLedError(LED_MESH_DISCONNECTED);
    send_change(false);
  }
  else
  {
    // Only set to OK if not in homing or other error state
    if (!homingActive && stepper.distanceToGo() == 0)
    {
      setLedError(LED_OK);
    }
  }

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
    send_change(true);
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
        stepper.moveTo(stepper.currentPosition());
        homingTimestamp = now;
        // Match setup behavior: once trigger is hit, go directly to right-offset zeroing
        if (homingStage == 1 || homingStage == 31)
        {
          // fine approach trigger -> move to real home offset before zeroing
          homingStage = 32;
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

  // Apply homing state machine
  if (homingActive)
  {
    if (homingStage == 1)
    {
      // coarse approach running; keep calling run until either triggered or timeout
      if (millis() - homingStartTime > HOMING_TIMEOUT_MS)
      {
        LOG_ERROR("Homing timeout during coarse approach");
        setLedError(LED_HOMING_TIMEOUT);
        ledBlink(0x00FFFF00, 5, 200); // Blink yellow 5 times
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
      // trigger received during coarse approach; wait until motion fully stops
      if (stepper.distanceToGo() != 0)
      {
        stepper.run();
      }
      else
      {
        // back off away from the switch (negative = rightward)
        stepper.setMaxSpeed(MOTORSPEED * HOMING_FAST_MULT);
        stepper.setAcceleration(MOTORACC * HOMING_FAST_MULT);
        stepper.move(-HOMING_BACKOFF_STEPS);
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
        // fine approach slowly toward switch (positive = toward home)
        stepper.setMaxSpeed(MOTORSPEED * HOMING_SLOW_MULT);
        stepper.setAcceleration(MOTORACC * HOMING_SLOW_MULT);
        stepper.move(HOMING_FINE_STEPS);
        homingStage = 31;
      }
    }
    else if (homingStage == 31)
    {
      // fine approach running; ISR finalizes when switch triggers
      if (millis() - homingStartTime > HOMING_TIMEOUT_MS)
      {
        LOG_ERROR("Homing timeout during fine approach");
        setLedError(LED_HOMING_TIMEOUT);
        ledBlink(0x00FFFF00, 5, 200); // Blink yellow 5 times
        homingActive = false;
        homingStage = 0;
      }
      else
      {
        stepper.run();
      }
    }
    else if (homingStage == 32)
    {
      // wait until fine-approach stop is fully settled, then move right to real home
      if (stepper.distanceToGo() != 0)
      {
        stepper.run();
      }
      else
      {
        stepper.setMaxSpeed(MOTORSPEED * HOMING_FAST_MULT);
        stepper.setAcceleration(MOTORACC * HOMING_FAST_MULT);
        stepper.move(-HOMING_ZERO_OFFSET_STEPS);
        homingStage = 41;
      }
    }
    else if (homingStage == 41)
    {
      // running rightward zero-offset move; finalize at real home
      if (stepper.distanceToGo() != 0)
      {
        stepper.run();
      }
      else
      {
        stepper.setCurrentPosition(0);
        uitvoeren = 0;
        target = 0;
        pref_target = target;
        current = target;
        newTargetReceived = false;
        state = 2; // idle
        restoreNormalMotionProfile();
        stepper.disableOutputs();
        homingActive = false;
        homingStage = 0;
      }
    }
  }
  else if (stepper.distanceToGo() != 0)
  {
    // Motor is moving - show cyan
    setLedColorRGB(0, 255, 255);
    stepper.run();
  }

  // While homing is active, skip normal target/state processing.
  if (homingActive)
  {
    return;
  }

  if (millis() - prev_millis >= 10000 && gohome != false)
  {
    gohome = false;
  }

  while (ishome == true)
  {
    newTargetReceived = false;
    homeing();
    pref_target = target;
    ishome = false;
  }

  // homeing() may have been started above in this same loop iteration.
  // Do not let normal moveTo() overwrite the homing motion target.
  if (homingActive)
  {
    return;
  }

  stepper.moveTo(uitvoeren);

  if (stepper.targetPosition() != stepper.currentPosition())
  {
    stepper.enableOutputs();
    stepper.run();
    ismoved = true;
    if (TOTALSTEPS != 0)
    {
      long pos = -stepper.currentPosition();
      if (pos < 0)
      {
        pos = 0;
      }
      else if (pos > TOTALSTEPS)
      {
        pos = TOTALSTEPS;
      }
      current = (byte)((pos * 100L) / TOTALSTEPS);
    }
    else
    {
      current = 0;
    }
    // State mapping: 0 = closing, 1 = opening, 2 = idle
    if (stepper.targetPosition() > stepper.currentPosition())
    {
      state = 0; // closing (toward home)
    }
    else
    {
      state = 1; // opening (away from home)
    }
    // current = (stepper.currentPosition() / (TOTALROND * MOTOR_STEPS * MICROSTEPS)) * 100.000L;
  }
  else
  {
    state = 2; // idle
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
    stepper.disableOutputs();
  }
}
