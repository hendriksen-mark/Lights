#include "processCommand.h"

WiFiClient client;

String sendHttpRequest(int button, String mac, IPAddress bridgeIp)
{
  String msg;
  msg.reserve(50); // Pre-allocate to reduce memory fragmentation
  msg = "";

  while (true)
  {
    if (!client.connect(bridgeIp, BRIDGE_PORT))
    {
      // LOG_ERROR("Connection failed");
      infoLedError(); // Show connection error
      delay(100);
      return "Connection failed";
    }
    LOG_INFO("Connected!");
    LOG_DEBUG("msg:", msg);
    String url;
    url.reserve(100); // Pre-allocate to reduce memory fragmentation
    url = "/switch";
    // url += "?mac=";
    // url += mac;

    if (msg == "device not found" || msg == "no mac in list")
    {
      // url = "/switch";
      url += "?devicetype=";
      if (button >= 1000)
      {
        url += (String)switchType;
      }
      else
      {
        url += (String)motionType;
      }
      url += "&mac=";
      url += mac;
    }
    else if (msg == "command applied")
    {
      client.stop();
      blinkLed(1, 50); // Quick success blink
      return msg;
    }
    else if (msg == "unknown device" || msg == "missing mac address")
    {
      client.stop();
      infoLedError(); // Show error for unknown device
      delay(100);
      return msg;
    }
    else
    {
      int batteryPercent = 100;
      // url = "/switch?mac=";
      url += "?mac=";
      url += mac;
      url += "&button=";
      url += button;
      url += "&presence=true";
      url += "&battery=";
      url += batteryPercent;
    }
    LOG_DEBUG("url:", url);

    String message;
    message.reserve(150); // Pre-allocate to reduce memory fragmentation
    message = String("GET ");
    message += url;
    message += " HTTP/1.1\r\n";
    message += "Host: ";
    message += bridgeIp;
    message += "\r\n";
    message += "Connection: close\r\n\r\n";

    client.println(message);

    if (client.println() == 0)
    {
      // LOG_ERROR("Failed to send request");
      client.stop();
      return "Failed to send request";
    }

    // Check HTTP status
    char status[32] = {0};
    client.readBytesUntil('\r', status, sizeof(status));
    if (strcmp(status, "HTTP/1.1 200 OK") != 0)
    {
      // LOG_ERROR("Unexpected response:", status);
      client.stop();
      return "Unexpected response: ";
    }

    // Skip HTTP headers
    char endOfHeaders[] = "\r\n\r\n";
    if (!client.find(endOfHeaders))
    {
      // LOG_ERROR("Invalid response");
      client.stop();
      return "Invalid response";
    }

    // Allocate the JSON document
    JsonDocument doc;

    // Parse JSON object
    DeserializationError error = deserializeJson(doc, client);
    if (error)
    {
      // LOG_ERROR("deserializeJson() failed:", error.c_str());
      client.stop();
      return "deserializeJson() failed: ";
    }
    JsonObject obj = doc.as<JsonObject>();
    if (!!obj["success"].isNull())
    {
      msg = doc["success"].as<String>();
    }
    if (!!obj["fail"].isNull())
    {
      msg = doc["fail"].as<String>();
    }
  }
  client.stop();
  return msg;
}
