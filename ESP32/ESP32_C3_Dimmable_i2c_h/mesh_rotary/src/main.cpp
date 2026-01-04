#if defined(VSCODE)
#include "functions.h"
#else
#include "include/functions.h"
#endif

painlessMesh mesh;

uint32_t master = 0;
unsigned long MasterPreviousMillis = 0;

ESPRotary r;
Button2 b;

void setup() {
  //********** CHANGE PIN FUNCTION  TO GPIO **********
  //GPIO 1 (TX) swap the pin to a GPIO.
  //pinMode(1, FUNCTION_3);
  //GPIO 3 (RX) swap the pin to a GPIO.
  pinMode(BUTTON_PIN, FUNCTION_3);
  //**************************************************
  b.begin(BUTTON_PIN);
  b.setPressedHandler(initial_press);
  b.setDoubleClickHandler(repeat);
  b.setLongClickHandler(long_release);
  b.setLongClickDetectedHandler(long_press);


  r.begin(ROTARY_PIN1, ROTARY_PIN2, CLICKS_PER_STEP);
  r.setChangedHandler(showDirection);
  mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.initOTAReceive(String(room));
}

void loop() {
  mesh.update();
  send_change();
  r.loop();
  b.loop();
}
