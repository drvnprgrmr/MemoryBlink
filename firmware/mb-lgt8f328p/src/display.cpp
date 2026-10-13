#include "main.h"

#include <stdio.h>

namespace
{
constexpr uint8_t LCD_COLUMNS = 16;

void write_row(uint8_t row, const char *text)
{
  if (!lcd_ready)
  {
    Serial.println(text);
    return;
  }

  lcd.setCursor(0, row);
  uint8_t column = 0;
  while (text[column] != '\0' && column < LCD_COLUMNS)
    lcd.print(text[column++]);
  while (column++ < LCD_COLUMNS)
    lcd.print(' ');
}

void format_mode(char *text, size_t size)
{
  // Show zero for each documented default and one for its alternative.
  snprintf(text, size, "%u%u%u%u",
           (game_mode & MODE_FORWARD_RECALL) ? 0U : 1U,
           (game_mode & MODE_APPEND_SEQUENCE) ? 0U : 1U,
           (game_mode & MODE_PLAIN_LEDS) ? 1U : 0U,
           (game_mode & MODE_SOUND) ? 0U : 1U);
}
}

bool init_display()
{
  if (!lcd.begin(LCD_COLUMNS, 2))
  {
    Serial.println("LCD not detected; using serial output.");
    return false;
  }

  lcd.backlight();
  lcd.clear();
  return true;
}

void show_main_menu(uint8_t selection, bool has_previous_settings)
{
  static const char *items[] = {"New Game", "Continue", "Settings"};
  uint8_t first_item = selection == 2 ? 1 : 0;
  uint8_t second_item = selection == 2 ? 2 : 1;
  char first_row[17];
  char second_row[17];

  snprintf(first_row, sizeof(first_row), "%c%s%s", selection == first_item ? '>' : ' ',
           items[first_item], first_item == 1 && !has_previous_settings ? " (none)" : "");
  snprintf(second_row, sizeof(second_row), "%c%s%s", selection == second_item ? '>' : ' ',
           items[second_item], second_item == 1 && !has_previous_settings ? " (none)" : "");
  write_row(0, first_row);
  write_row(1, second_row);
}

void show_idle_screen()
{
  write_row(0, "Memory Blink!");
  write_row(1, "Press any key");
}

void show_settings_saved()
{
  write_row(0, "Settings saved");
  write_row(1, "Returning...");
}

void show_mode_menu(uint8_t selection)
{
  static const char *names[NUM_PADS] = {"Recall", "Sequence", "LED type", "Sound"};
  static const char *default_values[NUM_PADS] = {"Forward", "Append", "Colored", "On"};
  static const char *alternative_values[NUM_PADS] = {"Backward", "Refresh", "Plain", "Quiet"};
  const uint8_t bits[NUM_PADS] = {
      MODE_FORWARD_RECALL,
      MODE_APPEND_SEQUENCE,
      MODE_PLAIN_LEDS,
      MODE_SOUND};

  const char *value = (game_mode & bits[selection]) ? alternative_values[selection] : default_values[selection];
  char title[17];
  char setting[17];
  snprintf(title, sizeof(title), "Settings %u/4", static_cast<unsigned>(selection + 1));
  snprintf(setting, sizeof(setting), "%s: %s", names[selection], value);
  write_row(0, title);
  write_row(1, setting);
}

void show_player_selection()
{
  char title[17];
  char status[17];
  snprintf(title, sizeof(title), "Player %u", static_cast<unsigned>(selected_player));
  snprintf(status, sizeof(status), "High score: %02u", static_cast<unsigned>(highscore));
  write_row(0, title);
  write_row(1, status);
}

void show_game_status()
{
  char mode[5];
  char title[17];
  char status[17];
  format_mode(mode, sizeof(mode));
  snprintf(title, sizeof(title), "Player %u (G%s)", static_cast<unsigned>(selected_player), mode);
  snprintf(status, sizeof(status), "Score:%02u (HS:%02u)", static_cast<unsigned>(score), static_cast<unsigned>(highscore));
  write_row(0, title);
  write_row(1, status);
}

void show_watch_prompt()
{
  Serial.println("Watch sequence");
}

void show_input_prompt()
{
  Serial.println("Repeat sequence");
}

void show_game_result(bool won)
{
  char status[17];
  snprintf(status, sizeof(status), "Score:%02u HS:%02u", static_cast<unsigned>(score), static_cast<unsigned>(highscore));
  write_row(0, won ? "You win!" : "Game over");
  write_row(1, status);
}

void show_no_previous_settings()
{
  write_row(0, "No saved mode");
  write_row(1, "Choose New Game");
}