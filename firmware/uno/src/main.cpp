#include <Arduino.h>
#include "MemoryBlink.h"


constexpr uint8_t ledPins[NUM_PADS]{9, 8, 7, 6};
constexpr uint8_t plainLedPins[NUM_PADS]{5, 4, 3, 2};

constexpr uint8_t buttonPins[NUM_PADS]{A0, A1, A2, A3};
constexpr uint8_t buzzerPin = 10;

void setup()
{
  Serial.begin(9600);
  Serial.println("MemoryBlink starting up!");

  MemoryBlink game{ledPins, plainLedPins, buttonPins, buzzerPin};

  game.initLcd();

  game.startScreen();
  game.gameplaySetup();
  game.playerSetup();


  while (true)
  {
    game.gameLoop();
    game.startScreen();
  }
}
