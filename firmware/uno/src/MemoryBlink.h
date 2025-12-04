#pragma once

#include <Bonezegei_Printf.h>
#include "buzzer.h"

#define NUM_PADS 4
#define MAX_SEQUENCE 100

constexpr Note padNotes[NUM_PADS]{Note::NOTE_E4, Note::NOTE_CS4, Note::NOTE_A4, Note::NOTE_E3};

const MelodyStep successMelody[] = {
    {Note::NOTE_C5, 100}, // Start mid
    {Note::NOTE_E5, 100}, // Up
    {Note::NOTE_G5, 100}, // Up
    {Note::NOTE_C6, 250}  // High note, held longer
};
const int successLen = sizeof(successMelody) / sizeof(successMelody[0]);

const MelodyStep failureMelody[] = {
    {Note::NOTE_G3, 300},  // Start low
    {Note::NOTE_FS3, 300}, // Down a semi-tone
    {Note::NOTE_F3, 300},  // Down a semi-tone
    {Note::NOTE_E3, 600}   // End on a long, low note
};
const int failureLen = sizeof(failureMelody) / sizeof(failureMelody[0]);

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
  uint32_t holdTimer{0};
};

/* -------------------------------------------------------------------------- */

enum class GameMode
{
  CLASSIC,
  REVERSE,
  SHUFFLE,
};

class MemoryBlink
{

private:
  uint8_t const *ledPins;
  uint8_t const *buttonPins;

  Buzzer buzzer;
  Button buttons[NUM_PADS]{};

  int level{0}; // starts at level 0

  int generatedSequence[MAX_SEQUENCE]{};
  int generatedSequenceLength = 0;

  int inputSequence[MAX_SEQUENCE]{};
  int inputSequenceLength = 0;

  bool gameRunning = false;

  bool shouldScanButtons{false};

  uint32_t debounceTime{300}, holdTime{2 * 1000};
  uint32_t scanTimer{0}, scanTimeout{5 * 1000};

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
  void getButtonInput(GameModeHandler onPress);
  void gameLoop(GameModeHandler onPress);

private:
  void classicHandler();
  void shuffleHandler();

public:
  MemoryBlink(uint8_t const ledPins[NUM_PADS], uint8_t const buttonPins[NUM_PADS], uint8_t const buzzerPin);
  ~MemoryBlink();

public:
  void startGame(GameMode mode);
};
