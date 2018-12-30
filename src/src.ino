#include "Arduino.h"
#include "Configuration.h"

extern "C" {
#include "user_interface.h"
}


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

void report_memory()
{
    uint32_t free = system_get_free_heap_size();
    Serial.print("Free memory: ");
    Serial.println(free);
    Serial.flush();
}

void setup ()
{
    pinMode(LED, OUTPUT);
    blink(10, 400, 100);
    digitalWrite(LED, HIGH);

    Serial.begin(115200);
    Serial.println("Configure?");
    report_memory();

    setupConfiguration(&configuration, "DEVICE_SETUP");

    Serial.println("Configured.");
    report_memory();

    blink(10, 100, 100);

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
            if (buflen > 2)
            {
                buffer[buflen] = 0;
                Serial.print("reconfiguring: ");
                Serial.println(buffer);
                Serial.flush();
                reconfigure(&configuration, buffer);
            }
            else
            {
                Serial.println("nop ok");
                Serial.flush();
            }

            reportConfiguration(&configuration);
            report_memory();

            blink(5, 100, 100);
        }
    }

    blink(1, 500, 500);
}
