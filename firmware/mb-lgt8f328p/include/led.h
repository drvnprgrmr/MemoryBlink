#pragma once

#include "main.h"

constexpr uint8_t coloredLedPins[NUM_PADS]{12, 11, 9, 8};
constexpr uint8_t plainLedPins[NUM_PADS]{A1, A2, A6, A7};

enum LedType {
  LED_COLORED = 0,
  LED_PLAIN = 1,
};

void initLed();
void ledOn(int pos, LedType type);
void ledOnFor(int pos, int duration, LedType type);
void ledOff(int pos, LedType type);