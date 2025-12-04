#include "MemoryBlink.h"

Bonezegei_Printf debug(&Serial);

MemoryBlink::MemoryBlink(uint8_t const ledPins[NUM_PADS], uint8_t const buttonPins[NUM_PADS], uint8_t const buzzerPin)
    : ledPins(ledPins), buttonPins(buttonPins), buzzer(buzzerPin)
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
  buzzer.playMelody(successMelody, successLen);

  // clear input sequence
  clearInputSequence();

  // stop scanning buttons
  shouldScanButtons = false;

  // add delay after
  delay(1000);
}

void MemoryBlink::endGame()
{
  // add delay before
  delay(1000);

  // todo: save high score

  // reset level
  level = 0;
  debug.printf("You Lost!\n");

  // play failure melody
  buzzer.playMelody(failureMelody, failureLen);

  // clear both sequences
  clearInputSequence();
  clearGeneratedSequence();

  // stop scanning buttons
  shouldScanButtons = false;

  // end current game
  gameRunning = false;

  // add delay after
  delay(1000);
}

void MemoryBlink::getButtonInput(GameModeHandler onPress)
{
  shouldScanButtons = true;
  scanTimer = millis();

  static auto buttonDidTimeout{
      [this]() -> bool
      {
        return millis() - scanTimer > scanTimeout;
      }};

  while (!buttonDidTimeout() && shouldScanButtons)
  {
    for (int i = 0; i < NUM_PADS; i++)
    {
      int level = digitalRead(buttonPins[i]);
      Button &button = buttons[i];

      if (level == 0) // i.e. button has been pulled down
      {
        if ((button.state == ButtonState::IDLE || button.state == ButtonState::RELEASED) &&
            (millis() - button.holdTimer > debounceTime))
        {
          button.state = ButtonState::PRESSED;
          button.holdTimer = millis();

          // Reset timeout timer
          scanTimer = millis();

          // blink pad
          padOn(i);
          delay(200);
          padOff(i);

          // update input sequence
          inputSequence[inputSequenceLength++] = i;

          // call the passed in function
          (onPress != nullptr) ? (this->*onPress)() : debug.printf("WARN: onPress not defined!\n");
        }

        else if (button.state == ButtonState::PRESSED &&
                 (millis() - button.holdTimer > holdTime))
        {
          button.state = ButtonState::HELD;

          debug.printf("Button held: %i\n", i);
        }
      }
      else
      {
        if (button.state == ButtonState::PRESSED || button.state == ButtonState::HELD)
        {
          button.state = ButtonState::RELEASED;
        }
        else if (button.state == ButtonState::RELEASED)
        {
          button.state = ButtonState::IDLE;
        }
      }
    }
  }

  if (buttonDidTimeout())
  {
    debug.printf("Sorry, you timed out!\n");
    delay(1000);
    endGame();
  }
}

void MemoryBlink::startGame(GameMode mode)
{
  switch (mode)
  {

  case GameMode::CLASSIC:
  {
    gameLoop(&MemoryBlink::classicHandler);
    break;
  }

  case GameMode::SHUFFLE:
  {
    gameLoop(&MemoryBlink::shuffleHandler);
    break;
  }

  default:
    break;
  }
}

void MemoryBlink::gameLoop(GameModeHandler onPress)
{
  // start game with a sequence equal to the number of pads to avoid trivial levels
  addToGeneratedSequence(NUM_PADS);

  debug.printf("Start ===========================\n");

  gameRunning = true;
  while (gameRunning)
  {
    // display sequence
    displayGeneratedSequence();

    // get user input
    getButtonInput(onPress);
  }

  debug.printf("End ===========================\n\n\n");
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
