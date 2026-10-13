#include "main.h"

const uint8_t coloredLedPins[NUM_PADS] = {12, 11, 9, 8};
const uint8_t plainLedPins[NUM_PADS] = {A1, A2, A6, A7};

void initLed()
{
    for (int i = 0; i < NUM_PADS; i++)
    {
        pinMode(coloredLedPins[i], OUTPUT);
        digitalWrite(coloredLedPins[i], LOW);

        pinMode(plainLedPins[i], OUTPUT);
        digitalWrite(plainLedPins[i], LOW);
    }
}

void ledOn(int pos, LedType type)
{
    if (pos < 0 || pos >= NUM_PADS)
    {
        return;
    }

    if (type == LED_COLORED)
    {
        digitalWrite(coloredLedPins[pos], HIGH);
    }
    else if (type == LED_PLAIN)
    {
        digitalWrite(plainLedPins[pos], HIGH);
    }
}

void ledOnFor(int pos, int duration, LedType type)
{
    ledOn(pos, type);
    delay(duration);
    ledOff(pos, type);
}

void ledOff(int pos, LedType type)
{
    if (pos < 0 || pos >= NUM_PADS)
    {
        return;
    }

    if (type == LED_COLORED)
    {
        digitalWrite(coloredLedPins[pos], LOW);
    }
    else if (type == LED_PLAIN)
    {
        digitalWrite(plainLedPins[pos], LOW);
    }
}
