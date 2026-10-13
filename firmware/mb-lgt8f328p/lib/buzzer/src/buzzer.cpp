#include "buzzer.h"

Buzzer::Buzzer(uint8_t pinNumber) : pin(pinNumber)
{
}

Buzzer::~Buzzer()
{
}

void Buzzer::begin()
{
  pinMode(pin, OUTPUT);
  noTone(pin);
}

void Buzzer::play(Note note)
{
  // Explicitly cast the Enum to int to get the frequency
  int frequency = static_cast<int>(note);

  tone(pin, frequency);
}

void Buzzer::stop()
{
  noTone(pin);
}

void Buzzer::playMelody(const MelodyStep *melody, int length)
{
  for (int i = 0; i < length; i++)
  {
    Note currentNote = melody[i].note;
    int duration = melody[i].duration;

    // start
    play(currentNote);

    // delay
    delay(duration);
  }
  stop();
}
