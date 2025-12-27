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
      esp8266.print('P');
      esp8266.print('\n');
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
        char respBuf[192];
        respBuf[0] = '\0';
        bool ok = processCommand(cmdBuf, respBuf, sizeof(respBuf));
        if (ok && respBuf[0] != '\0') {
          esp8266.print(respBuf);
          esp8266.print('\n');
          if (DEBUG == true) {
            Serial.print("send: ");
            Serial.println(respBuf);
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


bool processCommand(const char *inCmd, char *respBuf, size_t respBufLen)
{
  if (inCmd == NULL || inCmd[0] == '\0') return false;
  if (inCmd[0] == ':')
  {
    // copy and remove spaces into a local buffer
    char tmp[64];
    size_t ti = 0;
    for (size_t i = 0; inCmd[i] != '\0' && ti < sizeof(tmp) - 1; ++i) {
      if (inCmd[i] == ' ') continue;
      tmp[ti++] = inCmd[i];
    }
    tmp[ti] = '\0';
    char command = tmp[1];
    const char *rest = &tmp[2];
    switch (command)
    {
      case 'S': // set
        return handleMeadeSetInfo(rest, respBuf, respBufLen);
      case 'G': // get
        return handleMeadeGetInfo(rest, respBuf, respBufLen);
      default:
        break;
    }
  }
  return false;
}

bool handleMeadeSetInfo(const char *inCmd, char *respBuf, size_t respBufLen)
{
  if (inCmd == NULL || inCmd[0] == '\0') return false;
  if (inCmd[0] == 'A')  // set target pos :SA100#
  {
    int parsed = atoi(inCmd + 1);
    if (parsed < 0) parsed = 0;
    if (parsed > 100) parsed = 100;
    target = (byte)parsed;
    long totalSteps = totalrond * MOTOR_STEPS * MICROSTEPS;
    uitvoeren = (totalSteps * (long)target) / 100L;
    return jsoninfo(respBuf, respBufLen);
  }
  if (inCmd[0] == 'H')  // go home :SH#
  {
    homeing();
    return jsoninfo(respBuf, respBufLen);
  }
  return false;
}

bool handleMeadeGetInfo(const char *inCmd, char *respBuf, size_t respBufLen)
{
  if (inCmd == NULL || inCmd[0] == '\0') return false;
  char cmdOne = inCmd[0];

  switch (cmdOne)
  {
    case 'C': // get current pos
      return jsoninfo(respBuf, respBufLen);
    default:
      break;
  }
  return false;
}

bool jsoninfo(char *respBuf, size_t respBufLen) {
  if (respBuf == NULL || respBufLen == 0) return false;
  JsonDocument root;
  root["target"] = target;
  root["current"] = current;
  if (target > current) {
    root["state"] = 0;
  } else if (target < current) {
    root["state"] = 1;
  } else {
    root["state"] = 2;
  }
  size_t n = serializeJson(root, respBuf, respBufLen);
  return n > 0;
}
