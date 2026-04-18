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
  buzzer.play(Note::NOTE_B0);

  delay(1000);

  buzzer.stop();

  delay(1000);
}

MemoryBlink::~MemoryBlink()
{
}

/* -------------------------------------------------------------------------- */
/*                                   PADS                                     */
/* -------------------------------------------------------------------------- */

void MemoryBlink::padOn(int pos)
{
  /* --------------------------------- lights --------------------------------- */
  if (bitRead(gameplay, 3))
  {
    digitalWrite(ledPins[pos], 1);
  }
  else
  {
    digitalWrite(plainLedPins[pos], 1);
  }

  /* ---------------------------------- sound --------------------------------- */
  if (bitRead(gameplay, 2))
  {
    buzzer.play(padNotes[pos]);
  }
}

void MemoryBlink::padOff(int pos)
{
  /* --------------------------------- lights --------------------------------- */
  if (bitRead(gameplay, 3))
  {
    digitalWrite(ledPins[pos], 0);
  }
  else
  {
    digitalWrite(plainLedPins[pos], 0);
  }

  /* ---------------------------------- sound --------------------------------- */
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
  // decrease the flash time and delay time every 10 levels
  if (score > 0 && !(score % (MAX_SEQUENCE / NUM_PADS)))
  {
    flashTime -= 100;
    delayTime -= 50;
  }

  for (int i = 0; i < generatedSequenceLength; i++)
  {
    debug.printf("yes?\n");
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
  if (score > highscore)
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

  // debug.printf("should get input: %i\n", shouldGetInput);

  buttonMan.resetButtonsState();
  while (shouldGetInput)
  {
    // check if a button was updated
    Button const *updatedButton = buttonMan.getUpdate();

    if (updatedButton != nullptr)
    {
      debug.printf("button => value: %i, state: %i\n", updatedButton->value, updatedButton->state);
      if (updatedButton->state == ButtonState::PRESSED)
      {
        // reset timeout timer
        scanTimer = millis();

        // turn on pad
        padOn(updatedButton->value);

        // update input sequence
        inputSequence[inputSequenceLength++] = updatedButton->value;
      }

      else if (updatedButton->state == ButtonState::PRESS_RELEASE)
      {
        debug.printf("\n\nis it you?\n\n");
        // turn off the pad on release
        padOff(updatedButton->value);

        //!
        // verify the pressed button
        checkInput();
      }
    }

    if (millis() - scanTimer > SCAN_TIMEOUT)
    {
      // getting input timed out
      endGame();
    }
  }
}

uint8_t MemoryBlink::getHighscore()
{
  uint8_t highscoreAddr = MEM_START + (player - 1) * bit(NUM_PADS) + gameplay;

  return EEPROM.read(highscoreAddr);
}

void MemoryBlink::setHighscore(uint8_t highscore)
{
  uint8_t highscoreAddr = MEM_START + (player - 1) * bit(NUM_PADS) + gameplay;

  EEPROM.write(highscoreAddr, highscore);
}

void MemoryBlink::gameLoop()
{
  // reset variables
  reinitialize();

  // start sequence from NUM_PADS to avoid trivial start
  addToGeneratedSequence(NUM_PADS);

  // print high score to the screen and delay
  drawTop();
  drawBottom();

  gameRunning = true;
  while (gameRunning)
  {
    // display sequence
    displayGeneratedSequence();

    printVars("before input");

    // get user input
    getInput();

    printVars("after input");
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
  gameplay = EEPROM.read(MEM_START - 1);

  buttonMan.resetButtonsState();
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
        EEPROM.update(MEM_START - 1, gameplay);
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
  player = EEPROM.read(MEM_START - 2);

  // choose gameplay options for the current game
  buttonMan.resetButtonsState();
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
      EEPROM.update(MEM_START - 2, player);
      return;
    }
  }
}

void MemoryBlink::drawBottom()
{
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

void MemoryBlink::startScreen()
{

  lcd.setCursor(0, 0);
  lcd.print("Memory Blink!");

  lcd.setCursor(0, 1);
  lcd.print("Press to start.");

  buttonMan.resetButtonsState();
  while (true)
  {
    // todo: find out a way to turn off the arduino after a while if the user doesn't press anything (or put in low power sleep)
    // todo: play "elevator music"

    Button const *updatedButton = buttonMan.getUpdate(ButtonState::PRESSED);
    if (updatedButton != nullptr)
    {
      // show a "loading animation"
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Ready Player ");
      lcd.print(player);
      lcd.print("  ");

      lcd.setCursor(0, 1);
      for (int i = 0; i < NUM_PADS; i++) // todo: play startup tune
      {
        delay(1000);
        lcd.print(".");
      }
      delay(1000);

      return;
    }
  }
}

void MemoryBlink::initLcd()
{
  // Initialize the lcd
  lcd.init();
  lcd.backlight();
}

void MemoryBlink::checkInput()
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

void MemoryBlink::reinitialize()
{
  // load last gameplay
  gameplay = EEPROM.read(MEM_START - 1);

  // load last player
  player = EEPROM.read(MEM_START - 2);

  flashTime = STARTING_FLASH_TIME, delayTime = STARTING_DELAY_TIME;
  highscore = getHighscore();

}

void MemoryBlink::printVars(const char *tag)
{
  debug.printf("TAG: %s\n", tag);
  debug.printf("======\n");

  debug.printf("flash time: %i\n", flashTime);
  debug.printf("delay time: %i\n", delayTime);
  debug.printf("scanTimer: %lu, millis: %lu\n", scanTimer, millis());

  debug.printf("game running: %i\n", gameRunning);
  debug.printf("should get input: %i\n", shouldGetInput);

  debug.printf("======\n\n\n\n");
}
