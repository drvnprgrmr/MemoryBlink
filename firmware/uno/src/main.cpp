#include <Arduino.h>
#include "MemoryBlink.h"

constexpr uint8_t ledPins[NUM_PADS]{2, 3, 4, 5};
constexpr uint8_t buttonPins[NUM_PADS]{6, 7, 8, 9};
constexpr uint8_t buzzerPin = 10;

void setup()
{
  Serial.begin(9600);
  Serial.println("MemoryBlink starting up!");

  MemoryBlink game{ledPins, buttonPins, buzzerPin};
  while (true)
  {
    game.startGame();
  }
}
