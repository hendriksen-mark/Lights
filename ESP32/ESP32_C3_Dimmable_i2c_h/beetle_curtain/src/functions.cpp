#if defined(ESP_PLATFORM)
#include "functions.h"
#else
#include "include/functions.h"
#endif

unsigned long MasterPreviousMillis = 0;

void serialWait()
{
	// Give user time to open serial monitor: print a heartbeat during the 10s wait
  const int waitMs = 10000;
	const int intervalMs = 500;
	int loops = waitMs / intervalMs;
	for (int i = 0; i < loops; ++i)
	{
		LOG_INFO("Waiting for serial monitor...", (waitMs - i * intervalMs) / 1000, "seconds remaining");
		infoLedPulse(1, intervalMs); // Pulse white while waiting for serial monitor
	}
}

void setLedColor(uint32_t color)
{
	// Extract RGB from 32-bit color (0x00RRGGBB format)
	uint8_t r = (color >> 16) & 0xFF;
	uint8_t g = (color >> 8) & 0xFF;
	uint8_t b = color & 0xFF;
	statusLED.SetPixelColor(0, RgbColor(r, g, b));
	statusLED.Show();
}

void setLedColorRGB(uint8_t r, uint8_t g, uint8_t b)
{
	statusLED.SetPixelColor(0, RgbColor(r, g, b));
	statusLED.Show();
}

void setLedError(LedErrorCode errorCode)
{
	switch (errorCode)
	{
		case LED_OK:
			setLedColorRGB(0, 255, 0); // Green - normal operation
			break;
		case LED_MESH_DISCONNECTED:
			setLedColorRGB(255, 0, 0); // Red - mesh connection lost
			break;
		case LED_HOMING_TIMEOUT:
			setLedColorRGB(255, 255, 0); // Yellow - homing timeout
			break;
		case LED_MOTOR_STALL:
			setLedColorRGB(255, 128, 0); // Orange - motor stall
			break;
		case LED_GENERAL_ERROR:
			setLedColorRGB(255, 0, 255); // Magenta - general error
			break;
		default:
			setLedColorRGB(255, 0, 0); // Red for unknown errors
			break;
	}
}

void ledBlink(uint32_t color, uint8_t times, uint16_t duration)
{
	for (uint8_t i = 0; i < times; i++)
	{
		setLedColor(color);
		delay(duration / 2);
		setLedColor(0); // Turn off
		delay(duration / 2);
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
		uint8_t brightness = (uint8_t)(progress * 255);
		setLedColorRGB(brightness, brightness, brightness); // White fade
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
		uint8_t brightness = (uint8_t)(progress * 255);
		setLedColorRGB(brightness, brightness, brightness); // White fade
		delay(stepDelay);
	}
	setLedColor(0); // Ensure LED is fully off at the end
}

void set_Target_Pos(byte target_set)
{
  if (target_set > 100)
    target_set = 100;
  target = target_set;
  // Home is 0%. Higher percentages move away from home (negative direction).
  uitvoeren = -((TOTALSTEPS * (long)target) / 100L);
  // Note: send_change() will be called from main loop when request flag is set
}

void restoreNormalMotionProfile()
{
  stepper.setMaxSpeed(MOTORSPEED * MICROSTEPS);
  stepper.setAcceleration(MOTORACC * MICROSTEPS);
  stepper.setSpeed(MOTORSPEED * MICROSTEPS);
}

void stable()
{
  LOG_DEBUG("stable");
  long i = 0;
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
  ismoved = false;
}

void homeing()
{
  LOG_DEBUG("HOME");
  // setup non-blocking homing: long search to trigger, then right-offset zeroing
  gohome = true;
  stopRequested = false;
  stepper.enableOutputs();
  stepper.setMaxSpeed(MOTORSPEED * HOMING_FAST_MULT);
  stepper.setAcceleration(MOTORACC * HOMING_FAST_MULT);
  stepper.setSpeed(MOTORSPEED * HOMING_FAST_MULT);

  // If the sensor is already active, skip coarse approach and go straight to backoff/fine approach.
  if (digitalRead(HOME_SWITCH) == LOW)
  {
    stepper.moveTo(stepper.currentPosition());
    homingStage = 32;
  }
  else
  {
    // Coarse search: always move far enough to reach trigger from any commanded position.
    stepper.move(HOMING_SEARCH_STEPS);
    homingStage = 1;
  }

  homingActive = true;
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
  LOG_DEBUG("Sending status update:", buf);
  
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
