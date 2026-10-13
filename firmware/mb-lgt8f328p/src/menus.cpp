#include "main.h"

namespace
{
constexpr uint8_t mode_bits[NUM_PADS] = {
    MODE_FORWARD_RECALL,
    MODE_APPEND_SEQUENCE,
    MODE_PLAIN_LEDS,
    MODE_SOUND};

uint8_t previous_game_mode = DEFAULT_GAME_MODE;
bool has_previous_settings = false;

uint8_t highscore_address()
{
  // One byte per mode and player; scores 0-40 fit without packing or extra writes.
  return EEPROM_HIGHSCORE_BASE + (selected_player - 1) * 16 + game_mode;
}

void save_game_mode()
{
  EEPROM.update(EEPROM_MODE_ADDRESS, game_mode);
  EEPROM.update(EEPROM_MODE_VERSION_ADDRESS, EEPROM_MODE_VERSION);
  previous_game_mode = game_mode;
  has_previous_settings = true;
}
}

void apply_game_mode()
{
  padSettings.ledType = (game_mode & MODE_PLAIN_LEDS) ? LED_PLAIN : LED_COLORED;
  padSettings.soundOff = (game_mode & MODE_SOUND) == 0;
}

void load_game_settings()
{
  uint8_t stored_mode = EEPROM.read(EEPROM_MODE_ADDRESS);
  uint8_t stored_version = EEPROM.read(EEPROM_MODE_VERSION_ADDRESS);
  has_previous_settings = stored_mode <= 0x0F;

  if (has_previous_settings && stored_version != EEPROM_MODE_VERSION)
  {
    // Older LGT builds used this bit to mean colored LEDs instead of plain LEDs.
    stored_mode ^= MODE_PLAIN_LEDS;
    EEPROM.update(EEPROM_MODE_ADDRESS, stored_mode);
    EEPROM.update(EEPROM_MODE_VERSION_ADDRESS, EEPROM_MODE_VERSION);
  }

  previous_game_mode = has_previous_settings ? stored_mode : DEFAULT_GAME_MODE;
  game_mode = previous_game_mode;

  uint8_t stored_player = EEPROM.read(EEPROM_PLAYER_ADDRESS);
  selected_player = stored_player >= 1 && stored_player <= NUM_PADS ? stored_player : 1;

  apply_game_mode();
  highscore = load_highscore();
}

uint8_t load_highscore()
{
  uint8_t stored = EEPROM.read(highscore_address());
  return stored == 0xFF ? 0 : stored;
}

void save_highscore(uint8_t value)
{
  EEPROM.update(highscore_address(), value);
}

void configure_game_mode()
{
  game_mode = has_previous_settings ? previous_game_mode : DEFAULT_GAME_MODE;
  apply_game_mode();
  uint8_t selection = 0;
  show_mode_menu(selection);
  clear_button_events();

  while (true)
  {
    loop_btns();
    ButtonEvent event;
    while (pop_button_event(event))
    {
      if (event.type != BUTTON_CLICKED)
        continue;

      if (event.pad == 0)
      {
        selection = (selection + NUM_PADS - 1) % NUM_PADS;
        show_mode_menu(selection);
      }
      else if (event.pad == 1)
      {
        selection = (selection + 1) % NUM_PADS;
        show_mode_menu(selection);
      }
      else if (event.pad == 2)
      {
        game_mode ^= mode_bits[selection];
        apply_game_mode();
        show_mode_menu(selection);
      }
      else if (event.pad == 3)
      {
        save_game_mode();
        clear_button_events();
        show_settings_saved();
        delay(1200);
        return;
      }
    }
    delay(2);
  }
}

bool select_player()
{
  uint8_t selection = selected_player - 1;
  highscore = load_highscore();
  show_player_selection();
  clear_button_events();

  while (true)
  {
    loop_btns();
    ButtonEvent event;
    while (pop_button_event(event))
    {
      if (event.type != BUTTON_CLICKED)
        continue;

      if (event.pad == 0)
      {
        selection = (selection + NUM_PADS - 1) % NUM_PADS;
      }
      else if (event.pad == 1)
      {
        selection = (selection + 1) % NUM_PADS;
      }
      else if (event.pad == 2)
      {
        selected_player = selection + 1;
        EEPROM.update(EEPROM_PLAYER_ADDRESS, selected_player);
        highscore = load_highscore();
        clear_button_events();
        return true;
      }
      else if (event.pad == 3)
      {
        clear_button_events();
        return false;
      }

      selected_player = selection + 1;
      highscore = load_highscore();
      show_player_selection();
    }
    delay(2);
  }
}

void run_main_menu()
{
  uint8_t selection = 0;
  bool idle_screen = false;
  show_main_menu(selection, has_previous_settings);

  while (true)
  {
    loop_btns();
    ButtonEvent event;
    if (!pop_button_event(event))
    {
      delay(2);
      continue;
    }
    if (event.type != BUTTON_CLICKED)
      continue;

    if (idle_screen)
    {
      idle_screen = false;
      clear_button_events();
      show_main_menu(selection, has_previous_settings);
      continue;
    }

    if (event.pad == 0)
    {
      selection = (selection + 2) % 3;
      show_main_menu(selection, has_previous_settings);
      continue;
    }
    if (event.pad == 1)
    {
      selection = (selection + 1) % 3;
      show_main_menu(selection, has_previous_settings);
      continue;
    }
    if (event.pad == 3)
    {
      idle_screen = true;
      show_idle_screen();
      continue;
    }
    if (event.pad != 2)
      continue;

    clear_button_events();
    if (selection == 0)
    {
      game_mode = DEFAULT_GAME_MODE;
      apply_game_mode();
      if (select_player())
        play_game();
    }
    else if (selection == 1)
    {
      if (has_previous_settings)
      {
        game_mode = previous_game_mode;
        apply_game_mode();
        if (select_player())
          play_game();
      }
      else
      {
        show_no_previous_settings();
        delay(1200);
      }
    }
    else
    {
      configure_game_mode();
    }
    clear_button_events();
    show_main_menu(selection, has_previous_settings);
  }
}