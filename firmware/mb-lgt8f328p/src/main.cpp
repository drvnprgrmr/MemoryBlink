
#include "main.h"

Bonezegei_Printf debug(&Serial);
LiquidCrystal_I2C lcd(PCF8574_ADDR_A21_A11_A01);
bool lcd_ready = false;

PadSettings padSettings = {LED_COLORED, false};
uint8_t game_mode = DEFAULT_GAME_MODE;
uint8_t selected_player = 1;
uint8_t score = 0;
uint8_t highscore = 0;

// Menu order: previous, next, enter, escape.
const uint8_t button_pins[NUM_PADS] = {7, 6, 5, 4};
const uint8_t buzzer_pin = 10;

Button2 buttons[NUM_PADS];
Buzzer buzzer(buzzer_pin);

void setup()
{
  Serial.begin(9600);
  Serial.println("MemoryBlink starting up!");

  buzzer.begin();
  lcd_ready = init_display();
  initLed();
  testLeds();
  init_btns();

  load_game_settings();
}

void loop()
{
  run_main_menu();
}
