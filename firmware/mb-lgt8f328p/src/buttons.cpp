#include "main.h"

namespace
{
constexpr uint8_t EVENT_QUEUE_SIZE = 16;
ButtonEvent event_queue[EVENT_QUEUE_SIZE];
uint8_t queue_head = 0;
uint8_t queue_tail = 0;
uint8_t queue_count = 0;
bool game_input_enabled = false;
bool button_feedback_enabled = true;

void queue_event(uint8_t pad, ButtonEventType type)
{
  if (queue_count == EVENT_QUEUE_SIZE)
    return;

  event_queue[queue_tail] = {pad, type};
  queue_tail = (queue_tail + 1) % EVENT_QUEUE_SIZE;
  queue_count++;
}

void game_btn_pressed(Button2 &btn)
{
  uint8_t pad = static_cast<uint8_t>(btn.getID());
  if (button_feedback_enabled)
    pad_on(pad);
  if (game_input_enabled)
    queue_event(pad, BUTTON_PRESSED);
}

void game_btn_released(Button2 &btn)
{
  uint8_t pad = static_cast<uint8_t>(btn.getID());
  if (button_feedback_enabled)
    pad_off(pad);
  if (game_input_enabled)
    queue_event(pad, BUTTON_RELEASED);
}

void game_btn_clicked(Button2 &btn)
{
  uint8_t clicks = btn.getNumberOfClicks();
  while (clicks-- > 0)
    queue_event(static_cast<uint8_t>(btn.getID()), BUTTON_CLICKED);
}

}

void init_btns()
{
  for (int i = 0; i < NUM_PADS; i++)
  {
    buttons[i].begin(button_pins[i]);
    buttons[i].setID(i);
    buttons[i].setDebounceTime(30);
    buttons[i].setDoubleClickTime(200);
    buttons[i].setPressedHandler(game_btn_pressed);
    buttons[i].setReleasedHandler(game_btn_released);
    buttons[i].setClickHandler(game_btn_clicked);
    buttons[i].setDoubleClickHandler(game_btn_clicked);
    buttons[i].setTripleClickHandler(game_btn_clicked);
  }
}

void loop_btns()
{
  for (int i = 0; i < NUM_PADS; i++)
  {
    buttons[i].loop();
  }
}

bool pop_button_event(ButtonEvent &event)
{
  if (queue_count == 0)
    return false;

  event = event_queue[queue_head];
  queue_head = (queue_head + 1) % EVENT_QUEUE_SIZE;
  queue_count--;
  return true;
}

void clear_button_events()
{
  queue_head = 0;
  queue_tail = 0;
  queue_count = 0;
}

void set_game_input_enabled(bool enabled)
{
  game_input_enabled = enabled;
}

void set_button_feedback_enabled(bool enabled)
{
  button_feedback_enabled = enabled;
}