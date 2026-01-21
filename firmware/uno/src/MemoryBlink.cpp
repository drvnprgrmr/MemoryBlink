#include "MemoryBlink.h"

Bonezegei_Printf debug(&Serial);

// todo: move to header file
// LiquidCrystal_I2C lcd(0x27, 16, 2);

MemoryBlink::MemoryBlink(uint8_t const ledPins[NUM_PADS], uint8_t const plainLedPins[NUM_PADS], uint8_t const buttonPins[NUM_PADS], uint8_t const buzzerPin)
    : ledPins(ledPins), plainLedPins(plainLedPins), buzzer(buzzerPin), buttonMan(buttonPins)
{
  // Initialize pins
  for (int i = 0; i < NUM_PADS; i++)
  {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], 0);

    pinMode(plainLedPins[i], OUTPUT);
    digitalWrite(plainLedPins[i], 0);

    pinMode(buttonPins[i], INPUT_PULLUP);
  }

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
  if (gameplay[GAMEPLAY_COLOR_IDX])
  {
    digitalWrite(ledPins[pos], 1);
  }
  else
  {
    digitalWrite(plainLedPins[pos], 1);
  }
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
  if (gameplay[GAMEPLAY_COLOR_IDX])
  {
    digitalWrite(ledPins[pos], 0);
  }
  else
  {
    digitalWrite(plainLedPins[pos], 0);
  }
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
  if (gameplay[GAMEPLAY_SOUND_IDX])
  {
    buzzer.play(padNotes[pos]);
  }
}

void MemoryBlink::padOff(int pos)
{
  ledOff(pos);
  if (gameplay[GAMEPLAY_SOUND_IDX])
  {
    buzzer.stop();
  }
}

/* -------------------------------------------------------------------------- */
/*                                  SEQUENCES                                 */
/* -------------------------------------------------------------------------- */

void MemoryBlink::addToGeneratedSequence(int count)
{
  if (generatedSequenceLength < MAX_SEQUENCE)
  {
    for (int i = 0; i < count; i++)
    {
      generatedSequence[generatedSequenceLength++] = random(NUM_PADS);
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

  // update level on screen
  lcd.setCursor(0, 1);
  lcd.print("Level:");
  lcd.print(level);

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

  // reset the screen state
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Memory Blink!");

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

void MemoryBlink::getInput()
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

    // check if a button was pressed
    Button const *updatedButton = buttonMan.getUpdate(ButtonState::PRESSED);

    if (updatedButton != nullptr)
    {
      // reset timeout timer
      scanTimer = millis();

      // blink pad
      padOn(updatedButton->value);
      delay(200);
      padOff(updatedButton->value);

      // update input sequence
      inputSequence[inputSequenceLength++] = updatedButton->value;

      // call the handler
      handler();
    }
  }
}

void MemoryBlink::gameLoop(uint8_t highscore)
{
  // add to the sequence
  addToGeneratedSequence();
  
  debug.printf("G:%i%i%i%i (highscore: %i)\n", gameplay[0], gameplay[1], gameplay[2], gameplay[3], highscore);
  debug.printf("----------------------\n");
  
  // print high score to the screen and delay
  /* ----------------------------------- top ---------------------------------- */
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Player:1");

  lcd.setCursor(11, 0);
  lcd.print("(G");
  lcd.print(gameplay[0]);
  lcd.print(gameplay[1]);
  lcd.print(gameplay[2]);
  lcd.print(gameplay[3]);
  lcd.print(")");


/* --------------------------------- bottom --------------------------------- */

  lcd.setCursor(0, 1);
  lcd.print("Score:");
  if (level < 10)
  {
    lcd.print("0");
  }
  lcd.print(level);

  lcd.setCursor(9, 1);
  lcd.print("(HS:");
  if (highscore < 10)
  {
    lcd.print("0");
  }
  lcd.print(highscore);
  lcd.print(")");


  /* -------------------------------------------------------------------------- */

  gameRunning = true;
  while (gameRunning)
  {
    // display sequence
    displayGeneratedSequence();

    // get user input
    getInput();
  }

  debug.printf("===========================\n\n\n");
}

void MemoryBlink::startGame()
{
  // todo: find out a way to turn off the arduino after a while

  // get user's choice for the current game
  while (true)
  {
    Button const *updatedButton = buttonMan.getUpdate(ButtonState::PRESS_RELEASE); // just take first input

    if (updatedButton != nullptr)
    {
      // toggle that particular gameplay setting
      gameplay[updatedButton->value] ^= 1;

      // turn led at position on or off.
      if (gameplay[updatedButton->value])
      {
        ledOn(updatedButton->value);
      }
      else
      {
        ledOff(updatedButton->value);
      }
    }
  }
}

void MemoryBlink::initLcd()
{
  // Initialize the lcd
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Memory Blink!");
}

/* -------------------------------------------------------------------------- */
/*                                 GAME MODES                                 */
/* -------------------------------------------------------------------------- */

void MemoryBlink::handler()
{
  int inputSequencePos = inputSequenceLength - 1;

  /* ------------------------ Determine fail condition ------------------------ */

  bool didFail{false};
  if (gameplay[GAMEPLAY_RECALL_IDX]) // do a forward recall check
  {
    didFail = inputSequence[inputSequencePos] != generatedSequence[inputSequencePos];
  }
  else // do a backward recall check
  {
    didFail = inputSequence[inputSequencePos] != generatedSequence[generatedSequenceLength - inputSequenceLength];
  }

  /* ------------- Check if input failed the determined condition ------------- */
  if (didFail)
  {
    // update high score if it's been beaten
    if (level > eeprom_read_byte(&memStart))
    {
      eeprom_update_byte(&memStart, level);
    }

    return endGame();
  }

  // Check if the last sequence has been entered
  else if (inputSequenceLength == generatedSequenceLength)
  {
    // level up the game
    levelUp();

    if (gameplay[GAMEPLAY_SEQUENCE_IDX]) // i.e. plusone mode
    {
      // add a random position to the generated sequence
      addToGeneratedSequence();
    }
    else // i.e. shuffle mode
    {
      // reset the generated sequence
      clearGeneratedSequence();

      // add new set of random sequence plus offset
      addToGeneratedSequence(NUM_PADS + level);
    }
  }
}
