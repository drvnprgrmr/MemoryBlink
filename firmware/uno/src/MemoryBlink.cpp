#include "MemoryBlink.h"

Bonezegei_Printf debug(&Serial);

MemoryBlink::MemoryBlink(uint8_t const ledPins[NUM_PADS], uint8_t const buttonPins[NUM_PADS], uint8_t const buzzerPin)
    : ledPins(ledPins), buzzer(buzzerPin), buttonMan(buttonPins)
{
  // Initialize pins
  for (int i = 0; i < NUM_PADS; i++)
  {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  // Initialize buzzer
  Buzzer buzzer{buzzerPin};

  // Seed RNG (if A0 is floating it helps vary the sequence)
  randomSeed(analogRead(A0));

  // Startup blink
  ledOn();
  buzzer.play(Note::NOTE_B0);

  delay(1000);

  ledOff();
  buzzer.stop();

  delay(1000);
}

MemoryBlink::~MemoryBlink()
{
}

/* -------------------------------------------------------------------------- */
/*                                   OUTPUTS                                  */
/* -------------------------------------------------------------------------- */

void MemoryBlink::ledOn(int pos)
{
  digitalWrite(ledPins[pos], 1);
}

void MemoryBlink::ledOn()
{
  for (int i = 0; i < NUM_PADS; i++)
  {
    ledOn(i);
  }
}

void MemoryBlink::ledOff(int pos)
{
  digitalWrite(ledPins[pos], 0);
}

void MemoryBlink::ledOff()
{
  for (int i = 0; i < NUM_PADS; i++)
  {
    ledOff(i);
  }
}

void MemoryBlink::padOn(int pos)
{
  ledOn(pos);
  buzzer.play(padNotes[pos]);
}

void MemoryBlink::padOff(int pos)
{
  ledOff(pos);
  buzzer.stop();
}

/* -------------------------------------------------------------------------- */
/*                                  SEQUENCES                                 */
/* -------------------------------------------------------------------------- */

void MemoryBlink::addToGeneratedSequence()
{
  generatedSequence[generatedSequenceLength++] = random(NUM_PADS);
}

void MemoryBlink::addToGeneratedSequence(int count)
{
  if (generatedSequenceLength < MAX_SEQUENCE)
  {
    for (int i = 0; i < count; i++)
    {
      addToGeneratedSequence();
    }
  }
}

void MemoryBlink::displayGeneratedSequence()

{
  for (int i = 0; i < generatedSequenceLength; i++)
  {
    int pos = generatedSequence[i];

    padOn(pos);
    delay(500);
    padOff(pos);

    delay(200);
  }
}

void MemoryBlink::clearGeneratedSequence()
{
  generatedSequenceLength = 0;
}

void MemoryBlink::clearInputSequence()
{
  inputSequenceLength = 0;
}

/* -------------------------------------------------------------------------- */
/*                                  GAMEPLAY                                  */
/* -------------------------------------------------------------------------- */

void MemoryBlink::levelUp()
{
  // add delay before
  delay(1000);

  // increase level
  level++;
  debug.printf("Level up: %i\n", level);

  // play success melody
  buzzer.playMelody(successMelody, NUM_PADS);

  // clear input sequence
  clearInputSequence();

  // stop scanning buttons
  shouldGetInput = false;

  // add delay after
  delay(1000);
}

void MemoryBlink::endGame()
{
  // add delay before
  delay(1000);

  // reset level
  level = 0;
  debug.printf("You Lost!\n");

  // play failure melody
  buzzer.playMelody(failureMelody, NUM_PADS);

  // clear both sequences
  clearInputSequence();
  clearGeneratedSequence();

  // stop scanning buttons
  shouldGetInput = false;

  // end current game
  gameRunning = false;

  // add delay after
  delay(1000);
}

void MemoryBlink::getInput(GameModeHandler handler)
{
  shouldGetInput = true;
  scanTimer = millis();

  while (shouldGetInput)
  {
    if (millis() - scanTimer > scanTimeout)
    {
      // getting input timed out
      debug.printf("Sorry, you timed out!\n");
      delay(1000);
      endGame();
    }

    // scan the buttons to update the states
    buttonMan.scanButtons();

    if (buttonMan.updatedButtonsCount)
    {
      Button const *updatedButton = buttonMan.updatedButtons[0]; // just take first input

      if (updatedButton->state == ButtonState::PRESSED)
      {
        // reset timeout timer
        scanTimer = millis();

        // blink pad
        padOn(updatedButton->value);
        delay(200);
        padOff(updatedButton->value);

        // update input sequence
        inputSequence[inputSequenceLength++] = updatedButton->value;

        // call the passed in function
        (handler != nullptr) ? (this->*handler)() : debug.printf("WARN: handler not defined!\n");
      }
    }
  }
}

void MemoryBlink::startGame(GameMode mode)
{
  switch (mode)
  {

  case GameMode::CLASSIC:
  {

    gameLoop(&MemoryBlink::classicHandler, "Classic", eeprom_read_byte(&classicHighScoreLocation));
    break;
  }

  case GameMode::CLASSIC_REVERSED:
  {
    gameLoop(&MemoryBlink::classicReversedHandler, "Classic Reversed", eeprom_read_byte(&classicReversedHighScoreLocation));
    break;
  }

  case GameMode::SHUFFLE:
  {
    gameLoop(&MemoryBlink::shuffleHandler, "Shuffle", eeprom_read_byte(&shuffleHighScoreLocation));
    break;
  }

  case GameMode::SHUFFLE_REVERSED:
  {
    gameLoop(&MemoryBlink::shuffleReversedHandler, "Shuffle Reversed", eeprom_read_byte(&shuffleReversedHighScoreLocation));
    break;
  }

  default:
    break;
  }
}

void MemoryBlink::gameLoop(GameModeHandler handler, const char *gameModeName, uint8_t highscore)
{
  // start game with a sequence equal to the number of pads to avoid trivial levels
  addToGeneratedSequence(NUM_PADS);

  debug.printf("%s (highscore: %i)\n", gameModeName, highscore);
  debug.printf("----------------------\n");

  gameRunning = true;
  while (gameRunning)
  {
    // display sequence
    displayGeneratedSequence();

    // get user input
    getInput(handler);
  }

  debug.printf("===========================\n\n\n");
}

/* -------------------------------------------------------------------------- */
/*                                 GAME MODES                                 */
/* -------------------------------------------------------------------------- */

// Classic mode handler
void MemoryBlink::classicHandler()
{
  // Check if user enters the exact sequence that was described
  int inputSequencePos = inputSequenceLength - 1;
  if (inputSequence[inputSequencePos] != generatedSequence[inputSequencePos])
  {
    // update high score if it's been beaten
    if (eeprom_read_byte(&classicHighScoreLocation) > level)
    {
      eeprom_update_byte(&classicHighScoreLocation, level);
    }

    return endGame();
  }

  // Check if the last sequence has been entered
  else if (inputSequenceLength == generatedSequenceLength)
  {
    levelUp();

    // add a random position to the generated sequence
    addToGeneratedSequence();
  }
}

// Classic Reversed mode handler
void MemoryBlink::classicReversedHandler()
{
  // Check if user enters the reverse sequence that was described
  int inputSequencePos = inputSequenceLength - 1;
  if (inputSequence[inputSequencePos] != generatedSequence[generatedSequenceLength - inputSequenceLength])
  {
    // update high score if it's been beaten
    if (eeprom_read_byte(&classicReversedHighScoreLocation) > level)
    {
      eeprom_update_byte(&classicReversedHighScoreLocation, level);
    }

    return endGame();
  }

  // Check if the last sequence has been entered
  else if (inputSequenceLength == generatedSequenceLength)
  {
    levelUp();

    // add a random position to the generated sequence
    addToGeneratedSequence();
  }
}

// Shuffle mode handler
void MemoryBlink::shuffleHandler()
{
  // Check if user enters the exact sequence that was described
  int inputSequencePos = inputSequenceLength - 1;
  if (inputSequence[inputSequencePos] != generatedSequence[inputSequencePos])
  {
    // update high score if it's been beaten
    if (eeprom_read_byte(&shuffleHighScoreLocation) > level)
    {
      eeprom_update_byte(&shuffleHighScoreLocation, level);
    }

    return endGame();
  }

  // Check if the last sequence has been entered
  else if (inputSequenceLength == generatedSequenceLength)
  {
    // level up the game
    levelUp();

    // reset the generated sequence
    clearGeneratedSequence();

    // add new set of random sequence plus offset
    addToGeneratedSequence(NUM_PADS + level);
  }
}

// Shuffle Reversed mode handler
void MemoryBlink::shuffleReversedHandler()
{
  // Check if user enters the exact sequence that was described
  int inputSequencePos = inputSequenceLength - 1;
  if (inputSequence[inputSequencePos] != generatedSequence[generatedSequenceLength - inputSequenceLength])
  {
    // update high score if it's been beaten
    if (eeprom_read_byte(&shuffleReversedHighScoreLocation) > level)
    {
      eeprom_update_byte(&shuffleReversedHighScoreLocation, level);
    }

    return endGame();
  }

  // Check if the last sequence has been entered
  else if (inputSequenceLength == generatedSequenceLength)
  {
    // level up the game
    levelUp();

    // reset the generated sequence
    clearGeneratedSequence();

    // add new set of random sequence plus offset
    addToGeneratedSequence(NUM_PADS + level);
  }
}
