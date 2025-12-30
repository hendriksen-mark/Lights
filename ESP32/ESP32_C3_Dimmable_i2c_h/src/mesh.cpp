#include "mesh.h"
#include "functions.h"

int subip = 0;
IPAddress bridgeIp;
int bridgePort = BRIDGE_PORT;

painlessMesh mesh;

int value;
String room_mac;
bool change = false;
uint32_t curtain_id = 0;

byte target;  // 0-100%
byte current; // 0-100%

byte target_ont;  // 0-100%
byte current_ont; // 0-100%

bool fout = false;

int state_ont; // 0 1 2

WebServer server_gordijn(PORDIJN_SERVER_PORT);
WebServer server_mesh(MESH_SERVER_PORT);

void loadMeshConfig()
{
  JsonDocument doc;
  if (!readJsonFile(MESH_CONFIG_PATH, doc))
  {
    LOG_DEBUG("mesh config not found, using defaults");
    return;
  }
  // parse base address from BRIDGE_IP string and set default subip
  IPAddress base;
  base.fromString(String(BRIDGE_IP));
  subip = base[3];
  if (doc["subip"].is<int>())
  {
    int v = doc["subip"];
    if (v >= 0 && v <= 255)
    {
      subip = v;
      LOG_DEBUG("Loaded mesh subip:", subip);
    }
  }
  base[3] = subip;
  bridgeIp = base;
  if (doc["bridge"].is<int>())
  {
    int v = doc["bridge"];
    if (v > 0 && v <= 65535)
    {
      bridgePort = v;
      LOG_DEBUG("Loaded mesh bridge port:", bridgePort);
    }
  }
}

void saveMeshConfig()
{
  JsonDocument doc;
  doc["subip"] = subip;
  doc["bridge"] = bridgePort;
  if (!writeJsonFile(MESH_CONFIG_PATH, doc))
  {
    LOG_DEBUG("Failed to save mesh config");
  }
  else
  {
    LOG_DEBUG("Saved mesh config");
  }
}

void mesh_setup()
{
  // mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE | DEBUG ); // all types on
  // mesh.setDebugMsgTypes( ERROR | CONNECTION | SYNC | S_TIME );  // set before init() so that you can see startup messages
  // mesh.setDebugMsgTypes( ERROR | CONNECTION | S_TIME );  // set before init() so that you can see startup messages
  mesh.setDebugMsgTypes(ERROR | MESH_STATUS | CONNECTION | COMMUNICATION);
  // mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT);
  mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT, MESH_CONNECT_MODE, MESH_HIDDEN);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  newConnectionCallback(0);

  loadMeshConfig();

  server_gordijn.on(F("/"), handleRoot);
  server_gordijn.on("/setTargetPosTest/", set_Target_Pos_test);
  server_gordijn.on("/CurrentPosTest", get_current_pos_test);
  server_gordijn.on("/getTargetPosTest", get_target_pos_test);
  server_gordijn.on("/StateTest", get_state_test);
  server_gordijn.on("/Home", homeing);
  server_gordijn.on("/setTargetPos/", set_Target_Pos);
  server_gordijn.on("/CurrentPos", get_current_pos);
  server_gordijn.on("/getTargetPos", get_target_pos);
  server_gordijn.on("/State", get_state);

  server_gordijn.on("/info", handleinfo);

  server_gordijn.on("/reset", []() { // trigger manual reset
    server_gordijn.send(200, "text/html", "reset");
    resetESP(); 
  });

  server_gordijn.onNotFound(handleNotFound);

  server_gordijn.begin();

  server_mesh.on(F("/"), mesh_handleRoot);
  server_mesh.on("/setIP/", set_IP);
  server_mesh.on("/setPORT/", set_PORT);
  server_mesh.onNotFound(mesh_handleNotFound);
  server_mesh.begin();
}

void mesh_loop()
{
  server_gordijn.handleClient();
  server_mesh.handleClient();
  mesh.update();
  send_change();
}

void send_change()
{
  if (change == true)
  {
    LOG_DEBUG("value:", value);
    LOG_DEBUG("room_mac:", room_mac);
    LOG_ERROR(sendHttpRequest(value, room_mac, bridgeIp, bridgePort));
    change = false;
  }
}

void newConnectionCallback(uint32_t nodeId)
{

  JsonDocument doc;
  doc["master"] = uint32_t(mesh.getNodeId());
  String msg;
  msg.reserve(256);
  serializeJson(doc, msg);

  LOG_DEBUG("newConnection nodeId:", nodeId);
  LOG_DEBUG("newConnection msg:", msg);

  if (nodeId > 0)
  {
    mesh.sendSingle(nodeId, msg);
  }
  else
  {
    mesh.sendBroadcast(msg);
  }
}

void receivedCallback(uint32_t from, String &msg)
{
  infoLight(RgbColor(0, 255, 0)); // Green for mesh messages
  JsonDocument root;
  DeserializationError error = deserializeJson(root, msg);

  if (error)
  {
    LOG_ERROR("deserializeJson() failed:", error.c_str());
    return;
  }
  LOG_DEBUG("nodeId:", from);
  LOG_DEBUG("msg:", msg);
  if (bool(root["got_master"]) == true)
  {
    if (root["device"] == "switch")
    {
      room_mac = static_cast<const char *>(root["room_mac"]);
      value = (int)root["value"];
      change = true;
    }
    if (root["device"] == "curtain")
    {
      curtain_id = uint32_t(root["curtain_id"]);
      target_ont = (int)root["target"];
      current_ont = (int)root["current"];
      state_ont = (int)root["state"];
    }
  }
  else
  {
    if (root["device"] == "curtain")
    {
      curtain_id = uint32_t(root["curtain_id"]);
    }
    newConnectionCallback(from);
  }
}

void handleRoot()
{
  String message = "<!DOCTYPE HTML>";
  message += "<html>";
  message += "<h1 align=center>Curtain control over ethernet+mesh</h1><br><br>";
  message += "set Target Pos  = ";
  message += target;
  message += "<br>";
  message += "rep Target Pos  = ";
  message += target_ont;
  message += "<br>";
  message += "rep Current Pos  = ";
  message += current_ont;
  message += "<br>";
  message += "rep State  = ";
  message += state_ont;
  message += "<br>";
  message += "error  = ";
  message += fout;
  message += "<br><br>";

  message += "<form action=\"/setTargetPosTest/\">";
  message += "SET Target";
  message += "<input type=\"range\" name=\"Pos\" min=\"0\" max=\"100\" value=\"" + (String)target + "\" step=\"1\" class=\"slider\">";
  // message += "<input type=\"text\"  name=\"Pos\" value=\"" + (String)target + "\">";
  message += "<input type=\"submit\" value=\"Submit\">";
  message += "</form>";
  message += "Current Target  = ";
  message += target;

  // message += "<a href=\"//setTargetPosTEST\"\"><button>SET Target Pos TEST</button></a>"; //aanpassen voor invullen

  message += "<br><br>";

  message += "<a href=\"/CurrentPosTest\"\"><button>GET Current Pos TEST</button></a>";
  message += "<a href=\"/getTargetPosTest\"\"><button>GET Target Pos TEST</button></a>";
  message += "<a href=\"/StateTest\"\"><button>GET State TEST</button></a>";

  message += "<a href=\"/Home\"\"><button>Home curtain</button></a>";

  message += "<br><br>";
  message += "Links voor Homebridge";
  message += "<br><br>";

  message += "<a href=\"/setTargetPos/?Pos=50\"\"><button>SET Target Pos 50%</button></a>";
  message += "<a href=\"/CurrentPos\"\"><button>GET Current Pos</button></a>";
  message += "<a href=\"/getTargetPos\"\"><button>GET Target Pos</button></a>";
  message += "<a href=\"/State\"\"><button>GET State</button></a>";

  message += "<br><br>";
  message += "<a href=\"/info\"\"><button>Info</button></a>";
  message += "<a href=\"/\"\"><button>RELOAD PAGE</button></a><br/>";
  message += "<br><br>";
  message += "<a href=\"/reset\"\"><button>RESET</button></a><br/>";

  message += "</html>";
  server_gordijn.send(200, "text/html", message);
}

void sendData(String msg)
{
  if (curtain_id > 0)
  {
    mesh.sendSingle(curtain_id, msg);
  }
  else
  {
    mesh.sendBroadcast(msg);
  }
}

void set_Target_Pos_test()
{
  for (uint8_t i = 0; i < server_gordijn.args(); i++)
  {
    if (server_gordijn.argName(i) == F("Pos"))
    {
      target = server_gordijn.arg(i).toInt();
    }
  }
  JsonDocument doc;
  doc["device"] = "curtain";
  doc["homing"] = false;
  doc["request"] = true;
  doc["target"] = target;
  String msg;
  msg.reserve(256);
  serializeJson(doc, msg);
  sendData(msg);
  server_gordijn.sendHeader("Location", "/", true); // Redirect to our html web page
  server_gordijn.send(302, "text/plane", "");
}

void set_Target_Pos()
{
  for (uint8_t i = 0; i < server_gordijn.args(); i++)
  {
    if (server_gordijn.argName(i) == F("Pos"))
    {
      target = server_gordijn.arg(i).toInt();
    }
  }
  JsonDocument doc;
  doc["device"] = "curtain";
  doc["homing"] = false;
  doc["request"] = true;
  doc["target"] = target;
  String msg;
  msg.reserve(256);
  serializeJson(doc, msg);
  sendData(msg);
  server_gordijn.send(200, F("text/plain"), F("OK"));
}

void homeing()
{
  JsonDocument doc;
  doc["device"] = "curtain";
  doc["homing"] = true;
  doc["request"] = false;
  String msg;
  msg.reserve(256);
  serializeJson(doc, msg);
  sendData(msg);
  server_gordijn.sendHeader("Location", "/", true); // Redirect to our html web page
  server_gordijn.send(302, "text/plane", "");
}

void get_current_pos_test()
{
  JsonDocument doc;
  doc["device"] = "curtain";
  doc["homing"] = false;
  doc["request"] = true;
  String msg;
  msg.reserve(256);
  serializeJson(doc, msg);
  sendData(msg);
  server_gordijn.sendHeader("Location", "/", true); // Redirect to our html web page
  server_gordijn.send(302, "text/plane", "");
}

void get_current_pos()
{
  JsonDocument doc;
  doc["device"] = "curtain";
  doc["homing"] = false;
  doc["request"] = true;
  String msg;
  msg.reserve(256);
  serializeJson(doc, msg);
  sendData(msg);
  server_gordijn.send(200, F("text/plain"), (String)current_ont);
}

void get_target_pos_test()
{
  JsonDocument doc;
  doc["device"] = "curtain";
  doc["homing"] = false;
  doc["request"] = true;
  String msg;
  msg.reserve(256);
  serializeJson(doc, msg);
  sendData(msg);
  server_gordijn.sendHeader("Location", "/", true); // Redirect to our html web page
  server_gordijn.send(302, "text/plane", "");
}

void get_target_pos()
{
  JsonDocument doc;
  doc["device"] = "curtain";
  doc["homing"] = false;
  doc["request"] = true;
  String msg;
  msg.reserve(256);
  serializeJson(doc, msg);
  sendData(msg);
  server_gordijn.send(200, F("text/plain"), (String)target_ont);
}

void get_state_test()
{
  JsonDocument doc;
  doc["device"] = "curtain";
  doc["homing"] = false;
  doc["request"] = true;
  String msg;
  msg.reserve(256);
  serializeJson(doc, msg);
  sendData(msg);
  server_gordijn.sendHeader("Location", "/", true); // Redirect to our html web page
  server_gordijn.send(302, "text/plane", "");
}

void get_state()
{
  JsonDocument doc;
  doc["device"] = "curtain";
  doc["homing"] = false;
  doc["request"] = true;
  String msg;
  msg.reserve(256);
  serializeJson(doc, msg);
  sendData(msg);
  server_gordijn.send(200, F("text/plain"), (String)state_ont);
}

void handleinfo()
{

  String message = "<!DOCTYPE HTML>";
  message += "<html>";
  message += "info<br><br>";
  message += "IP: ";
  message += WiFi.localIP().toString();

  message += "<a href=\"/\"\"><button>HOME PAGE</button></a><br/>";
  server_gordijn.send(200, "text/html", message);
}

void handleNotFound()
{
  String message = "<!DOCTYPE HTML>";
  message = "File Not Found<br><br>";
  message += "URI: ";
  message += server_gordijn.uri();
  message += "<br>Method: ";
  message += (server_gordijn.method() == HTTP_GET) ? "GET" : "POST";
  message += "<br>Arguments: ";
  message += server_gordijn.args();
  message += "<br>";
  for (uint8_t i = 0; i < server_gordijn.args(); i++)
  {
    message += " " + server_gordijn.argName(i) + ": " + server_gordijn.arg(i) + "\n";
  }
  message += "<br><br>";
  message += "<a href=\"/\"\"><button>HOME PAGE</button></a><br/>";
  server_gordijn.send(404, "text/html", message);
}

void mesh_handleNotFound()
{
  String message = "<!DOCTYPE HTML>";
  message = "File Not Found<br><br>";
  message += "URI: ";
  message += server_mesh.uri();
  message += "<br>Method: ";
  message += (server_mesh.method() == HTTP_GET) ? "GET" : "POST";
  message += "<br>Arguments: ";
  message += server_mesh.args();
  message += "<br>";
  for (uint8_t i = 0; i < server_mesh.args(); i++)
  {
    message += " " + server_mesh.argName(i) + ": " + server_mesh.arg(i) + "\n";
  }
  message += "<br><br>";
  message += "<a href=\"/\"\"><button>HOME PAGE</button></a><br/>";
  server_mesh.send(404, "text/html", message);
}

void mesh_handleRoot()
{
  String message = "<!DOCTYPE HTML>";
  message += "<html>";
  message += "<h1 align=center>Set IP for mesh command</h1><br><br>";

  message += "<form action=\"/setIP/\">";
  message += "SET IP";
  message += "<input type=\"text\"  name=\"subip\" value=\"" + (String)subip + "\">";
  message += "<input type=\"submit\" value=\"Submit\">";
  message += "</form>";
  message += "Current IP = ";
  message += bridgeIp.toString();

  message += "<br><br>";
  message += "<form action=\"/setPORT/\">";
  message += "SEET PORT";
  message += "<input type=\"text\"  name=\"subport\" value=\"" + (String)bridgePort + "\">";
  message += "<input type=\"submit\" value=\"Submit\">";
  message += "</form>";
  message += "Current PORT = ";
  message += String(bridgePort);

  message += "<br><br>";
  message += "<a href=\"/\"\"><button>RELOAD PAGE</button></a><br/>";

  message += "</html>";
  server_mesh.send(200, "text/html", message);
}

void set_IP()
{
  for (uint8_t i = 0; i < server_mesh.args(); i++)
  {
    if (server_mesh.argName(i) == F("subip"))
    {
      subip = server_mesh.arg(i).toInt();
    }
  }
  {
    IPAddress base;
    base.fromString(String(BRIDGE_IP));
    base[3] = subip;
    bridgeIp = base;
  }
  saveMeshConfig();
  server_mesh.sendHeader("Location", "/", true); // Redirect to our html web page
  server_mesh.send(302, "text/plane", "");
}

void set_PORT()
{
  for (uint8_t i = 0; i < server_mesh.args(); i++)
  {
    if (server_mesh.argName(i) == F("subport"))
    {
      bridgePort = server_mesh.arg(i).toInt();
    }
  }
  saveMeshConfig();
  server_mesh.sendHeader("Location", "/", true); // Redirect to our html web page
  server_mesh.send(302, "text/plane", "");
}
