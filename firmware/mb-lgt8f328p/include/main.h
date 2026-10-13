#pragma once

#include <Arduino.h>
#include <Bonezegei_Printf.h>
#include <Button2.h>
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>

#include "buzzer.h"

constexpr uint8_t NUM_PADS = 4;
constexpr uint8_t MAX_LEVELS = 40;
constexpr uint8_t MAX_SEQUENCE_LENGTH = MAX_LEVELS + NUM_PADS - 1;
constexpr uint8_t EEPROM_MODE_ADDRESS = 0x0F;
constexpr uint8_t EEPROM_PLAYER_ADDRESS = 0x0E;
constexpr uint8_t EEPROM_HIGHSCORE_BASE = 0x10;
constexpr uint8_t EEPROM_MODE_VERSION_ADDRESS = 0x0D;
constexpr uint8_t EEPROM_MODE_VERSION = 1;
constexpr uint16_t LEVEL_INTERVAL_MS = 4000;

enum LedType : uint8_t
{
    LED_COLORED = 0,
    LED_PLAIN = 1,
};

enum GameModeBit : uint8_t
{
    MODE_SOUND = 0x01,           // Difficulty weight 1
    MODE_PLAIN_LEDS = 0x02,      // Difficulty weight 2; colored LEDs are the default
    MODE_APPEND_SEQUENCE = 0x04, // Difficulty weight 4
    MODE_FORWARD_RECALL = 0x08,  // Difficulty weight 8
};

constexpr uint8_t DEFAULT_GAME_MODE = MODE_FORWARD_RECALL | MODE_APPEND_SEQUENCE | MODE_SOUND;

enum ButtonEventType : uint8_t
{
    BUTTON_PRESSED,
    BUTTON_RELEASED,
    BUTTON_CLICKED,
};

struct ButtonEvent
{
    uint8_t pad;
    ButtonEventType type;
};

struct PadSettings
{
    LedType ledType;
    bool soundOff;
};

extern Bonezegei_Printf debug;
extern LiquidCrystal_I2C lcd;
extern bool lcd_ready;
extern PadSettings padSettings;
extern uint8_t game_mode;
extern uint8_t selected_player;
extern uint8_t score;
extern uint8_t highscore;
extern const uint8_t button_pins[NUM_PADS];
extern const uint8_t buzzer_pin;
extern Button2 buttons[NUM_PADS];
extern Buzzer buzzer;
extern const Note padNotes[NUM_PADS];
extern const MelodyStep successMelody[NUM_PADS];
extern const MelodyStep failureMelody[NUM_PADS];

extern const uint8_t coloredLedPins[NUM_PADS];
extern const uint8_t plainLedPins[NUM_PADS];

#ifndef DEBUG
#define DEBUG 1
#endif

#define debugPrint(...)          \
    do                             \
    {                              \
        if (DEBUG)                   \
        {                            \
            debug.printf(__VA_ARGS__); \
        }                            \
    } while (0)

void initLed();
void ledOn(int pos, LedType type);
void ledOnFor(int pos, int duration, LedType type);
void ledOff(int pos, LedType type);

void pad_on(int pos);
void pad_off(int pos);
void testLeds();
void init_btns();
void loop_btns();
bool pop_button_event(ButtonEvent &event);
void clear_button_events();
void set_game_input_enabled(bool enabled);
void set_button_feedback_enabled(bool enabled);

bool init_display();
void show_mode_menu(uint8_t selection);
void show_main_menu(uint8_t selection, bool has_previous_settings);
void show_idle_screen();
void show_settings_saved();
void show_no_previous_settings();
void show_player_selection();
void show_game_status();
void show_watch_prompt();
void show_input_prompt();
void show_game_result(bool won);

void load_game_settings();
void apply_game_mode();
uint8_t load_highscore();
void save_highscore(uint8_t value);
void configure_game_mode();
void run_main_menu();
bool select_player();
void play_game();