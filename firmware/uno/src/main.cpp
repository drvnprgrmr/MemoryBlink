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

int flashOnTime = 180;
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

void addToSequence()
{
  generatedSequence[generatedSequenceLength++] = random(NUM_PADS);
}

void addToSequence(int count)
{
  if (generatedSequenceLength < MAX_SEQUENCE)
  {
    for (int i = 0; i < count; i++)
    {
      addToSequence();
    }
  }
}

void clearSequence()
{
  generatedSequenceLength = 0;
}

void displaySequence()
{
  for (int i = 0; i < generatedSequenceLength; i++)
  {
    int pos = generatedSequence[i];
    ledOn(pos);
    delay(500);

    ledOff(pos);
    delay(200);
  }
}

void levelUp()
{
  level++;
  debug.printf("Level Up!. New Level: %i\n", level);
}

void onSuccess()
{
  // display flash sequence
  flashLed(1000);

  // increase level
  level++;
  debug.printf("Level Up!. New Level: %i\n", level);

  // add a random position
  addToSequence();

  // stop scanning buttons
  shouldScanButtons = false;
}

void onFailure()
{
  // display flash sequence
  flashLed(475);
  delay(50);
  flashLed(475);

  // reset level
  level = 1;
  debug.printf("You Lost!\n");

  // clear sequence
  clearSequence();

  // stop scanning buttons
  shouldScanButtons = false;

  // end simon game
  simonRunning = false;
}

void onTimeout()
{
  debug.printf("Sorry, you timed out!");
  delay(500);
  onFailure();
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
    onFailure();
  }

  // Check if the last sequence has been entered
  if (inputSequenceLength == generatedSequenceLength)
  {
    onSuccess();
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

  debug.printf("Did timeout? %i\n", (bool)buttonDidTimeout());
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
  addToSequence(4); // start with 4 blinks

  simonRunning = true;
  while (simonRunning)
  {
    debug.printf("Starting level %i\n", level);

    // pause before each level
    delay(3 * 1000);

    displaySequence();
    getButtonInput(onPressSimon, onTimeout);
  }
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
