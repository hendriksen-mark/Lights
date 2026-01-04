#ifndef CONFIG_H
#define CONFIG_H

#pragma once

// #define room slaapkamer
// #define room woonkamer
#define room keuken
// #define room gang
// #define room badkamer
// #define room gang_beweging
// #define room badkamer_beweging

#define MESH_PREFIX "HomeMesh"
#define MESH_PASSWORD "Qwertyuiop1"
#define MESH_PORT 5555

#define ROTARY_PIN1 2
#define ROTARY_PIN2 0
#define BUTTON_PIN 3
#define CLICKS_PER_STEP 4

#define REQUEST_TIMEOUT 2000
#define MOTION_DEBOUNCE_TIME 500 // ms - prevent motion spam

#endif // CONFIG_H
