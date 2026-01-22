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
  if (bitRead(gameplay, 3))
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
  if (bitRead(gameplay, 3))
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
  if (bitRead(gameplay, 2))
  {
    buzzer.play(padNotes[pos]);
  }
}

void MemoryBlink::padOff(int pos)
{
  ledOff(pos);
  if (bitRead(gameplay, 2))
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
  score++;

  // update level on screen
  drawBottom();

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

  // update high score if it's been beaten
  if (score > getHighscore())
  {
    setHighscore(score);
  }

  // reset level
  score = 0;

  // reset the screen state
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Try again!");

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

uint8_t MemoryBlink::getHighscore()
{
  uint8_t highscoreAddr = memStart + (player - 1) * bit(NUM_PADS) + gameplay;

  return EEPROM.read(highscoreAddr);
}

void MemoryBlink::setHighscore(uint8_t highscore)
{
  uint8_t highscoreAddr = memStart + (player - 1) * bit(NUM_PADS) + gameplay;

  EEPROM.write(highscoreAddr, highscore);
}

void MemoryBlink::gameLoop()
{

  uint8_t highscore = getHighscore();

  // add to the sequence
  addToGeneratedSequence();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Ready Player ");
  lcd.print(player);
  lcd.print("  ");

  delay(4000);

  // print high score to the screen and delay
  drawTop();
  drawBottom();

  gameRunning = true;
  while (gameRunning)
  {
    // display sequence
    displayGeneratedSequence();

    // get user input
    getInput();
  }
}

void MemoryBlink::gameplaySetup()
{
  lcd.clear();
  delay(1000);

  // update the screen
  drawTop();
  drawBottom("Gameplay Setup  ");

  // load last gameplay
  gameplay = EEPROM.read(memStart - 1);

  // choose gameplay options for the current game
  while (true)
  {
    Button const *updatedButton = buttonMan.getUpdate();

    if (updatedButton != nullptr)
    {
      if (updatedButton->state == ButtonState::PRESS_RELEASE)
      {
        // toggle that particular gameplay setting
        gameplay ^= gameplayMasks[updatedButton->value];

        // update the screen
        drawTop();
      }

      // exit setup on any button hold
      else if (updatedButton->state == ButtonState::HELD)
      {
        // save last gameplay
        EEPROM.update(memStart - 1, gameplay);
        return;
      }
    }
  }
}

void MemoryBlink::drawTop()
{
  lcd.setCursor(0, 0);
  lcd.print("Player ");
  lcd.print(player);
  lcd.print(" (G");

  lcd.print(bitRead(gameplay, 0));
  lcd.print(bitRead(gameplay, 1));
  lcd.print(bitRead(gameplay, 2));
  lcd.print(bitRead(gameplay, 3));

  lcd.print(")");
}

void MemoryBlink::playerSetup()
{
  lcd.clear();
  delay(1000);

  // update the screen
  drawTop();
  drawBottom("Select a Player.");

  // load last player
  player = EEPROM.read(memStart - 2);

  // choose gameplay options for the current game
  while (true)
  {
    Button const *updatedButton = buttonMan.getUpdate();

    if (updatedButton->state == ButtonState::PRESS_RELEASE)
    {
      // set the player for the current game
      player = updatedButton->value + 1;

      // update the screen
      drawTop();
    }

    // exit setup on any button hold
    else if (updatedButton->state == ButtonState::HELD)
    {
      // save last player
      EEPROM.update(memStart - 2, player);
      return;
    }
  }
}

void MemoryBlink::drawBottom()
{
  uint8_t highscore = getHighscore();

  lcd.setCursor(0, 1);
  lcd.print("Score:");
  if (score < 10)
  {
    lcd.print("0");
  }
  lcd.print(score);

  lcd.setCursor(9, 1);
  lcd.print("(HS:");
  if (highscore < 10)
  {
    lcd.print("0");
  }
  lcd.print(highscore);
  lcd.print(")");
}

void MemoryBlink::drawBottom(const char *displayText)
{
  lcd.setCursor(0, 1);
  lcd.print(displayText);
}


void MemoryBlink::initLcd()
{
  // Initialize the lcd
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Memory Blink!");

  lcd.setCursor(0, 1);
  lcd.print("Press to start.");

  while (true)
  {
    // todo: find out a way to turn off the arduino after a while if the user doesn't press anything (or put in low power sleep)
    Button const *updatedButton = buttonMan.getUpdate(ButtonState::PRESSED);
    if (updatedButton != nullptr)
    {
      return;
    }
  }
}

/* -------------------------------------------------------------------------- */
/*                                 GAME MODES                                 */
/* -------------------------------------------------------------------------- */

void MemoryBlink::handler()
{
  int inputSequencePos = inputSequenceLength - 1;

  /* ------------------------ Determine fail condition ------------------------ */

  bool didFail{false};
  if (bitRead(gameplay, 1)) // do a forward recall check
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
    return endGame();
  }

  // Check if the last sequence has been entered
  else if (inputSequenceLength == generatedSequenceLength)
  {
    // level up the game
    levelUp();

    if (bitRead(gameplay, 0)) // i.e. plusone mode
    {
      // add a random position to the generated sequence
      addToGeneratedSequence();
    }
    else // i.e. shuffle mode
    {
      // reset the generated sequence
      clearGeneratedSequence();

      // add new set of random sequence plus offset
      addToGeneratedSequence(NUM_PADS + score);
    }
  }
}
