#include <Arduino.h>
#include <Bonezegei_Printf.h>
#include "buzzer.h"

#define NUM_PADS 4
#define MAX_SEQUENCE 100

Bonezegei_Printf debug(&Serial);

constexpr uint8_t ledPins[NUM_PADS]{2, 3, 4, 5};
constexpr uint8_t buttonPins[NUM_PADS]{6, 7, 8, 9};
constexpr uint8_t buzzerPin = 10;

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

enum class LevelEnd
{
  PASS,
  MISS,
  TIMEOUT,
};

enum class GameMode
{
  CLASSIC,
  REVERSE,
  SHUFFLE,
};

class MemoryBlink
{

private:
  Buzzer buzzer;
  Button buttons[NUM_PADS]{};

  int level{1};

  int generatedSequence[MAX_SEQUENCE]{};
  int generatedSequenceLength = 0;

  int inputSequence[MAX_SEQUENCE]{};
  int inputSequenceLength = 0;

  bool gameRunning = false;

  bool shouldScanButtons{false};

  uint32_t debounceTime{300}, holdTime{2 * 1000};
  uint32_t scanTimer{0}, scanTimeout{5 * 1000};

  // Game mode generic handlers
  using GameModeOnPressHandler = void (MemoryBlink::*)();
  using GameModeOnTimeoutHandler = void (MemoryBlink::*)();
  using GameModeSetup = void (MemoryBlink::*)();

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
  void getButtonInput(GameModeOnPressHandler onPress, GameModeOnTimeoutHandler onTimeout);
  void gameLoop(GameModeSetup _setup, GameModeOnPressHandler onPress, GameModeOnTimeoutHandler onTimeout);

private:
  void onSuccessSimon();
  void onFailureSimon();
  void onTimeoutSimon();
  void setupSimon();
  void onPressSimon();

public:
  MemoryBlink(uint8_t const ledPins[NUM_PADS], uint8_t const buttonPins[NUM_PADS], uint8_t const buzzerPin);
  ~MemoryBlink();

public:
  void gameLoop(GameMode mode);
};

MemoryBlink::MemoryBlink(uint8_t const ledPins[NUM_PADS], uint8_t const buttonPins[NUM_PADS], uint8_t const buzzerPin)
    : buzzer(buzzerPin)
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

void MemoryBlink::levelUp()
{
  level++;
  debug.printf("Level Up!. New Level: %i\n", level);
}

void MemoryBlink::getButtonInput(GameModeOnPressHandler onPress, GameModeOnTimeoutHandler onTimeout)
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
          debug.printf("Button pressed: %i\n", i);
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
    onTimeout != nullptr ? (this->*onTimeout)() : debug.printf("WARN: onTimeout not defined!\n");
  }
}

/* ----------------------------- Simon Handlers ----------------------------- */

void MemoryBlink::setupSimon()
{
  addToGeneratedSequence(4); // start with 4 blinks
}

void MemoryBlink::onSuccessSimon()
{
  // add a slight delay before displaying
  delay(500);

  // play success melody
  buzzer.playMelody(successMelody, successLen);

  // increase level
  levelUp();

  // add a random position
  addToGeneratedSequence();

  // clear input
  clearInputSequence();

  // stop scanning buttons
  shouldScanButtons = false;
}

void MemoryBlink::onFailureSimon()
{
  delay(500);

  // play failure melody
  buzzer.playMelody(failureMelody, failureLen);

  // reset level
  level = 1;
  debug.printf("You Lost!\n");

  // clear sequences
  clearGeneratedSequence();
  clearInputSequence();

  // stop scanning buttons
  shouldScanButtons = false;

  // end simon game
  gameRunning = false;
}

void MemoryBlink::onTimeoutSimon()
{
  debug.printf("Sorry, you timed out!\n");
  delay(1000);
  onFailureSimon();
}

void MemoryBlink::onPressSimon()
{
  // Check if user enters the exact sequence that was described
  int inputSequencePos = inputSequenceLength - 1;
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

void MemoryBlink::gameLoop(GameMode mode)
{
  switch (mode)
  {

  case GameMode::CLASSIC:
  {
    // start the classic simon game mode
    gameLoop(&MemoryBlink::setupSimon, &MemoryBlink::onPressSimon, &MemoryBlink::onTimeoutSimon);

    break;
  }

  default:
    break;
  }
}

void MemoryBlink::gameLoop(GameModeSetup _setup, GameModeOnPressHandler onPress, GameModeOnTimeoutHandler onTimeout)
{
  (this->*_setup)();

  gameRunning = true;
  while (gameRunning)
  {
    debug.printf("Starting level %i\n", level);

    // pause before each level
    delay(3 * 1000);

    displayGeneratedSequence();
    getButtonInput(onPress, onTimeout);
  }

  debug.printf("===========================\n\n");
}

void setup()
{
  Serial.begin(9600);
  Serial.println("MemoryBlink starting up!");
}

void loop()
{
  MemoryBlink game{ledPins, buttonPins, buzzerPin};

  game.gameLoop(GameMode::CLASSIC);
}
