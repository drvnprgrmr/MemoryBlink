#include <Arduino.h>
// #include "MemoryBlink.h"
#include <Bonezegei_Printf.h>

#include <Button2.h>
#include "main.h"
#include "buzzer.h"
#include "led.h"

Bonezegei_Printf debug(&Serial);

#define DEBUG 1

#define debugPrint(...) \
  do { \
    if (DEBUG) { \
      debug.printf(__VA_ARGS__); \
    } \
  } while (0)



typedef struct GameplaySettings
{
  LedType ledType;
  bool soundOff;
} GameplaySettings;


GameplaySettings gameplaySettings = {LED_COLORED, false};

Button2 buttons[NUM_PADS];
constexpr uint8_t buttonPins[NUM_PADS]{7, 6, 5, 4};
constexpr uint8_t buzzerPin = 10;

Buzzer buzzer(buzzerPin);

void game_btn_pressed(Button2 &btn)
{
  debugPrint("Button %d pressed\n", btn.getID());
  ledOn(btn.getID(), gameplaySettings.ledType);
  if (!gameplaySettings.soundOff)
  {
    buzzer.play(Note::NOTE_C4);
  }
}

void game_btn_released(Button2 &btn)
{
  debugPrint("Button %d released\n", btn.getID());
  ledOff(btn.getID(), gameplaySettings.ledType);
  buzzer.stop();
}

void init_btns()
{
  for (int i = 0; i < NUM_PADS; i++)
  {
    buttons[i].begin(buttonPins[i]);
    buttons[i].setID(i);
    buttons[i].setPressedHandler(game_btn_pressed);
    buttons[i].setReleasedHandler(game_btn_released);
  }
}

void loop_btns()
{
  for (int i = 0; i < NUM_PADS; i++)
  {
    buttons[i].loop();
  }
}

void testLeds()
{
    for (int i = 0; i < NUM_PADS; i++)
    {
        ledOn(i, LED_COLORED);
        buzzer.play(Note::NOTE_C4);
        delay(500);
        ledOff(i, LED_COLORED);
        buzzer.stop();
    }

    for (int i = 0; i < NUM_PADS; i++)
    {
        ledOn(i, LED_PLAIN);
        buzzer.play(Note::NOTE_D4);
        delay(500);
        ledOff(i, LED_PLAIN);
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
  testLeds();

  init_btns();
}

void loop()
{
  loop_btns();
}
