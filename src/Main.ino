#include "Arduino.h"
#include "Configuration.h"

void setup ()
{
    setupConfiguration(&configuration, "DEVICE_SETUP");
}

void loop ()
{
    reportConfiguration(&configuration);
    delay(1000);
}
