#ifndef CONFIG_H
#define CONFIG_H

#pragma once

// ============================================
// Configuration
// ============================================
#define STEP_MS 6 // milliseconds per step in transitions

// ============================================
// Select the light by uncommenting one of these
// ============================================
// #define woonkamer_1
// #define woonkamer_2
// #define woonkamer_3
// #define keuken
// #define slaapkamer
// #define gang
#define badkamer

// ============================================
// Do not edit below this line unless necessary, linked to i2c.cpp line 15
// ============================================
#ifdef woonkamer_1
#define ADRESS 9
#define PIN 1
#endif

#ifdef woonkamer_2
#define ADRESS 10
#define PIN 1
#endif

#ifdef woonkamer_3
#define ADRESS 11
#define PIN 1
#endif

#ifdef keuken
#define ADRESS 12
#define PIN 1
#endif

#ifdef slaapkamer
#define ADRESS 13
#define PIN 1
#endif

#ifdef gang
#define ADRESS 14
#define PIN 4
#endif

#ifdef badkamer
#define ADRESS 15
#define PIN 4
#endif

#endif // CONFIG_H
