#include "mesh.h"
#include "functions.h"

int subip = 0;
IPAddress bridgeIp;
int bridgePort = BRIDGE_PORT;

painlessMesh mesh;

// OTA Management
String otaFirmwarePath = "";
String otaDeviceName = "";
String otaHardware = "";
File otaFile;
std::shared_ptr<Task> otaTask = nullptr;
bool otaActive = false;

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

unsigned long lastCurtainPoll = 0;

WebServer server_gordijn(CURTAIN_SERVER_PORT);

bool loadConfig_mesh()
{
  REMOTE_LOG_DEBUG("load mesh config");
  JsonDocument doc;
  if (!readJsonFile(MESH_CONFIG_PATH, doc))
  {
    REMOTE_LOG_INFO("Create new file with default values");
    return saveConfig_mesh();
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
    }
  }
  return true;
}

bool saveConfig_mesh()
{
  REMOTE_LOG_DEBUG("save mesh config");
  JsonDocument doc;
  doc["subip"] = subip;
  doc["bridge"] = bridgePort;
  return writeJsonFile(MESH_CONFIG_PATH, doc);
}

// List all .bin files on SD card in /firmware directory
String listFirmwareFiles()
{
  String files = "";
  if (!fs_exists("/firmware"))
  {
    return "No /firmware directory found";
  }

  File dir = fs_open("/firmware", "r");
  if (!dir || !dir.isDirectory())
  {
    return "Failed to open /firmware directory";
  }

  while (true)
  {
    File entry = dir.openNextFile();
    if (!entry)
      break;

    if (!entry.isDirectory())
    {
      String name = entry.name();
      if (name.endsWith(".bin"))
      {
        files += name + " (" + String(entry.size()) + " bytes)<br>";
      }
    }
    entry.close();
  }
  dir.close();

  if (files.length() == 0)
  {
    return "No .bin files found in /firmware";
  }
  return files;
}

// Parse firmware filename: firmware_<hardware>_<role>.bin
// Returns true if valid, fills hardware and role
bool parseFirmwareName(const String &filename, String &hardware, String &role)
{
  // Remove path if present
  String name = filename;
  int lastSlash = name.lastIndexOf('/');
  if (lastSlash >= 0)
  {
    name = name.substring(lastSlash + 1);
  }

  // Check format: firmware_<hardware>_<role>.bin
  if (!name.startsWith("firmware_") || !name.endsWith(".bin"))
  {
    return false;
  }

  int firstUnderscore = name.indexOf('_');
  int secondUnderscore = name.indexOf('_', firstUnderscore + 1);
  int dotPos = name.lastIndexOf('.');

  if (firstUnderscore < 0 || secondUnderscore < 0 || dotPos < 0)
  {
    return false;
  }

  hardware = name.substring(firstUnderscore + 1, secondUnderscore);
  role = name.substring(secondUnderscore + 1, dotPos);

  // Validate hardware
  if (hardware != "ESP8266" && hardware != "ESP32")
  {
    return false;
  }

  return true;
}

// Start OTA update broadcast
void startOTA(const String &firmwarePath)
{
  if (otaActive && otaTask)
  {
    REMOTE_LOG_ERROR("OTA already active, stopping previous OTA");
    otaTask->disable();
    otaTask = nullptr;
    if (otaFile)
      otaFile.close();
    otaActive = false;
  }

  String fullPath = "/firmware/" + firmwarePath;
  if (!fs_exists(fullPath.c_str()))
  {
    REMOTE_LOG_ERROR("Firmware file not found:", fullPath);
    return;
  }

  String hardware, role;
  if (!parseFirmwareName(firmwarePath, hardware, role))
  {
    REMOTE_LOG_ERROR("Invalid firmware filename format:", firmwarePath);
    return;
  }

  otaFile = fs_open(fullPath.c_str(), FILE_READ);
  if (!otaFile)
  {
    REMOTE_LOG_ERROR("Failed to open firmware file:", fullPath);
    return;
  }

  REMOTE_LOG_INFO("Starting OTA for device:", role, "hardware:", hardware, "size:", otaFile.size());

  // Calculate MD5
  MD5Builder md5;
  md5.begin();
  md5.addStream(otaFile, otaFile.size());
  md5.calculate();
  String md5hash = md5.toString();

  REMOTE_LOG_INFO("Firmware MD5:", md5hash);

  // Initialize OTA send with lambda to read data
  mesh.initOTASend(
      [](painlessmesh::plugin::ota::DataRequest pkg, char *buffer)
      {
        otaFile.seek(OTA_PART_SIZE * pkg.partNo);
        size_t bytesRead = otaFile.readBytes(buffer, OTA_PART_SIZE);
        return min((unsigned)OTA_PART_SIZE, (unsigned)(otaFile.size() - (OTA_PART_SIZE * pkg.partNo)));
      },
      OTA_PART_SIZE);

  // Offer OTA to network
  size_t numParts = (otaFile.size() + OTA_PART_SIZE - 1) / OTA_PART_SIZE;
  otaTask = mesh.offerOTA(role, hardware, md5hash, numParts, false);

  otaFirmwarePath = firmwarePath;
  otaDeviceName = role;
  otaHardware = hardware;
  otaActive = true;

  REMOTE_LOG_INFO("OTA broadcast started. Will send for 60 minutes.");
}

// Stop OTA broadcast
void stopOTA()
{
  if (otaActive && otaTask)
  {
    otaTask->disable();
    otaTask = nullptr;
    if (otaFile)
      otaFile.close();
    otaActive = false;
    REMOTE_LOG_INFO("OTA broadcast stopped");
  }
}

void mesh_setup()
{
  REMOTE_LOG_DEBUG("Setup Mesh");
  infoLight(magenta);
  // mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE | DEBUG ); // all types on
  // mesh.setDebugMsgTypes( ERROR | CONNECTION | SYNC | S_TIME );  // set before init() so that you can see startup messages
  // mesh.setDebugMsgTypes( ERROR | CONNECTION | S_TIME );  // set before init() so that you can see startup messages
  mesh.setDebugMsgTypes(ERROR | MESH_STATUS | CONNECTION | COMMUNICATION);
  // mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT);
  mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT, MESH_CONNECT_MODE, MESH_HIDDEN);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  newConnectionCallback(0);

  discoverBridgeMdns();
  if (loadConfig_mesh())
  {
    REMOTE_LOG_DEBUG("mesh config loaded");
  }
  else
  {
    REMOTE_LOG_DEBUG("mesh config load failed, using defaults");
  }

  server_gordijn.on("/", handleRoot);
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
  server_gordijn.on("/setIP/", set_IP);
  server_gordijn.on("/setPORT/", set_PORT);
  server_gordijn.on("/discover/", []()
                    {
    discoverBridgeMdns();
    server_gordijn.sendHeader("Location", "/", true); // Redirect to our html web page
    server_gordijn.send(302, "text/plane", ""); });
  server_gordijn.on("/reset", []() { // trigger manual reset
    server_gordijn.send(200, "text/html", "reset");
    resetESP();
  });

  // OTA endpoints
  server_gordijn.on("/ota", handleOTAPage);
  server_gordijn.on("/ota/list", handleOTAList);
  server_gordijn.on("/ota/start", handleOTAStart);
  server_gordijn.on("/ota/stop", handleOTAStop);
  server_gordijn.on("/ota/status", handleOTAStatus);
  server_gordijn.on("/ota/upload", HTTP_POST, handleOTAUploadResponse, handleOTAUpload);

  server_gordijn.onNotFound(handleNotFound);

  server_gordijn.begin();
}

void mesh_loop()
{
  server_gordijn.handleClient();
  mesh.update();
  send_change();
  pollCurtainStatus();
}

void send_change()
{
  if (change == true)
  {
    REMOTE_LOG_DEBUG("value:", value);
    REMOTE_LOG_DEBUG("room_mac:", room_mac);
    REMOTE_LOG_ERROR(sendHttpRequest(value, room_mac, bridgeIp, bridgePort));
    change = false;
  }
}

void pollCurtainStatus()
{
  unsigned long currentMillis = millis();

  // Check if it's time to poll
  if (currentMillis - lastCurtainPoll >= CURTAIN_POLL_INTERVAL)
  {
    lastCurtainPoll = currentMillis;

    // Only poll if we have a curtain connected
    if (curtain_id > 0)
    {
      JsonDocument doc;
      doc["device"] = "curtain";
      doc["homing"] = false;
      doc["request"] = true;
      String msg;
      msg.reserve(256);
      serializeJson(doc, msg);
      sendData(msg);
      REMOTE_LOG_DEBUG("Polling curtain status, ID:", curtain_id);
    }
  }
}

void newConnectionCallback(uint32_t nodeId)
{

  JsonDocument doc;
  doc["master"] = uint32_t(mesh.getNodeId());
  String msg;
  msg.reserve(256);
  serializeJson(doc, msg);

  REMOTE_LOG_DEBUG("new nodeId:", nodeId);
  REMOTE_LOG_DEBUG("reply msg:", msg);

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
  infoLight(green); // Green for mesh messages
  JsonDocument root;
  DeserializationError error = deserializeJson(root, msg);

  if (error)
  {
    REMOTE_LOG_ERROR("deserializeJson() failed:", error.c_str());
    return;
  }
  REMOTE_LOG_DEBUG("from nodeId:", from);
  REMOTE_LOG_DEBUG("got msg:", msg);
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

// Helper function to create and send curtain command
void sendCurtainCommand(bool homing, bool request, int targetPos = -1, const char *logPath = "")
{
  JsonDocument doc;
  doc["device"] = "curtain";
  doc["homing"] = homing;
  doc["request"] = request;
  if (targetPos >= 0)
  {
    doc["target"] = targetPos;
  }
  String msg;
  msg.reserve(256);
  serializeJson(doc, msg);
  sendData(msg);
  REMOTE_LOG_DEBUG("from:", server_gordijn.client().remoteIP().toString(), logPath, msg);
}

// Helper to send response: redirect or plain text
void sendResponse(bool isRedirect, const String &content = "")
{
  if (isRedirect)
  {
    server_gordijn.sendHeader("Location", "/", true);
    server_gordijn.send(302, "text/plane", "");
  }
  else
  {
    server_gordijn.send(200, "text/plain", content);
  }
}

void set_Target_Pos_test()
{
  for (uint8_t i = 0; i < server_gordijn.args(); i++)
  {
    if (server_gordijn.argName(i) == "Pos")
    {
      target = server_gordijn.arg(i).toInt();
    }
  }
  sendCurtainCommand(false, true, target, "/setTargetPosTest");
  sendResponse(true); // Redirect
}

void set_Target_Pos()
{
  for (uint8_t i = 0; i < server_gordijn.args(); i++)
  {
    if (server_gordijn.argName(i) == "Pos")
    {
      target = server_gordijn.arg(i).toInt();
    }
  }
  sendCurtainCommand(false, true, target, "/setTargetPos");
  sendResponse(false, "OK");
}

void homeing()
{
  sendCurtainCommand(true, false, -1, "/homeing");
  sendResponse(true); // Redirect
}

void get_current_pos_test()
{
  sendCurtainCommand(false, true, -1, "/getCurrentPosTest");
  sendResponse(true); // Redirect
}

void get_current_pos()
{
  // NOTE: This endpoint returns the last cached value from current_ont.
  // The request is sent to the curtain device, but due to HTTP's synchronous nature,
  // we cannot wait for the mesh response. The actual updated value will be received
  // in receivedCallback() and cached in current_ont for the next request.
  // For real-time accuracy, poll this endpoint repeatedly or implement WebSocket/async pattern.
  sendCurtainCommand(false, true, -1, "/getCurrentPos");
  sendResponse(false, (String)current_ont);
}

void get_target_pos_test()
{
  sendCurtainCommand(false, true, -1, "/getTargetPosTest");
  sendResponse(true); // Redirect
}

void get_target_pos()
{
  // NOTE: Returns cached target_ont value. See get_current_pos() comment for details.
  sendCurtainCommand(false, true, -1, "/getTargetPos");
  sendResponse(false, (String)target_ont);
}

void get_state_test()
{
  sendCurtainCommand(false, true, -1, "/getStateTest");
  sendResponse(true); // Redirect
}

void get_state()
{
  // NOTE: Returns cached state_ont value (0=stopped, 1=opening, 2=closing).
  // See get_current_pos() comment for async limitations.
  sendCurtainCommand(false, true, -1, "/getState");
  sendResponse(false, (String)state_ont);
}

void handleinfo()
{

  String message = "<!DOCTYPE HTML>";
  message += "<html>";
  message += "info<br><br>";
  message += "IP: ";
  message += WiFi.localIP().toString();

  message += "<a href=\"/\"\"><button>HOME PAGE</button></a><br/>";
  message += "</html>";
  REMOTE_LOG_DEBUG("from:", server_gordijn.client().remoteIP().toString(), "/info");
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
  REMOTE_LOG_DEBUG("from:", server_gordijn.client().remoteIP().toString(), "not found:", server_gordijn.uri(), "args:", server_gordijn.args());
  server_gordijn.send(404, "text/html", message);
}

void set_IP()
{
  for (uint8_t i = 0; i < server_gordijn.args(); i++)
  {
    if (server_gordijn.argName(i) == "subip")
    {
      subip = server_gordijn.arg(i).toInt();
    }
  }
  {
    IPAddress base;
    base.fromString(String(BRIDGE_IP));
    base[3] = subip;
    bridgeIp = base;
  }
  saveConfig_mesh();
  REMOTE_LOG_DEBUG("from:", server_gordijn.client().remoteIP().toString(), "/setIP", bridgeIp.toString());
  server_gordijn.sendHeader("Location", "/", true); // Redirect to our html web page
  server_gordijn.send(302, "text/plane", "");
}

void set_PORT()
{
  for (uint8_t i = 0; i < server_gordijn.args(); i++)
  {
    if (server_gordijn.argName(i) == "subport")
    {
      bridgePort = server_gordijn.arg(i).toInt();
    }
  }
  saveConfig_mesh();
  REMOTE_LOG_DEBUG("from:", server_gordijn.client().remoteIP().toString(), "/setPORT", String(bridgePort));
  server_gordijn.sendHeader("Location", "/", true); // Redirect to our html web page
  server_gordijn.send(302, "text/plane", "");
}

// OTA Web Handlers
void handleOTAPage()
{
  String html = "<!DOCTYPE HTML><html><head><title>OTA Update</title></head><body>";
  html += "<h1>Mesh OTA Update</h1>";
  html += "<h2>Current OTA Status</h2>";
  if (otaActive)
  {
    html += "<p style='color:green;'><b>OTA Active</b></p>";
    html += "<p>Firmware: " + otaFirmwarePath + "</p>";
    html += "<p>Device: " + otaDeviceName + "</p>";
    html += "<p>Hardware: " + otaHardware + "</p>";
    html += "<a href='/ota/stop'><button>Stop OTA</button></a>";
  }
  else
  {
    html += "<p style='color:gray;'>No OTA active</p>";
  }

  html += "<h2>Available Firmware</h2>";
  html += "<p>" + listFirmwareFiles() + "</p>";

  html += "<h2>Start OTA Update</h2>";
  html += "<p>Firmware naming: <code>firmware_&lt;ESP32|ESP8266&gt;_&lt;devicename&gt;.bin</code></p>";
  html += "<p>Examples:</p>";
  html += "<ul>";
  html += "<li><code>firmware_ESP32_curtain.bin</code> - for beetle_curtain devices</li>";
  html += "<li><code>firmware_ESP32_keuken.bin</code> - for mesh_rotary in kitchen</li>";
  html += "<li><code>firmware_ESP32_slaapkamer.bin</code> - for mesh_rotary in bedroom</li>";
  html += "</ul>";

  html += "<form action='/ota/start' method='GET'>";
  html += "Firmware filename: <input type='text' name='file' placeholder='firmware_ESP32_curtain.bin' size='40'><br><br>";
  html += "<input type='submit' value='Start OTA Broadcast'>";
  html += "</form>";

  html += "<h2>Upload New Firmware</h2>";
  html += "<form method='POST' action='/ota/upload' enctype='multipart/form-data'>";
  html += "<input type='file' name='firmware' accept='.bin'><br><br>";
  html += "<input type='submit' value='Upload'>";
  html += "</form>";

  html += "<br><br><a href='/'><button>Back to Main</button></a>";
  html += "</body></html>";

  REMOTE_LOG_DEBUG("from:", server_gordijn.client().remoteIP().toString(), "/ota");
  server_gordijn.send(200, "text/html", html);
}

void handleOTAList()
{
  String files = listFirmwareFiles();
  REMOTE_LOG_DEBUG("from:", server_gordijn.client().remoteIP().toString(), "/ota/list");
  server_gordijn.send(200, "text/plain", files);
}

void handleOTAStart()
{
  if (!server_gordijn.hasArg("file"))
  {
    server_gordijn.send(400, "text/plain", "Missing 'file' parameter");
    return;
  }

  String filename = server_gordijn.arg("file");
  startOTA(filename);

  REMOTE_LOG_DEBUG("from:", server_gordijn.client().remoteIP().toString(), "/ota/start", filename);
  server_gordijn.sendHeader("Location", "/ota", true);
  server_gordijn.send(302, "text/plain", "");
}

void handleOTAStop()
{
  stopOTA();
  REMOTE_LOG_DEBUG("from:", server_gordijn.client().remoteIP().toString(), "/ota/stop");
  server_gordijn.sendHeader("Location", "/ota", true);
  server_gordijn.send(302, "text/plain", "");
}

void handleOTAStatus()
{
  String status = "{";
  status += "\"active\":" + String(otaActive ? "true" : "false") + ",";
  status += "\"firmware\":\"" + otaFirmwarePath + "\",";
  status += "\"device\":\"" + otaDeviceName + "\",";
  status += "\"hardware\":\"" + otaHardware + "\"";
  status += "}";

  REMOTE_LOG_DEBUG("from:", server_gordijn.client().remoteIP().toString(), "/ota/status");
  server_gordijn.send(200, "application/json", status);
}

void handleOTAUpload()
{
  HTTPUpload &upload = server_gordijn.upload();

  if (upload.status == UPLOAD_FILE_START)
  {
    String filename = upload.filename;
    if (!filename.endsWith(".bin"))
    {
      REMOTE_LOG_ERROR("Invalid file type, must be .bin");
      return;
    }

    // Validate firmware name format
    String hardware, role;
    if (!parseFirmwareName(filename, hardware, role))
    {
      REMOTE_LOG_ERROR("Invalid firmware filename format:", filename);
      return;
    }

    // Create /firmware directory if it doesn't exist
    if (!fs_exists("/firmware"))
    {
      fs_mkdir("/firmware");
    }

    String path = "/firmware/" + filename;
    REMOTE_LOG_INFO("Uploading firmware:", path);

    otaFile = fs_open(path.c_str(), FILE_WRITE);
    if (!otaFile)
    {
      REMOTE_LOG_ERROR("Failed to create file:", path);
    }
  }
  else if (upload.status == UPLOAD_FILE_WRITE)
  {
    if (otaFile)
    {
      otaFile.write(upload.buf, upload.currentSize);
    }
  }
  else if (upload.status == UPLOAD_FILE_END)
  {
    if (otaFile)
    {
      otaFile.close();
      REMOTE_LOG_INFO("Upload complete:", upload.totalSize, "bytes");
    }
  }
}

void handleOTAUploadResponse()
{
  REMOTE_LOG_DEBUG("from:", server_gordijn.client().remoteIP().toString(), "/ota/upload");
  server_gordijn.sendHeader("Location", "/ota", true);
  server_gordijn.send(302, "text/plain", "Upload complete, redirecting...");
}

void discoverBridgeMdns()
{
  REMOTE_LOG_DEBUG("Starting mDNS query for diyhue (_hue._tcp.local.) on Ethernet");

  // mDNS responder should be started on Ethernet in ESP_Server_setup();
  // perform a query directly — this will use the active mDNS responder/interface.

  // Query mDNS for _hue._tcp.local. with a short timeout (ms)
  int n = MDNS.queryService("hue", "tcp");
  if (n <= 0)
  {
    REMOTE_LOG_DEBUG("diyhue not found via mDNS, keeping configured bridge IP");
    return;
  }

  IPAddress firstIp(0, 0, 0, 0);
  int firstPort = 0;

  for (int i = 0; i < n; i++)
  {
    IPAddress ip = MDNS.IP(i);
    if (ip[0] == 0 && ip[1] == 0 && ip[2] == 0 && ip[3] == 0)
      continue;

    // Try to detect diyhue by hostname containing 'diyhue'
    String hostName;
#ifdef MDNS
    // many ESPmDNS implementations provide hostname(i)
    hostName = String(MDNS.hostname(i));
#endif

    if (hostName.length() > 0)
    {
      hostName.toLowerCase();
      if (hostName.indexOf("diyhue") >= 0)
      {
        bridgeIp = ip;
        int p = MDNS.port(i);
        if (p > 0)
          bridgePort = p;
        REMOTE_LOG_DEBUG("Found diyhue via mDNS (hostname match):", hostName, bridgeIp.toString(), "port:", bridgePort);
        return;
      }
    }

    // otherwise store first valid candidate as fallback
    if (firstIp[0] == 0 && firstIp[1] == 0 && firstIp[2] == 0 && firstIp[3] == 0)
    {
      firstIp = ip;
      firstPort = MDNS.port(i);
    }
  }

  // If we didn't find an explicit 'diyhue' hostname, use the first candidate
  if (firstIp[0] != 0 || firstIp[1] != 0 || firstIp[2] != 0 || firstIp[3] != 0)
  {
    bridgeIp = firstIp;
    if (firstPort > 0)
      bridgePort = firstPort;
    REMOTE_LOG_DEBUG("Using first mDNS candidate for bridge:", bridgeIp.toString(), "port:", bridgePort);
  }
}
