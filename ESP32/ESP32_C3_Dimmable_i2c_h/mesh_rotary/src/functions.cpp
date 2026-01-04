#if defined(VSCODE)
#include "functions.h"
#else
#include "include/functions.h"
#endif

void receivedCallback(uint32_t from, String &msg)
{
  JsonDocument root;
  DeserializationError error = deserializeJson(root, msg);

  if (error)
  {
    // Silently ignore malformed messages
    return;
  }

  // Update master if received
  if (root["master"].is<uint32_t>())
  {
    uint32_t newMaster = root["master"];
    if (newMaster != master)
    {
      master = newMaster;
      // Reset state when master changes
      change = false;
    }
  }
}

void ask_master()
{
  if ((unsigned long)(millis() - MasterPreviousMillis) >= REQUEST_TIMEOUT)
  {
    MasterPreviousMillis = millis();
    JsonDocument doc;
    doc["got_master"] = false;
    doc["device"] = "switch";
    doc["switch_id"] = mesh.getNodeId();

    char buf[JSON_BUFFER_SIZE];
    size_t len = serializeJson(doc, buf, sizeof(buf));

    if (len > 0 && len < sizeof(buf))
    {
      mesh.sendBroadcast(buf);
    }
  }
}

void send_change()
{
  // Check if we have a master
  if (master == 0)
  {
    ask_master();
    return;
  }

  // Only send if there's an actual change
  if (!change)
  {
    return;
  }

  // Motion debouncing for motion detectors
  if (isMotionDetector((RoomType)room))
  {
    unsigned long currentMillis = millis();
    if (currentMillis - lastMotionMillis < MOTION_DEBOUNCE_TIME)
    {
      return; // Ignore rapid motion triggers
    }
    lastMotionMillis = currentMillis;
  }

  JsonDocument doc;
  doc["got_master"] = true;
  doc["device"] = "switch";
  doc["room_mac"] = macToStr(mac0[room]);

  // For motion sensors, include explicit motion flag
  if (isMotionDetector((RoomType)room))
  {
    doc["motion"] = (value == MOTION_DETECTED);
  }
  doc["value"] = value;

  char buf[JSON_BUFFER_SIZE];
  size_t len = serializeJson(doc, buf, sizeof(buf));

  if (len > 0 && len < sizeof(buf))
  {
    mesh.sendSingle(master, buf);
    change = false;
  }
}

String macToStr(const uint8_t *mac)
{
  String result;
  result.reserve(17); // Pre-allocate: "XX:XX:XX:XX:XX:XX"

  for (size_t i = 0; i < MAC_BYTES; ++i)
  {
    if (mac[i] < 0x10)
      result += '0';
    result += String(mac[i], HEX);
    if (i < MAC_BYTES - 1)
      result += ':';
  }
  result.toUpperCase();
  return result;
}
