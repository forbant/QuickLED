#include <Arduino.h>
#include "FastLED.h"
#include "config.h"

CRGB leds[NUM_LEDS];
int valueArray[NUM_LEDS];
CRGB moveArray[NUM_LEDS];

//methods
void initMoveArray();
void tickMoveArray(bool);

// const
const int beamSize = 31;

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
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    pinMode(FORWARD_PIN, INPUT_PULLUP);
    pinMode(BACKWARD_PIN, INPUT_PULLUP);
    pinMode(STOVE_PIN, INPUT);
    pinMode(CHARGE_PIN, INPUT);

    for (size_t i = 0; i < NUM_LEDS; i++)
    {
        leds[i] = CRGB::Black;
    }
    initMoveArray();
    for(int i = 0; i < NUM_LEDS; i++) {
        leds[i] = moveArray[i];
    }
    leds[NUM_LEDS/2] = CRGB(0,0,255);
    FastLED.show();

    Serial.begin(9600);
    
}

void moveAnimation(bool direction)
{
    CRGB tempColor;
    initMoveArray();
    for (size_t i = 0; i < beamSize; i++)
    {
        for (size_t i = 0; i < NUM_LEDS; i++)
        {
            leds[i] = moveArray[i];
        }
        FastLED.show();
        delay(20);
        tickMoveArray(direction);
    }
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
    bool forwardButton = !digitalRead(FORWARD_PIN);
    bool backwardButton = !digitalRead(BACKWARD_PIN);

    if (mainButton)
    {
        switch (state)
        {
        case OFF:
            Serial.println("OFF");
            state = ON;
            for (value = 0; value < 255; value++)
            {
                for (size_t i = 0; i < NUM_LEDS; i++)
                {
                    leds[i] = CHSV(hue, 255, value);
                }
                FastLED.show();
            }
            break;
        case ON:
            Serial.println("ON");
            state = TUNE;
            break;
        case TUNE:
            Serial.println("TUNE");
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
    if (forwardButton)
    {
        moveAnimation(true);
    }
    if (backwardButton)
    {
        moveAnimation(false);
    }
}

void loop()
{
    readInputs();
}

///implementation
void initMoveArray()
{
    float stepSize = 2 * PI / (NUM_LEDS/4);

    // Fill the array with four complete sine waves
    for (int i = 0; i < NUM_LEDS; i++) {
        int s = cos(i * stepSize)*100;
        int v = map(s, -100, 100, 50, 255);
        moveArray[i] = CHSV(hue, 255, v);
    }
}

void tickMoveArray(bool direction)
{
    CRGB tempColor;
    int halfLed = NUM_LEDS / 2;
    if (direction)
    {
        for (int L = halfLed; L >= 0; L--)
        {
            if (L == 0)
            {
                moveArray[0] = tempColor;
                continue;
            }
            if (L == halfLed)
                tempColor = moveArray[halfLed];
            moveArray[L] = moveArray[L - 1];
        }
        for (int L = halfLed + 1; L < NUM_LEDS; L++)
        {
            if (L == NUM_LEDS - 1)
            {
                moveArray[NUM_LEDS - 1] = tempColor;
                continue;
            }
            if (L == halfLed + 1)
                tempColor = moveArray[halfLed + 1];
            moveArray[L] = moveArray[L + 1];
        }
    }
    else
    {
        for (int L = 0; L < halfLed; L++)
        {
            if (L == 0)
            {
                tempColor = moveArray[0];
            }
            if (L == halfLed - 1){
                moveArray[L] = tempColor;
                continue;
            }
            moveArray[L] = moveArray[L + 1];
        }
        for (int L = NUM_LEDS - 1; L >= halfLed; L--)
        {
            if (L == NUM_LEDS - 1)
            {
                moveArray[NUM_LEDS - 1] = tempColor;
            }
            if (L == halfLed){
                moveArray[L] = tempColor;
                continue;
            }
                
            moveArray[L] = moveArray[L - 1];
        }
    }
}