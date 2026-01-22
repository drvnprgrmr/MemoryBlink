#pragma once

#include <Arduino.h>

enum class ButtonState
{
  IDLE,
  PRESSED,
  PRESS_RELEASE,
  HOLD_RELEASE,
  HELD
};

struct Button
{
  ButtonState state{ButtonState::IDLE};
  uint32_t idleTimer{0};
  uint32_t holdTimer{0};
  uint8_t value;

  // add initializer for the value
  Button(uint8_t v) : value(v) {};

  // if none is given don't set the value
  Button() = default;
};

template <uint8_t count>
class ButtonMan
{
private:
  const uint8_t *pins;
  Button buttons[count];

  uint32_t debounceTime{300}, idleTime{5 * 1000}, holdTime{2 * 1000};
  uint32_t scanTimer{0};

  Button const *updatedButtons[count];
  uint8_t updatedButtonsCount{0};

private:
  void initializePins()
  {
    // Initialize the pins
    for (uint8_t i = 0; i < count; i++)
    {
      pinMode(pins[i], INPUT_PULLUP);
    }
  }

public:
  ButtonMan(uint8_t const pins[count]) : pins(pins), buttons()
  {

    initializePins();

    // set default values for the buttons
    for (uint8_t i = 0; i < count; i++)
    {
      Button *button = &buttons[i];
      button->value = i;
    }
  };
  ButtonMan(uint8_t const pins[count], uint8_t values[count]) : pins(pins), buttons(values)
  {
    initializePins();
  };

  ~ButtonMan() {
  };

  void scanButtons()
  {
    // reset updated buttons count
    updatedButtonsCount = 0;

    for (int i = 0; i < count; i++)
    {
      int level = digitalRead(pins[i]);
      Button *button = &buttons[i];

      bool updated = false;

      if (level == 0) // i.e. button has been pulled down
      {
        if ((button->state == ButtonState::PRESS_RELEASE || button->state == ButtonState::HOLD_RELEASE || button->state == ButtonState::IDLE) &&
            (millis() - button->holdTimer > debounceTime))
        {
          // start hold timer
          button->holdTimer = millis();

          button->state = ButtonState::PRESSED;
          updated = true;
        }

        else if (button->state == ButtonState::PRESSED &&
                 (millis() - button->holdTimer > holdTime))
        {
          button->state = ButtonState::HELD;
          updated = true;
        }
      }
      else
      {
        if ((button->state == ButtonState::PRESSED || button->state == ButtonState::HELD) &&
            (millis() - button->idleTimer > debounceTime))
        {
          // start idle timer
          button->idleTimer = millis();

          button->state = (button->state == ButtonState::PRESSED) ? ButtonState::PRESS_RELEASE : ButtonState::HOLD_RELEASE;
          updated = true;
        }
        else if (button->state != ButtonState::IDLE &&
                 (millis() - button->idleTimer > idleTime))
        {
          button->state = ButtonState::IDLE;
          updated = true;
        }
      }

      // check if a button was updated
      if (updated)
      {
        // add this button to the list of updated buttons
        updatedButtons[updatedButtonsCount++] = button;
      }
    }
  
  
  }

  // get a specific state update
  const Button *getUpdate(ButtonState state)
  {
    scanButtons(); // scan the buttons

    if (updatedButtonsCount)
    {
      Button const *updatedButton = updatedButtons[0]; // just take first input

      if (updatedButton->state == state)
      {
        return updatedButton;
      }
    }
    return nullptr;
  }

  // get any updated state
  const Button *getUpdate()
  {
    scanButtons(); // scan the buttons

    if (updatedButtonsCount)
    {
      Button const *updatedButton = updatedButtons[0]; // just take first input

      return updatedButton;
    }

    return nullptr;
  }
};
