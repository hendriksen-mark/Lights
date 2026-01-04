#if defined(VSCODE)
#include "functions.h"
#else
#include "include/functions.h"
#endif

painlessMesh mesh;
uint32_t master = 0;

ESPRotary r;
Button2 b;

void setup()
{
  // Configure GPIO pin for button (ESP8266 specific)
  // GPIO 3 (RX) swap the pin to a GPIO
  pinMode(BUTTON_PIN, FUNCTION_3);

  // Initialize button with handlers
  b.begin(BUTTON_PIN);
  b.setPressedHandler(initial_press);
  b.setDoubleClickHandler(repeat);
  b.setLongClickHandler(long_release);
  b.setLongClickDetectedHandler(long_press);

  // Initialize rotary encoder (only for non-motion rooms)
  if (!isMotionDetector((RoomType)room))
  {
    r.begin(ROTARY_PIN1, ROTARY_PIN2, CLICKS_PER_STEP);
    r.setChangedHandler(showDirection);
  }

  // Initialize mesh network
  mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.initOTAReceive(String(room));
}

void loop()
{
  mesh.update();
  send_change();

  // Only update rotary for non-motion rooms
  if (!isMotionDetector((RoomType)room))
  {
    r.loop();
  }

  b.loop();
}
