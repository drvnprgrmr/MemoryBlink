#include <Arduino.h>
// #include "MemoryBlink.h"
#include <Bonezegei_Printf.h>

#include <Button2.h>
#include "buzzer.h"
#include "main.h"

#define DEBUG 1

#define debugPrint(...) \
  do { \
    if (DEBUG) { \
      debug.printf(__VA_ARGS__); \
    } \
  } while (0)




constexpr uint8_t coloredLedPins[NUM_PADS]{12, 11, 9, 8};
constexpr uint8_t plainLedPins[NUM_PADS]{A1, A2, A6, A7};

constexpr uint8_t buttonPins[NUM_PADS]{7, 6, 5, 4};
constexpr uint8_t buzzerPin = 10;

Buzzer buzzer(buzzerPin);

// Create Button2 instances for each button
Button2 button1(buttonPins[0]);
Button2 button2(buttonPins[1]);
Button2 button3(buttonPins[2]);
Button2 button4(buttonPins[3]);

Bonezegei_Printf debug(&Serial);



void initLed() {
  for (int i = 0; i < NUM_PADS; i++) {
    pinMode(coloredLedPins[i], OUTPUT);
    digitalWrite(coloredLedPins[i], LOW);

    pinMode(plainLedPins[i], OUTPUT);
    digitalWrite(plainLedPins[i], LOW);
  }
}

void testLeds() {
  for (int i = 0; i < NUM_PADS; i++) {
    digitalWrite(coloredLedPins[i], HIGH);
    buzzer.play(Note::NOTE_C4);
    delay(200);
    digitalWrite(coloredLedPins[i], LOW);
    buzzer.stop();
  }

  for (int i = 0; i < NUM_PADS; i++) {
    digitalWrite(plainLedPins[i], HIGH);
    buzzer.play(Note::NOTE_D4);
    delay(200);
    digitalWrite(plainLedPins[i], LOW);
    buzzer.stop();
  }
}


void setup()
{
  Serial.begin(9600);
  Serial.println("MemoryBlink starting up!");

  // MemoryBlink game{coloredLedPins, plainLedPins, buttonPins, buzzerPin};

  // game.initLcd();

  // game.startScreen();
  // game.gameplaySetup();
  // game.playerSetup();


  initLed();
  while (true)
  {
    testLeds();
    // game.gameLoop();
    // game.startScreen();
  }
}
