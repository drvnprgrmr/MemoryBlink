/* -------------------------------------------------------------------------- */
/*                                 MemoryBlink                                */
/* -------------------------------------------------------------------------- */

/** NOTES TO ME
 * - No wifi features
 * - No mixing and matching of modes (10 simon, 20 reverse, e.t.c.)
 */

/* ----------------------------- Screen Version ----------------------------- */
/** GAMEPLAY
 * - At initial boot up, there are two options: choose game mode, change settings
 * - After choosing a game mode, you'll be asked to choose the player (every player has their high scores for that game mode by the side; also, at the end of the list there'll be an option to add an extra player  up to 100 (might be extended to the max available based on memory after every other feature's been implemented))
 * - Every player plays a full turn and must end before another player can play.
 * - The game runs on until you lose or get to the end at 100 steps per sequence (unlikely though :) ).
 * - Your level is updated on a screen/segment display after each level.
 * - The game is by default color agnostic. You can swap colors (physically replace LEDs) or use a RGB led (must be set in the settings) and set the colors in the settings. [COMES LATER]
 */

/** SETTINGS
 * - time increase [=fixed, decreasing to limit] (50ms increments)
 * - play sounds
 * - tones for each key (from predefined tone list)
 *
 * > Player settings
 * - reset high score for player
 * - add player
 */

/* ------------------------- Segment Display Version ------------------------ */
/** GAMEPLAY
 * - tbd
 */

#include <Arduino.h>
#include <Bonezegei_Printf.h>

#define NUM_PADS 4
#define MAX_SEQUENCE 100

Bonezegei_Printf debug(&Serial);

constexpr int ledPins[NUM_PADS] = {2, 3, 4, 5};
constexpr int buttonPins[NUM_PADS] = {6, 7, 8, 9};

// A helper function that does nothing but enforce types
void enforceHelper(void (*f)(int)) {}

template <typename F>
void loopPads(F op)
{
  if (false)
    enforceHelper(op);

  for (int i = 0; i < NUM_PADS; i++)
  {
    op(i);
  }
}

/* -------------------------------------------------------------------------- */

int level = 1;

bool simonRunning = false;

uint32_t buttonTimeout = 5UL * 1000UL; // 5s per button input
uint32_t buttonTimeoutTimer = millis();
bool shouldScanButtons = true;

int generatedSequence[MAX_SEQUENCE]{};
int generatedSequenceLength = 0;

int inputSequence[MAX_SEQUENCE]{};
int inputSequenceLength = 0;

/* -------------------------------------------------------------------------- */
/*                                LED Functions                               */
/* -------------------------------------------------------------------------- */

void ledOn(int ledPos)
{
  digitalWrite(ledPins[ledPos], 1);
}

void ledOn()
{
  loopPads([](int i)
           { ledOn(i); });
}

void ledOff(int ledPos)
{
  digitalWrite(ledPins[ledPos], 0);
}

void ledOff()
{
  loopPads([](int i)
           { ledOff(i); });
}

int flashOnTime = 0;
void setFlashOnTime(int onTime)
{
  flashOnTime = onTime;
}

void flashLed()
{
  ledOn();
  delay(flashOnTime);
  ledOff();
}

void flashLed(int ledPos)
{
  ledOn(ledPos);
  delay(flashOnTime);
  ledOff(ledPos);
}

void addToGeneratedSequence()
{
  generatedSequence[generatedSequenceLength++] = random(NUM_PADS);
}

void addToGeneratedSequence(int count)
{
  if (generatedSequenceLength < MAX_SEQUENCE)
  {
    for (int i = 0; i < count; i++)
    {
      addToGeneratedSequence();
    }
  }
}

void clearGeneratedSequence()
{
  generatedSequenceLength = 0;
}

void clearInputSequence()
{
  inputSequenceLength = 0;
}

void clearSequences()
{
  clearInputSequence();
  clearGeneratedSequence();
}

void displayGeneratedSequence()
{
  for (int i = 0; i < generatedSequenceLength; i++)
  {
    int pos = generatedSequence[i];

    setFlashOnTime(500);
    flashLed(pos);

    delay(200);
  }
}

void levelUp()
{
  level++;
  debug.printf("Level Up!. New Level: %i\n", level);
}

void onSuccessSimon()
{
  // add a slight delay before displaying
  delay(500);

  // display flash sequence
  setFlashOnTime(1000);
  flashLed();

  // increase level
  levelUp();

  // add a random position
  addToGeneratedSequence();

  // clear input
  clearInputSequence();

  // stop scanning buttons
  shouldScanButtons = false;
}

void onFailureSimon()
{
  delay(500);

  // display flash sequence
  setFlashOnTime(475);
  flashLed();
  delay(50);
  flashLed();

  // reset level
  level = 1;
  debug.printf("You Lost!\n");

  // clear sequences
  clearGeneratedSequence();
  clearInputSequence();

  // stop scanning buttons
  shouldScanButtons = false;

  // end simon game
  simonRunning = false;
}

void onTimeout()
{
  debug.printf("Sorry, you timed out!\n");
  delay(1000);
  onFailureSimon();
}

/* -------------------------------------------------------------------------- */
/*                              Button Functions                              */
/* -------------------------------------------------------------------------- */
uint32_t debounceTime = 300;
uint32_t holdTime = 2 * 1000;

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

// initialize buttons
Button buttons[NUM_PADS]{};

// Classic Simon game
void onPressSimon()
{
  // Check if user enters the exact sequence that was described
  int inputSequencePos = inputSequenceLength - 1;
  debug.printf("Input pos: %i, Input seq: %i, Generated seq: %i\n", inputSequencePos, inputSequence[inputSequencePos], generatedSequence[inputSequencePos]);
  if (inputSequence[inputSequencePos] != generatedSequence[inputSequencePos])
  {
    return onFailureSimon();
  }

  // Check if the last sequence has been entered
  else if (inputSequenceLength == generatedSequenceLength)
  {
    return onSuccessSimon();
  }
}

template <typename F>
void getButtonInput(F onPress, F onTimeout)
{
  shouldScanButtons = true;
  buttonTimeoutTimer = millis();

  auto buttonDidTimeout{
      []() -> bool
      {
        return millis() - buttonTimeoutTimer > buttonTimeout;
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
          debug.printf("Button pressed: %i\n", i);
          button.holdTimer = millis();

          // Reset timeout timer
          buttonTimeoutTimer = millis();

          // Flash LED on
          setFlashOnTime(200);
          flashLed(i);

          // update input sequence
          inputSequence[inputSequenceLength++] = i;

          // call the passed in function
          onPress();
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
    onTimeout();
  }
}

void simonLoop()
{
  addToGeneratedSequence(4); // start with 4 blinks

  simonRunning = true;
  while (simonRunning)
  {
    debug.printf("Starting level %i\n", level);

    // pause before each level
    delay(3 * 1000);

    displayGeneratedSequence();
    getButtonInput(onPressSimon, onTimeout);
  }

  debug.printf("===========================\n\n");
}

void setup()
{
  Serial.begin(9600);

  Serial.println("MemoryBlink starting up!");
  debug.printf("Just checking...\n");

  // Initialize pins
  for (int i = 0; i < NUM_PADS; i++)
  {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  // Seed RNG (if A0 is floating it helps vary the sequence)
  randomSeed(analogRead(A0));

  // Startup blink
  for (int i = 0; i < NUM_PADS; i++)
  {
    ledOn();
    delay(300);
    ledOff();
    delay(300);
  }
}

void loop()
{
  simonLoop();
}
