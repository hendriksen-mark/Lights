#include "painlessMesh.h"
#include <Arduino_JSON.h>

#define   MESH_PREFIX     "HomeMesh"
#define   MESH_PASSWORD   "Qwertyuiop1"
#define   MESH_PORT       5555
painlessMesh  mesh;

int value;
String room_mac;
bool change = false;

void mesh_setup() {
  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  newConnectionCallback(0);

}

void mesh_loop() {
  mesh.update();
  if (change == true) {
    LOG_ERROR(sendHttpRequest(value, room_mac));
    //procesdata(room, value);
    change = false;
  }
}

void newConnectionCallback(uint32_t nodeId) {
  JSONVar jsonReadings;
  jsonReadings["master"] = mesh.getNodeId();
  String msg = JSON.stringify(jsonReadings);
  if (nodeId > 0) {
    mesh.sendSingle(nodeId, msg);
  } else {
    mesh.sendBroadcast(msg);
  }
}

void receivedCallback( uint32_t from, String &msg ) {
  JSONVar myObject = JSON.parse(msg);
  if (bool(myObject["got_master"]) == true) {
    room_mac = (const char*)myObject["room_mac"];
    value = (int)myObject["value"];
    change = true;
  } else {
    newConnectionCallback(from);
  }
}
