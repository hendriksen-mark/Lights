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
  // Configure GPIO pins based on room type
  if (isMotionDetector((RoomType)room))
  {
    // Motion detector room: setup radar sensor with interrupt
    pinMode(RADAR_OUT_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(RADAR_OUT_PIN), radarInterrupt, CHANGE);
  }
  else
  {
    // Rotary switch room: setup button
    // GPIO 3 (RX) swap the pin to a GPIO
    pinMode(BUTTON_PIN, FUNCTION_3);

    // Initialize button with handlers
    b.begin(BUTTON_PIN);
    b.setPressedHandler(initial_press);
    b.setDoubleClickHandler(repeat);
    b.setLongClickHandler(long_release);
    b.setLongClickDetectedHandler(long_press);

    // Initialize rotary encoder
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

  // Only update rotary and button for non-motion rooms
  if (!isMotionDetector((RoomType)room))
  {
    r.loop();
    b.loop();
  }
}
