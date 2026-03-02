#include "processCommand.h"

void sendHttpRequest(int button, String mac, IPAddress bridgeIp, int bridgePort)
{
  HTTPClient http;
  String response = "";
  bool isRegistered = false;
  int retryCount = 0;

  // Build URL
  String url = "http://" + bridgeIp.toString() + ":" + String(bridgePort) + "/switch";

  REMOTE_LOG_DEBUG("value:", button);
  REMOTE_LOG_DEBUG("room_mac:", mac);
  REMOTE_LOG_DEBUG("URL:", url);

  while (retryCount < MAX_RETRIES)
  {
    JsonDocument doc;
    doc["mac"] = mac;

    // First attempt or device not found - try registration
    if (!isRegistered && (response == "" || response == "device not found"))
    {
      doc["devicetype"] = (button >= 1000) ? switchType : motionType;
      REMOTE_LOG_INFO("Registering device...");
    }
    else if (response == "device registered" || response == "device already registered")
    {
      // Device is registered, now send update
      isRegistered = true;
      doc.remove("devicetype");
      doc["button"] = button;
      doc["presence"] = true;
      doc["battery"] = 100;
      REMOTE_LOG_INFO("Updating device...");
    }
    else if (response == "command applied")
    {
      // Success!
      blinkLed(1, 50);
      REMOTE_LOG_DEBUG("Response:", response);
      return;
    }
    else if (response == "unknown device" || response == "missing mac address" || response == "invalid json")
    {
      // Permanent errors
      infoLedError();
      REMOTE_LOG_ERROR("Response:", response);
      return;
    }

    // Serialize JSON
    String payload;
    serializeJson(doc, payload);
    REMOTE_LOG_DEBUG("Payload:", payload);

    // Make HTTP POST request
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(5000); // 5 second timeout

    int httpCode = http.POST(payload);

    if (httpCode > 0)
    {
      if (httpCode == HTTP_CODE_OK)
      {
        String responseBody = http.getString();
        REMOTE_LOG_DEBUG("Response:", responseBody);

        // Parse response JSON
        JsonDocument responseDoc;
        DeserializationError error = deserializeJson(responseDoc, responseBody);

        if (!error)
        {
          if (responseDoc["success"].is<String>())
          {
            REMOTE_LOG_DEBUG("Response success:", responseDoc["success"].as<String>());
            response = responseDoc["success"].as<String>();
          }
          else if (responseDoc["fail"].is<String>())
          {
            REMOTE_LOG_ERROR("Response fail:", responseDoc["fail"].as<String>());
            response = responseDoc["fail"].as<String>();
          }
          else
          {
            REMOTE_LOG_ERROR("Response unknown:", responseDoc.as<String>());
            response = "unknown response format";
          }

          // If we got a valid response, reset retry counter for next step
          retryCount = 0;
        }
        else
        {
          REMOTE_LOG_ERROR("JSON parse error:", error.c_str());
          retryCount++;
        }
      }
      else
      {
        REMOTE_LOG_ERROR("HTTP error:", httpCode);     
        retryCount++;
      }
    }
    else
    {
      REMOTE_LOG_ERROR("Connection failed:", http.errorToString(httpCode));
      infoLedError();
      retryCount++;
      delay(RETRY_DELAY_MS);
    }

    http.end();

    // Avoid infinite loop - break if too many retries
    if (retryCount >= MAX_RETRIES)
    {
      infoLedError();
      REMOTE_LOG_ERROR("Max retries exceeded");
      return;
    }
  }

  return;
}
