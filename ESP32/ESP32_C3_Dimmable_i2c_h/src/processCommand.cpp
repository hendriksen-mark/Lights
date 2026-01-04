#include "processCommand.h"

String sendHttpRequest(int button, String mac, IPAddress bridgeIp, int bridgePort)
{
  HTTPClient http;
  String response = "";
  bool isRegistered = false;
  int retryCount = 0;

  // Build URL
  String url = "http://" + bridgeIp.toString() + ":" + String(bridgePort) + "/switch";

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
      return response;
    }
    else if (response == "unknown device" || response == "missing mac address" || response == "invalid json")
    {
      // Permanent errors
      infoLedError();
      return response;
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
            response = responseDoc["success"].as<String>();
          }
          else if (responseDoc["fail"].is<String>())
          {
            response = responseDoc["fail"].as<String>();
          }
          else
          {
            response = "unknown response format";
          }

          // If we got a valid response, reset retry counter for next step
          retryCount = 0;
        }
        else
        {
          REMOTE_LOG_ERROR("JSON parse error:", error.c_str());
          response = "json parse error";
          retryCount++;
        }
      }
      else
      {
        REMOTE_LOG_ERROR("HTTP error:", httpCode);
        response = "HTTP error: " + String(httpCode);
        retryCount++;
      }
    }
    else
    {
      REMOTE_LOG_ERROR("Connection failed:", http.errorToString(httpCode));
      infoLedError();
      response = "Connection failed";
      retryCount++;
      delay(RETRY_DELAY_MS);
    }

    http.end();

    // Avoid infinite loop - break if too many retries
    if (retryCount >= MAX_RETRIES)
    {
      infoLedError();
      return "Max retries exceeded";
    }
  }

  return response;
}
