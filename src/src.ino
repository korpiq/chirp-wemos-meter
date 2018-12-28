#include "Arduino.h"
#include "Configuration.h"

#define LED 2

bool serialBegun = false;
bool led_state = false;

void blink (uint8_t times, uint16_t hi, uint16_t lo)
{
    while(times--)
    {
      digitalWrite(LED, HIGH);
      delay(hi);
      digitalWrite(LED, LOW);
      delay(lo);
    }
}

void setup ()
{
    pinMode(LED, OUTPUT);
    blink(10, 400, 100);
    digitalWrite(LED, HIGH);

    setupConfiguration(&configuration, "DEVICE_SETUP");

    blink(10, 100, 100);

    Serial.begin(115200);
    Serial.println("Hello");
    Serial.flush();
    serialBegun = true;
}

void loop ()
{
    if (Serial != serialBegun)
    {
        if (! serialBegun)
        {
            Serial.begin(115200);
            Serial.println("Hello");
            Serial.flush();

            blink(4, 200, 100);
        }
        else
        {
            blink(8, 100, 100);
        }

        serialBegun = !serialBegun;
    }

    if (serialBegun)
    {
        if(Serial.available())
        {
            blink(3, 200, 100);

            char buffer[CONFIG_SIZE + 1];
            size_t buflen = Serial.readBytesUntil('\n', buffer, CONFIG_SIZE);
            buffer[buflen] = 0;
            Serial.print("reconfiguring: ");
            Serial.println(buffer);
            reconfigure(&configuration, buffer);

            blink(5, 100, 100);
        }

        reportConfiguration(&configuration);
        Serial.flush();
    }

    blink(1, 500, 500);
}

