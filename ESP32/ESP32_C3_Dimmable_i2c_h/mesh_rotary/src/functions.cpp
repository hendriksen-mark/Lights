#if defined(VSCODE)
#include "functions.h"
#else
#include "include/functions.h"
#endif

void receivedCallback(uint32_t from, String& msg) {

  JsonDocument root;
  DeserializationError error = deserializeJson(root, msg);

  if (error)
  {
    //REMOTE_LOG_ERROR("deserializeJson() failed:", error.c_str());
    return;
  }

  if (root["master"].is<uint32_t>()) {
    master = root["master"];
  }
}

void ask_master() {
  if ((unsigned long)(millis() - MasterPreviousMillis) >= REQUEST_TIMEOUT) {
    MasterPreviousMillis = millis();
    JsonDocument doc;
    doc["got_master"] = false;
    doc["device"] = "switch";
    doc["switch_id"] = uint32_t(mesh.getNodeId());
    char buf[128];
    serializeJson(doc, buf, sizeof(buf));
    mesh.sendBroadcast(buf);
  }
}

void send_change() {
  if (change == true && master > 0) {
    JsonDocument doc;
    doc["got_master"] = true;
    doc["device"] = "switch";
    doc["room_mac"] = macToStr(mac0[room]);
    doc["value"] = value;
    char buf[128];
    serializeJson(doc, buf, sizeof(buf));
    mesh.sendSingle(master, buf);
  } else {
    ask_master();
  }
}

String macToStr(const uint8_t* mac) {
  String result;
  for (int i = 0; i < 6; ++i) {
    if (mac[i] < 0x10) result += F("0");
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  result.toUpperCase();
  return result;
}
