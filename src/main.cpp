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

unsigned long timer = millis();

enum State 
{
    MOVE_FORWARD,
    MOVE_BACKWARD,
    MAIN_PRESSED,
    NO_ACTION
};

State state = NO_ACTION;

void setup()
{
    FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);
    // FastLED.addLeds<WS2812, DATA_PIN, RGB>(leds, NUM_LEDS);

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    pinMode(FORWARD_PIN, INPUT);
    pinMode(BACKWARD_PIN, INPUT);
    pinMode(STOVE_PIN, INPUT);
    pinMode(CHARGE_PIN, INPUT);
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

void readInputs()
{

}

void loop()
{
    
}