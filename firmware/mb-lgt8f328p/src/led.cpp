#include "led.h"

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
    // Implementation for turning LED on
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
    // Implementation for turning LED on for a specified duration
    ledOn(pos, type);
    delay(duration);
    ledOff(pos, type);
}

void ledOff(int pos, LedType type)
{
    // Implementation for turning LED off
    if (type == LED_COLORED)
    {
        digitalWrite(coloredLedPins[pos], LOW);
    }
    else if (type == LED_PLAIN)
    {
        digitalWrite(plainLedPins[pos], LOW);
    }
}
