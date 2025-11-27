#include <Arduino.h>
#include <Bonezegei_Printf.h>
#include "buzzer.h"

#define NUM_PADS 4
#define MAX_SEQUENCE 100

Bonezegei_Printf debug(&Serial);

constexpr int ledPins[NUM_PADS]{2, 3, 4, 5};
constexpr int buttonPins[NUM_PADS]{6, 7, 8, 9};
Buzzer buzzer{10};
constexpr Note padNotes[NUM_PADS]{Note::NOTE_E4, Note::NOTE_CS4, Note::NOTE_A4, Note::NOTE_E3};

int level = 1;

bool simonRunning = false;

uint32_t buttonTimeout = 5UL * 1000UL; // 5s per button input
uint32_t buttonTimeoutTimer = millis();
bool shouldScanButtons = true;

int generatedSequence[MAX_SEQUENCE]{};
int generatedSequenceLength = 0;

int inputSequence[MAX_SEQUENCE]{};
int inputSequenceLength = 0;

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

void ledOn(int pos)
{
  digitalWrite(ledPins[pos], 1);
}

void ledOn()
{
  loopPads([](int i)
           { ledOn(i); });
}

void ledOff(int pos)
{
  digitalWrite(ledPins[pos], 0);
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

void flashLed(int pos)
{
  ledOn(pos);
  delay(flashOnTime);
  ledOff(pos);
}

void padOn(int pos)
{
  ledOn(pos);
  buzzer.play(padNotes[pos]);
}

void padOff(int pos)
{
  ledOff(pos);
  buzzer.stop();
}

/* -------------------------------------------------------------------------- */

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

    padOn(pos);
    delay(500);
    padOff(pos);

    delay(200);
  }
}

/* -------------------------------------------------------------------------- */

void levelUp()
{
  level++;
  debug.printf("Level Up!. New Level: %i\n", level);
}

void onSuccessSimon()
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

void onFailureSimon()
{
  delay(500);

  // play success melody
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

          // blink pad
          padOn(i);
          delay(200);
          padOff(i);

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
  ledOn();
  buzzer.play(Note::NOTE_B0);

  delay(1000);

  ledOff();
  buzzer.stop();
  
  delay(1000);
}

void loop()
{
  simonLoop();
}
