#pragma once

#include <Bonezegei_Printf.h>
#include <LiquidCrystal_I2C.h>
#include "buzzer.h"
#include "button_manager.hpp"

#define NUM_PADS 4
#define MAX_SEQUENCE 40
#define MEM_START 0x10 // where to start saving variables

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

#define GAMEPLAY_SEQUENCE_IDX 0 // 1:plusone 0:shuffle
#define GAMEPLAY_RECALL_IDX 1   // 1:forward 0: backward
#define GAMEPLAY_SOUND_IDX 2    // 1:sound 0:silent
#define GAMEPLAY_COLOR_IDX 3    // 1:different 0:same

class MemoryBlink
{

private:
  uint8_t const *ledPins;
  uint8_t const *plainLedPins;

  Buzzer buzzer;

  ButtonMan<NUM_PADS> buttonMan;

  // determines where we start saving data in the eeprom
  uint8_t memStart{0x10};

  // Each element dictates a game setting
  uint8_t gameplay[NUM_PADS]{1, 1, 1, 1};

  // Variable to track which player is currently playing (1-4)
  uint8_t player{1};

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

  void getInput();
  
  void gameplaySetup();
  void drawGameplaySetupScreen();
  
  void playerSetup();
  void drawPlayerSetupScreen();
  
  void gameLoop();
  void drawGameLoopScreen();

private:
  void handler();

public:
  // todo: use a config struct instead
  MemoryBlink(uint8_t const ledPins[NUM_PADS], uint8_t const plainLedPins[NUM_PADS], uint8_t const buttonPins[NUM_PADS], uint8_t const buzzerPin);
  ~MemoryBlink();

public:
  void initLcd();
  void startGame();
};
