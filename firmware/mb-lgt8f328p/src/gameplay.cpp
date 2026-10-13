#include "main.h"

#include <stdint.h>

const Note padNotes[NUM_PADS] = {Note::NOTE_E4, Note::NOTE_CS4, Note::NOTE_A4, Note::NOTE_E3};
const MelodyStep successMelody[NUM_PADS] = {
    {Note::NOTE_C5, 100},
    {Note::NOTE_E5, 100},
    {Note::NOTE_G5, 100},
    {Note::NOTE_C6, 250}};
const MelodyStep failureMelody[NUM_PADS] = {
    {Note::NOTE_G3, 300},
    {Note::NOTE_FS3, 300},
    {Note::NOTE_F3, 300},
    {Note::NOTE_E3, 600}};

namespace
{
uint8_t generated_sequence[MAX_SEQUENCE_LENGTH]{};
uint8_t generated_sequence_length = 0;
uint16_t flash_time = 500;
uint16_t delay_time = 125;
uint16_t input_timeout = 5000;
uint8_t stored_highscore = 0;

void wait_with_button_polling(uint32_t duration)
{
  uint32_t started_at = millis();
  while (millis() - started_at < duration)
  {
    loop_btns();
    delay(2);
  }
}

void add_to_sequence(uint8_t count)
{
  while (count-- > 0 && generated_sequence_length < MAX_SEQUENCE_LENGTH)
    generated_sequence[generated_sequence_length++] = random(NUM_PADS);
}

void display_sequence()
{
  set_game_input_enabled(false);
  set_button_feedback_enabled(false);
  show_game_status();
  show_watch_prompt();

  for (uint8_t i = 0; i < generated_sequence_length; i++)
  {
    uint8_t pad = generated_sequence[i];
    pad_on(pad);
    wait_with_button_polling(flash_time);
    pad_off(pad);
    wait_with_button_polling(delay_time);
  }

  clear_button_events();
  set_button_feedback_enabled(true);
  show_input_prompt();
}

bool collect_input(uint32_t &completed_at)
{
  uint8_t input_position = 0;
  uint8_t pressed_pad = 0;
  bool awaiting_release = false;
  uint32_t last_input_at = millis();

  clear_button_events();
  set_button_feedback_enabled(true);
  set_game_input_enabled(true);

  while (millis() - last_input_at < input_timeout)
  {
    loop_btns();
    ButtonEvent event;
    while (pop_button_event(event))
    {
      if (event.type == BUTTON_PRESSED)
      {
        pressed_pad = event.pad;
        awaiting_release = true;
        last_input_at = millis();
      }
      else if (event.type == BUTTON_RELEASED && awaiting_release && event.pad == pressed_pad)
      {
        awaiting_release = false;
        uint8_t sequence_position = (game_mode & MODE_FORWARD_RECALL)
                                        ? input_position
                                        : generated_sequence_length - input_position - 1;
        if (pressed_pad != generated_sequence[sequence_position])
        {
          set_game_input_enabled(false);
          return false;
        }

        input_position++;
        last_input_at = millis();
        if (input_position == generated_sequence_length)
        {
          completed_at = millis();
          set_game_input_enabled(false);
          return true;
        }
      }
    }

    delay(2);
  }

  set_game_input_enabled(false);
  return false;
}

void update_pace()
{
  // Each ten-level tier shortens flashes and input timeouts together.
  uint8_t tier = score / 10;
  flash_time = 500 - tier * 100;
  delay_time = 125 - tier * 25;
  input_timeout = 5000 - tier * 1000;
}

void wait_for_next_level(uint32_t completed_at)
{
  // Include the success cue in the four-second interval between rounds.
  set_game_input_enabled(false);
  set_button_feedback_enabled(false);
  uint32_t elapsed = millis() - completed_at;
  if (elapsed < LEVEL_INTERVAL_MS)
    wait_with_button_polling(LEVEL_INTERVAL_MS - elapsed);
  clear_button_events();
  set_button_feedback_enabled(true);
}

void turn_off_all_pads()
{
  for (uint8_t pad = 0; pad < NUM_PADS; pad++)
  {
    ledOff(pad, LED_COLORED);
    ledOff(pad, LED_PLAIN);
  }
}

void finish_game(bool won)
{
  set_game_input_enabled(false);
  set_button_feedback_enabled(false);
  buzzer.stop();
  turn_off_all_pads();

  if (score > stored_highscore)
  {
    highscore = score;
    // Commit the best score once per run; LGT EEPROM emulation page-swaps on writes.
    save_highscore(highscore);
    stored_highscore = highscore;
  }

  show_game_result(won);
  if (!padSettings.soundOff)
    buzzer.playMelody(won ? successMelody : failureMelody, NUM_PADS);

  wait_with_button_polling(1500);
  clear_button_events();
  set_button_feedback_enabled(true);
}
}

void pad_on(int pos)
{
  if (pos < 0 || pos >= NUM_PADS)
    return;

  ledOn(pos, padSettings.ledType);
  if (!padSettings.soundOff)
  {
    buzzer.play(padNotes[pos]);
  }
}

void pad_off(int pos)
{
  if (pos < 0 || pos >= NUM_PADS)
    return;

  ledOff(pos, padSettings.ledType);
  if (!padSettings.soundOff)
    buzzer.stop();
}

void testLeds()
{
  for (int i = 0; i < NUM_PADS; i++)
  {
    ledOn(i, LED_COLORED);
    buzzer.play(Note::NOTE_C4);
    delay(500);
    ledOff(i, LED_COLORED);
    buzzer.stop();
  }

  for (int i = 0; i < NUM_PADS; i++)
  {
    ledOn(i, LED_PLAIN);
    buzzer.play(Note::NOTE_D4);
    delay(500);
    ledOff(i, LED_PLAIN);
    buzzer.stop();
  }
}

void play_game()
{
  apply_game_mode();
  score = 0;
  stored_highscore = load_highscore();
  highscore = stored_highscore;
  generated_sequence_length = 0;

  randomSeed(static_cast<uint32_t>(analogRead(A0)) ^ micros());
  add_to_sequence(NUM_PADS);

  while (score < MAX_LEVELS)
  {
    update_pace();
    display_sequence();

    uint32_t completed_at = 0;
    if (!collect_input(completed_at))
    {
      finish_game(false);
      return;
    }

    score++;
    show_game_status();
    if (score > highscore)
      highscore = score;

    if (score == MAX_LEVELS)
    {
      finish_game(true);
      return;
    }

    if (!padSettings.soundOff)
      buzzer.playMelody(successMelody, NUM_PADS);

    if (game_mode & MODE_APPEND_SEQUENCE)
    {
      add_to_sequence(1);
    }
    else
    {
      generated_sequence_length = 0;
      add_to_sequence(NUM_PADS + score);
    }

    wait_for_next_level(completed_at);
  }
}