#if defined(VSCODE)
#include "functions.h"
#else
#include "include/functions.h"
#endif

// Helper function to set value and trigger change
static inline void setValue(int newValue)
{
    value = newValue;
    change = true;
}

void showDirection(ESPRotary &r)
{
    setValue(r.directionToString(r.getDirection()) == "RIGHT" ? ROTARY_RIGHT : ROTARY_LEFT);
}

void initial_press(Button2 &btn)
{
    setValue(isMotionDetector((RoomType)room) ? MOTION_DETECTED : PRESS_SHORT);
}

void repeat(Button2 &btn)
{
    setValue(isMotionDetector((RoomType)room) ? MOTION_DETECTED : PRESS_REPEAT);
}

void long_release(Button2 &btn)
{
    setValue(isMotionDetector((RoomType)room) ? MOTION_DETECTED : PRESS_LONG_RELEASE);
}

void long_press(Button2 &btn)
{
    setValue(isMotionDetector((RoomType)room) ? MOTION_DETECTED : PRESS_LONG_PRESS);
}

// Interrupt handler for HLK-LD2410C radar sensor
void IRAM_ATTR radarInterrupt()
{
    unsigned long currentMillis = millis();

    // Debounce check
    if (currentMillis - lastMotionMillis < MOTION_DEBOUNCE_TIME)
    {
        return;
    }
    lastMotionMillis = currentMillis;

    // Set value based on current pin state
    if (digitalRead(RADAR_OUT_PIN) == RADAR_ACTIVE_STATE)
    {
        value = MOTION_DETECTED;
    }
    else
    {
        value = MOTION_CLEARED;
    }
    change = true;
}
