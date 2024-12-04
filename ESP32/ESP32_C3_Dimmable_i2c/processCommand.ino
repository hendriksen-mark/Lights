WiFiClient client;

//const char* bridgeIp = "192.168.1.25";
//IPAddress bridgeIp(192, 168, 1, 25);
//const char* switchType = "ZLLSwitch";
#define switchType "ZLLSwitch"
#define motionType "ZLLPresence"

String sendHttpRequest(int button, String mac, IPAddress bridgeIp) {
  String msg = "";
  int val = true;

  while (val) {
    if (!client.connect(bridgeIp, 80)) {
      //LOG_ERROR("Connection failed");
      return "Connection failed";
    }
    LOG_INFO("Connected!");
    LOG_DEBUG("msg:", msg);
    String url = "/switch";
    //url += "?mac=";
    //url += mac;

    if (msg == "device not found" || msg == "no mac in list") {
      //url = "/switch";
      url += "?devicetype=";
      if (button >= 1000) {
        url += (String)switchType;
      } else {
        url += (String)motionType;
      }
      url += "&mac=";
      url += mac;
    } else if (msg == "command applied") {
      client.stop();
      val = false;
      return msg;
    } else if (msg == "unknown device" || msg == "missing mac address") {
      client.stop();
      val = false;
      return msg;
    } else {
      int batteryPercent = 100;
      //url = "/switch?mac=";
      url += "?mac=";
      url += mac;
      url += "&button=";
      url += button;
      url += "&presence=true";
      url += "&battery=";
      url += batteryPercent;
    }
    LOG_DEBUG("url:", url);

    String message = String("GET ");
    message += url;
    message += " HTTP/1.1\r\n";
    message += "Host: ";
    message += bridgeIp;
    message += "\r\n";
    message += "Connection: close\r\n\r\n";

    client.println(message);


    if (client.println() == 0) {
      //LOG_ERROR("Failed to send request");
      client.stop();
      return "Failed to send request";
    }

    // Check HTTP status
    char status[32] = {0};
    client.readBytesUntil('\r', status, sizeof(status));
    if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
      //LOG_ERROR("Unexpected response:", status);
      client.stop();
      return "Unexpected response: ";
    }

    // Skip HTTP headers
    char endOfHeaders[] = "\r\n\r\n";
    if (!client.find(endOfHeaders)) {
      //LOG_ERROR("Invalid response");
      client.stop();
      return "Invalid response";
    }

    // Allocate the JSON document
    // Use arduinojson.org/v6/assistant to compute the capacity.
    const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
    DynamicJsonDocument doc(capacity);

    // Parse JSON object
    DeserializationError error = deserializeJson(doc, client);
    if (error) {
      //LOG_ERROR("deserializeJson() failed:", error.f_str());
      client.stop();
      return "deserializeJson() failed: ";
    }
    JsonObject obj = doc.as<JsonObject>();
    if (obj.containsKey("success")) {
      msg = doc["success"].as<String>();
    }
    if (obj.containsKey("fail")) {
      msg = doc["fail"].as<String>();
    }
  }
  client.stop();
  return msg;
}
