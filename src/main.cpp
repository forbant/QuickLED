#include <Arduino.h>
#include "FastLED.h"

#define NUM_LEDS 124
#define DATA_PIN 3

#define BUTTON_PIN 4

#define FORWARD_PIN 11
#define BACKWARD_PIN 12
#define STOVE_PIN 0
#define CHARGE_PIN 0

//methods
void initMoveArray();
void tickMoveArray();

CRGB leds[NUM_LEDS];
CRGB moveArray[NUM_LEDS];

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
    FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    pinMode(FORWARD_PIN, INPUT_PULLUP);
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

void moveAnimation(bool direction)
{
    CRGB tempColor;
    if (direction)
    {
        // for (int i = 0; i < NUM_LEDS; i++)
        // {
        //     if (i % 20 < 10)
        //     {
        //         int w = map(i % 20, 0, 9, 10, 255);
        //         leds[i] = CHSV(hue, 255, w);
        //     }
        //     else
        //     {
        //         leds[i] = CRGB::Black;
        //     }
        // }
        initMoveArray();
        for (size_t i = 0; i < 20; i++)
        {
            for (size_t i = 0; i < NUM_LEDS; i++)
            {
                leds[i] = moveArray[i];
            }
            FastLED.show();
            delay(50);
            tickMoveArray();

            
            // for (int L = halfLed; L >= 0; L--)
            // {
            //     if(L == 0){
            //         leds[0] = tempColor;
            //         continue;
            //     }
            //     if(L == halfLed)
            //         tempColor = leds[halfLed];
            //     leds[L] = leds[L-1];
            // }
            // for (int L = halfLed+1; L < NUM_LEDS; L++)
            // {
            //     if(L == NUM_LEDS-1){
            //         leds[NUM_LEDS-1] = tempColor;
            //         continue;
            //     }
            //     if(L == halfLed+1)
            //         tempColor = leds[halfLed+1];
            //     leds[L] = leds[L+1];
            // }
        }
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
    bool backwardButton = digitalRead(BACKWARD_PIN);

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
}

void loop()
{
    readInputs();
}

///implementation
void initMoveArray()
{
    for (int i = 0; i < NUM_LEDS; i++)
    {
        if (i % 20 < 10)
        {
            int w = map(i % 20, 0, 9, 10, 255);
            moveArray[i] = CHSV(hue, 255, w);
        }
        else
        {
            moveArray[i] = CRGB::Black;
        }
    }
}

void tickMoveArray()
{
    CRGB tempColor;
    int halfLed = NUM_LEDS / 2;
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

    //
    // for (int L = 0; L < halfLed; L++)
    // {
    //     if (L == 0)
    //     {
    //         tempColor = moveArray[0];
    //         moveArray[0] = moveArray[NUM_LEDS - 1];
    //         continue;
    //     }
    //     moveArray[L - 1] = moveArray[L];
    // }

    // for (int L = NUM_LEDS - 1; L >= halfLed + 1; L--)
    // {
    //     if (L == NUM_LEDS - 1)
    //     {
    //         moveArray[NUM_LEDS - 1] = tempColor;
    //         continue;
    //     }
    //     moveArray[L] = moveArray[L - 1];
    // }
}