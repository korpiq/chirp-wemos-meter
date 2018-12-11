#include "Arduino.h"
#include "Configuration.h"

void setup ()
{
    Serial.begin(115200);
    setupConfiguration(&configuration, "DEVICE_SETUP");
}

void loop ()
{
    reportConfiguration(&configuration);
    delay(1000);
}
