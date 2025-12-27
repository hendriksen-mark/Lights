#include "processCommand.h"

void senddebug() {
  Serial.println();
  Serial.print("target : "); Serial.println(target);
  Serial.print("current : "); Serial.println(current);
  Serial.print("uitvoeren : "); Serial.println(uitvoeren);
  Serial.print("stepper.targetPosition() : "); Serial.println(stepper.targetPosition());
  Serial.print("stepper.currentPosition() : "); Serial.println(stepper.currentPosition());
}

void processSerialData(SoftwareSerial &esp8266)
{
  static char cmdBuf[64];
  static uint8_t idx = 0;

  while (esp8266.available() > 0) {
    int c = esp8266.read();
    if (c < 0) break;

    if (c == 0x06) {
      esp8266.println('P');
      continue;
    }

    if (c == '#') {
      // terminate and process
      cmdBuf[idx] = '\0';
      if (idx > 0) {
        if (DEBUG == true) {
          Serial.print("rec: ");
          Serial.println(cmdBuf);
        }
        String retVal = processCommand(String(cmdBuf));
        if (retVal != "") {
          esp8266.println(retVal);
          if (DEBUG == true) {
            Serial.print("send: ");
            Serial.println(retVal);
            senddebug();
          }
        }
      }
      idx = 0;
    } else {
      // accumulate (discard if overflow)
      if (idx < sizeof(cmdBuf) - 1) {
        cmdBuf[idx++] = (char)c;
      } else {
        // buffer overflow: reset
        idx = 0;
      }
    }
  }
}


String processCommand(String inCmd)
{
  if (inCmd[0] == ':')
  {
    // Apparently some LX200 implementations put spaces in their commands..... remove them with impunity.
    int spacePos;
    while ((spacePos = inCmd.indexOf(' ')) != -1)
    {
      inCmd.remove(spacePos, 1);
    }
    char command = inCmd[1];
    inCmd        = inCmd.substring(2);
    switch (command)
    {
      case 'S': // set
        return handleMeadeSetInfo(inCmd);
      case 'G': // get
        return handleMeadeGetInfo(inCmd);
      default:
        break;
    }
  }
  return "";
}

String handleMeadeSetInfo(String inCmd)
{
  if (inCmd[0] == 'A')  // set target pos :SA100#
  {
    target = inCmd.substring(1, 4).toInt();
    long totalSteps = totalrond * MOTOR_STEPS * MICROSTEPS;
    uitvoeren = (totalSteps * (long)target) / 100L;
    //uitvoeren = uitvoeren * MOTOR_STEPS * MICROSTEPS;
    return jsoninfo();
  }
  if (inCmd[0] == 'H')  // go home :SH#
  {
    homeing();
    return jsoninfo();
  }
  return "";
}

String handleMeadeGetInfo(String inCmd)
{
  char cmdOne = inCmd[0];

  switch (cmdOne)
  {
    case 'C': // get current pos
      return jsoninfo();
    default:
      break;
  }
  return "";
}

String jsoninfo() {
  JsonDocument root;
  root["target"] = target;
  root["current"] = current;
  if (target > current) {
    root["state"] =  "0";
  }
  if (target < current) {
    root["state"] =  "1";
  }
  if (target == current) {
    root["state"] =  "2";
  }
  String output;
  serializeJson(root, output);
  return output;
}
