#include <Arduino.h>
#include "FastLED.h"
#include "config.h"

#define READING_BUFFER 7

CRGB leds[NUM_LEDS];
int valueArray[NUM_LEDS];
CRGB moveArray[NUM_LEDS];

//methods
void initMoveArray();
void tickMoveArray(bool);
void chargingAnimation(HSVHue);

// const
const int waveSize = 31;
const int contrastValue = 50;
unsigned long chargeLedReadTimer = 0;
const int chargeLedReadThreshold = 250;

bool readingBuffer[READING_BUFFER];

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

size_t hue = 0;
int value = 0;

void setup()
{
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(FORWARD_PIN, INPUT_PULLUP);
    pinMode(BACKWARD_PIN, INPUT_PULLUP);
    pinMode(CHARGE_PIN, INPUT_PULLUP);

    for (size_t i = 0; i < NUM_LEDS; i++)
    {
        leds[i] = CRGB::Black;
    }
    FastLED.show();
    Serial.begin(9600);
}

void readLEDState()
{
    static unsigned long a = 900;
    int state = analogRead(CHARGE_PIN);
    Serial.print("a:");
    Serial.print(a);
    Serial.print(",");
    Serial.print("s:");
    Serial.println(state);
    delay(100);
}

void moveAnimation(bool direction)
{
    CRGB tempColor;
    initMoveArray();
    for (size_t i = 0; i < waveSize; i++)
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

void endMoveAction()
{
    CHSV color = CHSV(hue, 255, value);
    for (int i = 0; i < NUM_LEDS / 2; i++)
    {
        leds[i] = color;
        leds[NUM_LEDS - 1 - i] = color;
        FastLED.show();
    }
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
        value++;
        Serial.print("value:");Serial.println(value);
    }
}

void readInputs()
{
    bool mainButton = !digitalRead(BUTTON_PIN);
    bool forwardButton = !digitalRead(FORWARD_PIN);
    bool backwardButton = !digitalRead(BACKWARD_PIN);
    static bool isMoving = false;
    static bool isCharging = false;

    static int index = 0;

    if((millis() - chargeLedReadTimer) > chargeLedReadThreshold) {
        int chargeLedState = analogRead(CHARGE_PIN);
        chargeLedReadTimer = millis();
        Serial.print("chargingAnimation: ");
        Serial.println(chargeLedState);
        
        readingBuffer[index] = chargeLedState < 900;
        for (int i = 0; i < READING_BUFFER; i++)
        {
            Serial.print(readingBuffer[i]);Serial.print(" ");
        }
        Serial.println(";");
        index++;
        if(index >= READING_BUFFER)
        {
            index = 0;
        }
        bool wasBlink = false;
        for (int i = 0; i < READING_BUFFER; i++)
        {
            if(readingBuffer[i])
                wasBlink = true;
        }
        if(!wasBlink && isCharging)
        {
            chargingAnimation(HUE_BLUE);
            isCharging = false;
        }
        
        if(chargeLedState < 900 && !isCharging)
        {
            chargingAnimation(HUE_GREEN);
            isCharging = true;
        }
    }

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
            Serial.print("value:");Serial.println(value);
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
        if(!digitalRead(FORWARD_PIN))
        {
            moveAnimation(true);
            isMoving = true;
        }
    }
    if (backwardButton)
    {
        if(!digitalRead(BACKWARD_PIN))
        {
            moveAnimation(false);
            isMoving = true;
        }
    }

    if(!forwardButton && !backwardButton && isMoving) {
        endMoveAction();
        isMoving = false;
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
        int v = map(s, -100, 100, contrastValue, 255);
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

void chargingAnimation(HSVHue blinkHue) 
{
    int speed = 5;

    //Decrease brightness to 0
    for(int i = value; i >= 0; i-=speed)
    {
        for(size_t k = 0; k < NUM_LEDS; k++)
        {
            leds[k] = CHSV(hue, 255, i);
        }
        FastLED.show();
    }

    // //Light green light
    for(int i = 0; i < 255; i+=speed)
    {
        for(int k = 0; k < NUM_LEDS; k++)
        {
            leds[k] = CHSV(blinkHue, 255, i);
        }
        FastLED.show();
    }
    delay(300);

    // //Decrease green light brightness to 0
    for(int i = 255; i >= 0; i-=speed)
    {
        for(int k = 0; k < NUM_LEDS; k++)
        {
            leds[k] = CHSV(blinkHue, 255, i);
        }
        FastLED.show();
    }

    //Show saved state
    for(int i = 0; i < value; i+=speed)
    {
        for(int k = 0; k < NUM_LEDS; k++)
        {
            leds[k] = CHSV(hue, 255, i);
        }
        FastLED.show();
    }
}