#pragma once

#include <Bonezegei_Printf.h>
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>
#include "buzzer.h"
#include "button_manager.hpp"

#define NUM_PADS 4
#define MEM_START 0x10 // where to start saving variables
#define MAX_SEQUENCE 40

#define SCAN_TIMEOUT 5000 // 5 seconds timeout per scan

#define DEFAULT_GAMEPLAY 0B1111 // default gameplay
#define DEFAULT_PLAYER 1

#define STARTING_FLASH_TIME 500
#define STARTING_DELAY_TIME 200

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

constexpr uint8_t gameplayMasks[NUM_PADS] = {
    bit(0), // SEQUENCE MASK  1:plusone 0:shuffle
    bit(1), // RECALL MASK    1:forward 0: backward
    bit(2), // SOUND MASK     1:sound 0:silent
    bit(3), // COLOR MASK     1:different 0:same
};

class MemoryBlink
{

private: // CONSTANT VARIABLES
  uint8_t const *ledPins;
  uint8_t const *plainLedPins;
  Buzzer buzzer;
  ButtonMan<NUM_PADS> buttonMan;
  LiquidCrystal_I2C lcd{0x27, 16, 2}; // todo: create this later in init

private:                                                                   // WORKING VARIABLES
  uint8_t gameplay{DEFAULT_GAMEPLAY};                                      // Each bit dictates a game setting
  uint8_t player{DEFAULT_PLAYER};                                          // Variable to track which player is currently playing (1-4)
  uint16_t flashTime{STARTING_FLASH_TIME}, delayTime{STARTING_DELAY_TIME}; // Controls the timing of displayed sequences
  uint32_t scanTimer{0};                                                   // Keeps track of elapsed time since last input
  uint8_t score{0}, highscore{0};                                          // Tracks current player's score and highscore for selected gameplay

  int generatedSequence[MAX_SEQUENCE]{};
  int generatedSequenceLength = 0;

  int inputSequence[MAX_SEQUENCE]{};
  int inputSequenceLength = 0;

  bool gameRunning{false};
  bool shouldGetInput{false};

private:
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

  uint8_t getHighscore();
  void setHighscore(uint8_t highscore);

  void getInput();

  void drawTop();
  void drawBottom();
  void drawBottom(const char *displayText);

private:
  void checkInput();

  void reinitialize();
  void printVars(const char *tag);

public:
  // todo: use a config struct instead
  MemoryBlink(uint8_t const ledPins[NUM_PADS], uint8_t const plainLedPins[NUM_PADS], uint8_t const buttonPins[NUM_PADS], uint8_t const buzzerPin);
  ~MemoryBlink();

public:
  void initLcd();
  void startScreen();
  void gameplaySetup();
  void playerSetup();
  void gameLoop();
};
