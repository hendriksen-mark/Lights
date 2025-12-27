#include "functions.h"

painlessMesh mesh;

uint32_t master = 0;

byte target_set;//0-100%
byte target_ont;//0-100%
byte current_ont;//0-100%

int state_ont;//0 1 2

void receivedCallback(uint32_t from, String& msg) {

  JsonDocument root;
  DeserializationError error = deserializeJson(root, msg);
  if (error) return;

  if (root["master"].is<unsigned long>()) {
    master = uint32_t(root["master"]);
  }

  if (root["device"] == "curtain") {
    if (root["target"].is<int>()) {
      target_set = byte(root["target"]);
      set_Target_Pos(target_set);
    }
    if (root["homing"].is<bool>() && root["homing"].as<bool>()) {
      homeing();
    }
    if (root["request"].is<bool>() && root["request"].as<bool>()) {
      send_change();
    }
  }
}

void setup() {
  Serial.begin(9600);
  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | COMMUNICATION );
  mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  pinMode(RES, OUTPUT);
  digitalWrite(RES, LOW);
}

void loop() {
  mesh.update();
  if (master == 0) {
    ask_master();
  }
}
