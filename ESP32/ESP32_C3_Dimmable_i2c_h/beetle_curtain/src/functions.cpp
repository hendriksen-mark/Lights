#if defined(ESP_PLATFORM)
#include "functions.h"
#else
#include "include/functions.h"
#endif

unsigned long MasterPreviousMillis = 0;

void serialWait()
{
	// Give user time to open serial monitor: print a heartbeat during the 10s wait
	pinMode(LED_PIN, OUTPUT);
  const int waitMs = 10000;
	const int intervalMs = 500;
	int loops = waitMs / intervalMs;
	for (int i = 0; i < loops; ++i)
	{
		LOG_INFO("Waiting for serial monitor...", (waitMs - i * intervalMs) / 1000, "seconds remaining");
		infoLedPulse(1, intervalMs); // Pulse white while waiting for serial monitor
	}
}

void infoLedPulse(uint8_t pulses, uint16_t pulseDuration)
{
	for (uint8_t p = 0; p < pulses; p++)
	{
		infoLedFadeIn(pulseDuration / 2);
		infoLedFadeOut(pulseDuration / 2);
		if (p < pulses - 1)
		{
			delay(pulseDuration / 4); // Short pause between pulses
		}
	}
}

void infoLedFadeIn(uint16_t duration)
{

	uint8_t steps = 50;
	uint16_t stepDelay = duration / steps;

	for (uint8_t i = 0; i <= steps; i++)
	{
		float progress = (float)i / steps;
    analogWrite(LED_PIN, (uint8_t)(progress * 255));
		delay(stepDelay);
	}
}

void infoLedFadeOut(uint16_t duration)
{
	uint8_t steps = 50;
	uint16_t stepDelay = duration / steps;

	for (uint8_t i = steps; i > 0; i--)
	{
		float progress = (float)i / steps;
    analogWrite(LED_PIN, (uint8_t)(progress * 255));
		delay(stepDelay);
	}
  analogWrite(LED_PIN, 0); // Ensure LED is fully off at the end
}

void set_Target_Pos(byte target_set)
{
  if (target_set > 100)
    target_set = 100;
  target = target_set;
  long totalSteps = TOTALROND * MOTOR_STEPS * MICROSTEPS;
  uitvoeren = (totalSteps * (long)target) / 100L;
  // Note: send_change() will be called from main loop when request flag is set
}

void stable()
{
  LOG_DEBUG("stable");
  long i = 0;
  if (target > pref_target && target >= 2)
  {
    i = 2 * MOTOR_STEPS * MICROSTEPS;
  }
  else if (target < pref_target && target >= 2)
  {
    i = -2 * MOTOR_STEPS * MICROSTEPS;
  }

  stepper.move(i);
  stepper.runToPosition();

  if (target > pref_target && target >= 2)
  {
    i = -2 * MOTOR_STEPS * MICROSTEPS;
  }
  else if (target < pref_target && target >= 2)
  {
    i = 2 * MOTOR_STEPS * MICROSTEPS;
  }

  stepper.move(i);
  stepper.runToPosition();
  ismoved = false;
}

void homeing()
{
  LOG_DEBUG("HOME");
  // setup non-blocking two-step homing: coarse then fine
  gohome = true;
  stepper.setCurrentPosition(0);
  stepper.enableOutputs();
  // coarse approach
  stepper.setMaxSpeed(MOTORSPEED * HOMING_FAST_MULT);
  stepper.setAcceleration(MOTORACC * HOMING_FAST_MULT);
  stepper.setSpeed(MOTORSPEED * HOMING_FAST_MULT);
  stepper.moveTo(-HOMING_COARSE_STEPS);
  homingActive = true;
  homingStage = 1;
  homingStartTime = millis();
  homingTimestamp = homingStartTime;
}

void stopMotor()
{
  if (gohome == false)
  {
    stepper.stop();
    stepper.disableOutputs();
    prev_millis = millis();
    ishome = true;
    if (DEBUG == true)
    {
      LOG_ERROR("STOP");
    }
  }
}

void send_change(bool toMaster = false)
{
  // Check timeout only when broadcasting (looking for master)
  if (!toMaster && (unsigned long)(millis() - MasterPreviousMillis) < REQUEST_TIMEOUT)
  {
    return;
  }
  
  if (!toMaster)
  {
    MasterPreviousMillis = millis();
  }

  JsonDocument doc;
  doc["got_master"] = toMaster;
  doc["device"] = DEVICE_NAME;
  doc["curtain_id"] = uint32_t(mesh.getNodeId());
  doc["target"] = target;
  doc["current"] = current;
  doc["state"] = state;
  char buf[128];
  serializeJson(doc, buf, sizeof(buf));
  
  if (toMaster && master > 0)
  {
    mesh.sendSingle(master, buf);
  }
  else
  {
    mesh.sendBroadcast(buf);
  }
}

void senddebug()
{
  LOG_DEBUG("target :", target);
  LOG_DEBUG("current :", current);
  LOG_DEBUG("uitvoeren :", uitvoeren);
  LOG_DEBUG("stepper.targetPosition() :", stepper.targetPosition());
  LOG_DEBUG("stepper.currentPosition() :", stepper.currentPosition());
}
