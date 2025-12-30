#if defined(VSCODE)
#include "functions.h"
#else 
#include "include/functions.h"
#endif

unsigned long lastreq = 0;
unsigned long MasterPreviousMillis = 0;

int sendData(const char* command, char* responseBuf, size_t respBufLen, const int timeout, bool set_target, uint8_t retries) {
  if (millis() - lastreq < REQUEST_TIMEOUT && set_target != true) {
    return 0;
  }
  lastreq = millis();

  for (uint8_t attempt = 0; attempt < retries; ++attempt) {
    size_t idx = 0;
    if (respBufLen > 0) responseBuf[0] = '\0';
    Serial.print(command);
    unsigned long start = millis();
    while ((millis() - start) < (unsigned long)timeout) {
      delay(0);
      while (Serial.available() && idx < respBufLen - 1) {
        char c = Serial.read();
        responseBuf[idx++] = c;
        if (c == '\n') break; // stop at newline terminator
      }
      if (idx > 0 && responseBuf[idx - 1] == '\n') break;
    }
    if (idx > 0) {
      // strip trailing whitespace/newline/carriage returns
      while (idx > 0 && (responseBuf[idx - 1] == '\n' || responseBuf[idx - 1] == '\r' || isspace((unsigned char)responseBuf[idx - 1]))) idx--;
      responseBuf[idx] = '\0';
      // strip leading whitespace
      size_t startPos = 0;
      while (startPos < idx && isspace((unsigned char)responseBuf[startPos])) startPos++;
      if (startPos > 0) {
        memmove(responseBuf, responseBuf + startPos, idx - startPos + 1);
      }
      return (int)strlen(responseBuf);
    }
    if (respBufLen > 0) responseBuf[0] = '\0';
    delay(50);
  }
  return 0; // no response
}

void deserial(const char* message) {
  if (message == NULL || message[0] == '\0') return;
  JsonDocument json;
  DeserializationError error = deserializeJson(json, message);
  if (error) return; // malformed JSON -> ignore
  if (json["target"].is<int>()) target_ont = uint8_t(json["target"]);
  if (json["current"].is<int>()) current_ont = uint8_t(json["current"]);
  if (json["state"].is<int>()) state_ont = int(json["state"]);
}

void set_Target_Pos(byte target_set) {
  char cmd[16];
  snprintf(cmd, sizeof(cmd), ":SA%u#", (unsigned)target_set);
  char resp[256];
  int len = sendData(cmd, resp, sizeof(resp), 500, true, 3);
  if (len > 0) deserial(resp);
  // Immediately report updated state to master
  send_change();
}

void homeing() {
  char cmd[] = ":SH#";
  char resp[256];
  int len = sendData(cmd, resp, sizeof(resp), 500, false, 2);
  if (len > 0) deserial(resp);
}

void get_info() {
  char cmd[] = ":GC#";
  char resp[256];
  int len = sendData(cmd, resp, sizeof(resp), 500, false, 2);
  if (len > 0) deserial(resp);
}

void set_reset() {
  digitalWrite(RES, HIGH);
  delay(1000);
  ESP.restart();
}

void ask_master() {
  if ((unsigned long)(millis() - MasterPreviousMillis) >= REQUEST_TIMEOUT) {
    MasterPreviousMillis = millis();
    JsonDocument doc;
    doc["got_master"] = false;
    doc["device"] = "curtain";
    doc["curtain_id"] = uint32_t(mesh.getNodeId());
    doc["target"] = target_ont;
    doc["current"] = current_ont;
    doc["state"] = state_ont;
    char buf[256];
    serializeJson(doc, buf, sizeof(buf));
    mesh.sendBroadcast(String(buf));
  }
}

void send_change() {
  if (master > 0) {
    get_info();
    JsonDocument doc;
    doc["got_master"] = true;
    doc["device"] = "curtain";
    doc["curtain_id"] = uint32_t(mesh.getNodeId());
    doc["target"] = target_ont;
    doc["current"] = current_ont;
    doc["state"] = state_ont;
    char buf[256];
    serializeJson(doc, buf, sizeof(buf));
    mesh.sendSingle(master, String(buf));
  } else {
    ask_master();
  }
}
