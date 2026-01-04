#if defined(VSCODE)
#include "functions.h"
#else
#include "include/functions.h"
#endif

void showDirection(ESPRotary& r) {
  if (r.directionToString(r.getDirection()) == "RIGHT") {
    value = ROTARY_RIGHT;
  } else if (r.directionToString(r.getDirection()) == "LEFT") {
    value = ROTARY_LEFT;
  }
  change = true;
}

void initial_press(Button2& btn) {
  if (room <= badkamer) {
    value = PRESS_SHORT;
  } else {
    value = MOTION_DETECTED;
  }
  change = true;
}

void repeat(Button2& btn) {
  if (room <= badkamer) {
    value = PRESS_REPEAT;
  } else {
    value = MOTION_DETECTED;
  }
  change = true;
}

void long_release(Button2& btn) {
  if (room <= badkamer) {
    value = PRESS_LONG_RELEASE;
  } else {
    value = MOTION_DETECTED;
  }
  change = true;
}

void long_press(Button2& btn) {
  if (room <= badkamer) {
    value = PRESS_LONG_PRESS;
  } else {
    value = MOTION_DETECTED;
  }
  change = true;
}
