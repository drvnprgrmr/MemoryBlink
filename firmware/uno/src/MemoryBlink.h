#pragma once

#include <Bonezegei_Printf.h>
#include "buzzer.h"

#define NUM_PADS 4
#define MAX_SEQUENCE 100

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

/* -------------------------------------------------------------------------- */

enum class ButtonState
{
  IDLE,
  RELEASED,
  PRESSED,
  HELD
};

struct Button
{
  ButtonState state{ButtonState::IDLE};
  uint32_t idleTimer{0};
  uint32_t holdTimer{0};
  uint8_t value;

  // add initializer for the value
  Button(uint8_t v) : value(v) {};

  // if none is given don't set the value
  Button() = default;
};

/* -------------------------------------------------------------------------- */

enum class GameMode
{
  CLASSIC,
  CLASSIC_REVERSED,
  SHUFFLE,
  SHUFFLE_REVERSED
};

class MemoryBlink
{

private:
  uint8_t const *ledPins;

  uint8_t const *buttonPins;
  Button buttons[NUM_PADS]{0, 1, 2, 3};
  Button *updatedButtons[NUM_PADS];
  uint8_t updatedButtonsCount{0};
  uint32_t debounceTime{300}, idleTime{5 * 1000}, holdTime{2 * 1000};

  Buzzer buzzer;

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
  void addToGeneratedSequence();
  void addToGeneratedSequence(int count);
  void displayGeneratedSequence();
  void clearGeneratedSequence();
  void clearInputSequence();

private:
  void levelUp();
  void endGame();

  void scanButtons();
  void getInput(GameModeHandler handler);
  void gameLoop(GameModeHandler handler, const char *gameModeName, uint8_t highscore);

private:
  uint8_t classicHighScoreLocation = 0x10;
  void classicHandler();
  uint8_t classicReversedHighScoreLocation = 0x11;
  void classicReversedHandler();
  uint8_t shuffleHighScoreLocation = 0x12;
  void shuffleHandler();
  uint8_t shuffleReversedHighScoreLocation = 0x13;
  void shuffleReversedHandler();

public:
  MemoryBlink(uint8_t const ledPins[NUM_PADS], uint8_t const buttonPins[NUM_PADS], uint8_t const buzzerPin);
  ~MemoryBlink();

public:
  void startGame(GameMode mode);
};
