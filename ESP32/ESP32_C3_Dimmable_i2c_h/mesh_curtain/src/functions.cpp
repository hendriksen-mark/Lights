#include "functions.h"

unsigned long lastreq = 0;
unsigned long MasterPreviousMillis = 0;

String sendData(String command, const int timeout, bool set_target) {
  if (millis() - lastreq < REQUEST_TIMEOUT && set_target != true) {
    return "";
  } else {
    lastreq = millis();
    String response = "";
    Serial.print(command); // Send command to UNO/ESP (avoid automatic newline)
    response.reserve(256);
    unsigned long start = millis();
    while ((start + (unsigned long)timeout) > millis()) {
      delay(0);
      while (Serial.available()) { // De ESP heeft data om weer te geven, laat het zien in de serial monitor.
        char c = Serial.read(); // Lees het volgende karakter.
        response += c;
      }
    }
    return response;
  }
}

void deserial(String message) {
  if (message == "") return;
  JsonDocument json;
  DeserializationError error = deserializeJson(json, message);
  if (error) return; // malformed JSON -> ignore
  if (json["target"].is<int>()) target_ont = uint8_t(json["target"]);
  if (json["current"].is<int>()) current_ont = uint8_t(json["current"]);
  if (json["state"].is<int>()) state_ont = int(json["state"]);
}

void set_Target_Pos(byte target_set) {
  String message = ":SA";
  message += target_set;
  message += "#";
  deserial(sendData(message, 500, true));
  // Immediately report updated state to master
  send_change();
}


void homeing() {
  String message = ":SH#";
  deserial(sendData(message, 500, false));
}

void get_info() {
  String message = ":GC#";
  deserial(sendData(message, 500, false));
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
    String msg;
    serializeJson(doc, msg);
    mesh.sendBroadcast(msg);
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
    String msg;
    serializeJson(doc, msg);
    mesh.sendSingle(master, msg);
  }
  if (master == 0) {
    ask_master();
  }
}
