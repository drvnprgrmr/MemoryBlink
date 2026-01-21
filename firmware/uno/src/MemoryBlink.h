#pragma once

#include <Bonezegei_Printf.h>
#include <LiquidCrystal_I2C.h>
#include "buzzer.h"
#include "button_manager.hpp"

#define NUM_PADS 4
#define MAX_SEQUENCE 100
#define MEM_START 0x10  // where to start saving variables

constexpr Note padNotes[NUM_PADS]{Note::NOTE_E4, Note::NOTE_CS4, Note::NOTE_A4, Note::NOTE_E3};

const MelodyStep successMelody[NUM_PADS] = {
    {Note::NOTE_C5, 100}, // Start mid
    {Note::NOTE_E5, 100}, // Up
    {Note::NOTE_G5, 100}, // Up
    {Note::NOTE_C6, 250}  // High note, held longer
};

const MelodyStep failureMelody[NUM_PADS] = {
    {Note::NOTE_G3, 300},  // Start low
    {Note::NOTE_FS3, 300}, // Down a semi-tone
    {Note::NOTE_F3, 300},  // Down a semi-tone
    {Note::NOTE_E3, 600}   // End on a long, low note
};

enum class GameMode
{
  CLASSIC,
  CLASSIC_REVERSED,
  SHUFFLE,
  SHUFFLE_REVERSED
};

typedef struct
{
  uint8_t sequence = 1; // plusone: 1, random, 0
  int8_t recall = 1;    // forwards: 1, backwards: -1
  bool sound = true;
  bool color = true;
} Gameplay;

class MemoryBlink
{

private:
  uint8_t const *ledPins;
  uint8_t const *plainLedPins;

  Buzzer buzzer;

  ButtonMan<NUM_PADS> buttonMan;

  Gameplay gameplay{};

  // todo: create this later in init
  LiquidCrystal_I2C lcd{0x27, 16, 2};

  uint32_t scanTimer{0}, scanTimeout{5 * 1000};

  int level{0}; // starts at level 0

  int generatedSequence[MAX_SEQUENCE]{};
  int generatedSequenceLength = 0;

  int inputSequence[MAX_SEQUENCE]{};
  int inputSequenceLength = 0;

  bool gameRunning{false};
  bool shouldGetInput{false};

  // Generic game mode handler type
  // todo: use concepts for esp32
  using GameModeHandler = void (MemoryBlink::*)();

private:
  void ledOn(int pos);
  void ledOn();
  void ledOff(int pos);
  void ledOff();
  void padOn(int pos);
  void padOff(int pos);

private:
  void addToGeneratedSequence(int count = 1);
  void displayGeneratedSequence();
  void clearGeneratedSequence();
  void clearInputSequence();

private:
  void levelUp();
  void endGame();

  void getInput(GameModeHandler handler);
  void loadGame(GameMode mode);
  void gameLoop(GameModeHandler handler, const char *gameModeName, uint8_t highscore);

private: // 4 different game modes
  uint8_t classicHighScoreLocation = 0x10;
  void classicHandler();
  uint8_t classicReversedHighScoreLocation = 0x11;
  void classicReversedHandler();
  uint8_t shuffleHighScoreLocation = 0x12;
  void shuffleHandler();
  uint8_t shuffleReversedHighScoreLocation = 0x13;
  void shuffleReversedHandler();

public:
  // todo: use a config struct instead
  MemoryBlink(uint8_t const ledPins[NUM_PADS], uint8_t const plainLedPins[NUM_PADS], uint8_t const buttonPins[NUM_PADS], uint8_t const buzzerPin);
  ~MemoryBlink();

public:
  void initLcd();
  void startGame();
};
