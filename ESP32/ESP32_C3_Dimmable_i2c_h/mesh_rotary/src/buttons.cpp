#if defined(VSCODE)
#include "functions.h"
#else
#include "include/functions.h"
#endif

void showDirection(ESPRotary& r) {
  if (r.directionToString(r.getDirection()) == "RIGHT") {
    value = 1000;
  } else if (r.directionToString(r.getDirection()) == "LEFT") {
    value = 2000;
  }
  change = true;
}

void initial_press(Button2& btn) {
  if (room <= badkamer) {
    value = 3000;
  } else {
    value = true;
  }
  change = true;
}

void repeat(Button2& btn) {
  if (room <= badkamer) {
    value = 3001;
  } else {
    value = true;
  }
  change = true;
}

void long_release(Button2& btn) {
  if (room <= badkamer) {
    value = 3003;
  } else {
    value = true;
  }
  change = true;
}

void long_press(Button2& btn) {
  if (room <= badkamer) {
    value = 3010;
  } else {
    value = true;
  }
  change = true;
}
