#include "functions.h"

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
      }
      if (idx > 0 && responseBuf[idx - 1] == '}') break; // likely end of JSON
    }
    if (idx < respBufLen) responseBuf[idx] = '\0';
    else responseBuf[respBufLen - 1] = '\0';
    if (idx > 0) return (int)idx;
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
