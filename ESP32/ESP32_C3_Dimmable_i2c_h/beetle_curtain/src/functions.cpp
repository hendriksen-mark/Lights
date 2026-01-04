#if defined(ARDUINO_ESP32C3_DEV)
#include "functions.h"
#else
#include "include/functions.h"
#endif

unsigned long MasterPreviousMillis = 0;

void set_Target_Pos(byte target_set) {
  if (target_set > 100) target_set = 100;
  target = target_set;
  long totalSteps = TOTALROND * MOTOR_STEPS * MICROSTEPS;
  uitvoeren = (totalSteps * (long)target) / 100L;
  // Note: send_change() will be called from main loop when request flag is set
}

void stable() {
  LOG_DEBUG("stable");
  long i = 0;
  if (target > pref_target && target >= 2) {
    i = 2 * MOTOR_STEPS * MICROSTEPS;
  } else if ( target < pref_target && target >= 2) {
    i = -2 * MOTOR_STEPS * MICROSTEPS;
  }

  stepper.move(i);
  stepper.runToPosition();

  if (target > pref_target && target >= 2) {
    i = -2 * MOTOR_STEPS * MICROSTEPS;
  } else if ( target < pref_target && target >= 2) {
    i = 2 * MOTOR_STEPS * MICROSTEPS;
  }

  stepper.move(i);
  stepper.runToPosition();
  ismoved = false;
}


void homeing() {
  LOG_DEBUG("HOME");
  // setup non-blocking two-step homing: coarse then fine
  gohome = true;
  stepper.setCurrentPosition(0);
  digitalWrite(ENABLE, LOW);
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

void stopMotor() {
  if (gohome == false) {
    stepper.stop();
    digitalWrite(ENABLE, HIGH);
    prev_millis = millis();
    ishome = true;
    if (DEBUG == true)
    {
      LOG_ERROR("STOP");
    }
  }
}

void ask_master() {
  if ((unsigned long)(millis() - MasterPreviousMillis) >= REQUEST_TIMEOUT) {
    MasterPreviousMillis = millis();
    JsonDocument doc;
    doc["got_master"] = false;
    doc["device"] = "curtain";
    doc["curtain_id"] = uint32_t(mesh.getNodeId());
    doc["target"] = target;
    doc["current"] = current;
    doc["state"] = state;
    char buf[128];
    serializeJson(doc, buf, sizeof(buf));
    mesh.sendBroadcast(buf);
  }
}

void send_change() {
  if (master > 0) {
    JsonDocument doc;
    doc["got_master"] = true;
    doc["device"] = "curtain";
    doc["curtain_id"] = uint32_t(mesh.getNodeId());
    doc["target"] = target;
    doc["current"] = current;
    doc["state"] = state;
    char buf[128];
    serializeJson(doc, buf, sizeof(buf));
    mesh.sendSingle(master, buf);
  } else {
    ask_master();
  }
}

void senddebug() {
  LOG_DEBUG("target :", target);
  LOG_DEBUG("current :", current);
  LOG_DEBUG("uitvoeren :", uitvoeren);
  LOG_DEBUG("stepper.targetPosition() :", stepper.targetPosition());
  LOG_DEBUG("stepper.currentPosition() :", stepper.currentPosition());
}
