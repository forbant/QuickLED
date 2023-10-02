#include <Arduino.h>
#include "FastLED.h"

#define NUM_LEDS 124
#define DATA_PIN 3

#define BUTTON_PIN 4

#define FORWARD_PIN 8
#define BACKWARD_PIN 9
#define STOVE_PIN 10
#define CHARGE_PIN 11

CRGB leds[NUM_LEDS];

// Button
unsigned long buttonPressTime = 0;
unsigned long longPressThreshold = 2000;

enum State
{
    MOVE_FORWARD,
    MOVE_BACKWARD,
    ON,
    OFF,
    TUNE,
    S
};

State state = OFF;

CRGB mainColor[] =
    {
        CRGB(200, 0, 0),
        CRGB(200, 200, 0),
        CRGB(0, 200, 0),
        CRGB(0, 200, 200),
        CRGB(0, 0, 200),
        CRGB(200, 0, 200),
        CRGB(150, 150, 150),
};
int colorIndex = 0;
int hue = 0;
int value = 0;

void setup()
{
    // FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    pinMode(FORWARD_PIN, INPUT);
    pinMode(BACKWARD_PIN, INPUT);
    pinMode(STOVE_PIN, INPUT);
    pinMode(CHARGE_PIN, INPUT);

    for (size_t i = 0; i < NUM_LEDS; i++)
    {
        leds[i] = CRGB::Black;
    }
    FastLED.show();

    Serial.begin(9600);
}

void testColor(CRGB color)
{
    for (size_t i = 0; i < NUM_LEDS; i++)
    {
        leds[i] = color;
        FastLED.show();
        delay(10);
    }
    delay(2000);
    for (size_t i = 0; i < NUM_LEDS; i++)
    {
        leds[i] = CRGB::Black;
        FastLED.show();
        delay(10);
    }
    delay(1000);
}

void changeColor()
{
    hue++;
    CHSV colorH = CHSV(hue, 255, 255);
    for (size_t i = 0; i < NUM_LEDS; i++)
    {
        leds[i] = colorH;
    }
    FastLED.show();
    delay(50);
}

void handleTune()
{
    if (buttonPressTime == 0)
    {
        buttonPressTime = millis();
        return;
    }
    if ((millis() - buttonPressTime) > longPressThreshold)
    {
        while (!digitalRead(BUTTON_PIN))
        {
            changeColor();
        }
        state = ON;
        return;
    }
    if (digitalRead(BUTTON_PIN))
    {
        state = OFF;
        for (; value >= 0; value--)
        {
            for (size_t i = 0; i < NUM_LEDS; i++)
            {
                leds[i] = CHSV(hue, 255, value);
            }
            FastLED.show();
        }
    }
}

void readInputs()
{
    bool mainButton = !digitalRead(BUTTON_PIN);

    if (mainButton)
    {
        switch (state)
        {
        case OFF:
            state = ON;
            for (value = 0 ; value < 255; value++)
            {
                for (size_t i = 0; i < NUM_LEDS; i++)
                {
                    leds[i] = CHSV(hue, 255, value);
                }
                FastLED.show();
            }
            break;
        case ON:
            state = TUNE;
            break;
        case TUNE:
            handleTune();
            break;

        default:
            break;
        }
    }
    else
    {
        buttonPressTime = 0;
    }
}

void loop()
{
    readInputs();
}