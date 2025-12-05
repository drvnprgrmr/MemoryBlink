#include <Arduino.h>
#include "MemoryBlink.h"

constexpr uint8_t ledPins[NUM_PADS]{2, 3, 4, 5};
constexpr uint8_t buttonPins[NUM_PADS]{6, 7, 8, 9};
constexpr uint8_t buzzerPin = 10;

void setup()
{
  Serial.begin(9600);
  Serial.println("MemoryBlink starting up!");
}

void loop()
{
  MemoryBlink game{ledPins, buttonPins, buzzerPin};

  game.startGame(GameMode::CLASSIC);

  game.startGame(GameMode::CLASSIC_REVERSED);

  game.startGame(GameMode::SHUFFLE);

  game.startGame(GameMode::SHUFFLE_REVERSED);
}
