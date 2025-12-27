#include <Arduino.h>
#include <AccelStepper.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#include "config.h"
#include "driver.h"
#include "processCommand.h"

SoftwareSerial esp8266(ESP_SW_RX, ESP_SW_TX); // RX, TX

byte target;//0-100%
byte pref_target;//0-100%
byte current;//0-100%
//int totalrond = 43200;//graden
//byte totalrond = 120;//rondjes
//byte totalrond = 180L;
long uitvoeren;
long totalstep;

// Create a new instance of the AccelStepper class:
AccelStepper stepper = AccelStepper(stepper.DRIVER, STEP, DIR);

volatile bool ishome = false;
bool ismoved = false;
volatile bool gohome = false;
unsigned long prev_millis;
volatile bool stopRequested = false;
volatile unsigned long lastHomeEvent = 0;
volatile bool homingActive = false;
volatile uint8_t homingStage = 0;
long homingTarget = 0;
volatile unsigned long homingStartTime = 0;
volatile unsigned long homingTimestamp = 0;

// ISR: only set a flag to keep ISR quick and safe
void stopMotorISR() {
  stopRequested = true;
}

void setup() {
  pinMode(ENABLE, OUTPUT);
  digitalWrite(ENABLE, LOW);

  totalstep = (totalrond * MOTOR_STEPS * MICROSTEPS) / 100L; // steps representing 1%

  if (DEBUG == true)
  {
    Serial.begin(19200);
    Serial.println("PLAY");
    Serial.println(totalstep);
  }
  configureRAdriver(SW_RX, SW_TX, R_SENSE, DRIVER_ADDRESS, R_current, 100);
  esp8266.begin(9600);

  pinMode(home_switch, INPUT);

  stepper.setPinsInverted(RA_INVERT_DIR, false, false);
  stepper.setMaxSpeed(motorSpeed * 8); //3200
  stepper.setAcceleration(motorAcc * 8); //3200
  stepper.setSpeed(motorSpeed * 8);
  
  attachInterrupt(digitalPinToInterrupt(home_switch), stopMotorISR, FALLING);

  stepper.setCurrentPosition(0);//384000);

  if (DEBUG == true)
  {
    Serial.println("STARTUP HOMING");
    senddebug();
  }
  gohome = false;
  stepper.runToNewPosition(totalrond * MOTOR_STEPS * MICROSTEPS);

  stepper.setMaxSpeed(motorSpeed * MICROSTEPS); //3200
  stepper.setAcceleration(motorAcc * MICROSTEPS); //3200
  stepper.setSpeed(motorSpeed * MICROSTEPS);
}

void loop() {
  while (esp8266.available() > 0)
  {
    processSerialData(esp8266);
  }

  // Handle any requested stop from ISR in non-ISR context
  if (stopRequested) {
    unsigned long now = millis();
    if (now - lastHomeEvent >= HOMING_DEBOUNCE_MS) {
      lastHomeEvent = now;
      stopRequested = false;
      if (homingActive) {
        // If homing, interpret the switch as homing trigger
        stepper.stop();
        homingTimestamp = now;
        // advance to backoff stage
        if (homingStage == 1) {
          homingStage = 2;
        } else if (homingStage == 31) {
          // fine approach trigger -> finalize homing
          stepper.setCurrentPosition(totalrond * MOTOR_STEPS * MICROSTEPS);
          uitvoeren = totalrond * MOTOR_STEPS * MICROSTEPS;
          target = 100;
          pref_target = target;
          current = target;
          stepper.disableOutputs();
          digitalWrite(ENABLE, HIGH);
          homingActive = false;
          homingStage = 0;
        }
      } else {
        // normal stop
        stopMotor();
      }
    } else {
      stopRequested = false;
    }
  }

  // Non-blocking homing state machine
  if (homingActive) {
    // Apply state machine
    if (homingStage == 1) {
      // coarse approach running; keep calling run until either triggered or timeout
      if (millis() - homingStartTime > HOMING_TIMEOUT_MS) {
        // timeout: abort homing
        homingActive = false;
        homingStage = 0;
      } else {
        stepper.run();
      }
    } else if (homingStage == 2) {
      // we received trigger during coarse approach; wait until stepper stops
      if (stepper.distanceToGo() != 0) {
        stepper.run();
      } else {
        // start backoff move
        stepper.setMaxSpeed(motorSpeed * HOMING_FAST_MULT);
        stepper.setAcceleration(motorAcc * HOMING_FAST_MULT);
        stepper.move(HOMING_BACKOFF_STEPS);
        homingStage = 21;
      }
    } else if (homingStage == 21) {
      // running backoff
      if (stepper.distanceToGo() != 0) {
        stepper.run();
      } else {
        // start fine approach slowly
        stepper.setMaxSpeed(motorSpeed * HOMING_SLOW_MULT);
        stepper.setAcceleration(motorAcc * HOMING_SLOW_MULT);
        stepper.moveTo(-HOMING_FINE_STEPS);
        homingStage = 31;
      }
    } else if (homingStage == 31) {
      // fine approach; run until switch triggers (handled in ISR) or timeout
      if (millis() - homingTimestamp > HOMING_TIMEOUT_MS) {
        // fine approach timeout -> abort
        homingActive = false;
        homingStage = 0;
      } else {
        stepper.run();
      }
    }
  }

  if (millis() - prev_millis >= 10000 && gohome != false) {
    gohome = false;
  }

  while (ishome == true) {
    homeing();
    pref_target = target;
    ishome = false;
  }

  stepper.moveTo(uitvoeren);

  if (stepper.targetPosition() != stepper.currentPosition()) {
    digitalWrite(ENABLE, LOW);
    stepper.run();
    ismoved = true;
    if (totalstep != 0) {
      current = (byte)(stepper.currentPosition() / totalstep);
    } else {
      current = 0;
    }
    //current = (stepper.currentPosition() / (totalrond * MOTOR_STEPS * MICROSTEPS)) * 100.000L;
  }
  else {
    if (ismoved == true) {
      stable();
    }
    if (pref_target != target && stepper.currentPosition() == uitvoeren) {
      pref_target = target;
      current = target;
      if (DEBUG == true)
      {
        senddebug();
        Serial.println("current != target && stepper.currentPosition() == uitvoeren"); Serial.println();
      }
    }
    if (digitalRead(ENABLE) != HIGH) {
      digitalWrite(ENABLE, HIGH);
    }
  }

}

void stable() {
  if (DEBUG == true)
  {
    Serial.println("stable");
  }
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
  if (DEBUG == true)
  {
    Serial.println("HOME");
  }
  // setup non-blocking two-step homing: coarse then fine
  gohome = true;
  stepper.setCurrentPosition(0);
  digitalWrite(ENABLE, LOW);
  // coarse approach
  stepper.setMaxSpeed(motorSpeed * HOMING_FAST_MULT);
  stepper.setAcceleration(motorAcc * HOMING_FAST_MULT);
  stepper.setSpeed(motorSpeed * HOMING_FAST_MULT);
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
      Serial.println("STOP");
    }
  }
}
