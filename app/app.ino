// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// Please use an Arduino IDE 1.6.8 or greater

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiUdp.h>

#include <AzureIoTHub.h>
#include <AzureIoTProtocol_MQTT.h>
#include <AzureIoTUtility.h>
#include "Configuration.h"

#include "config.h"

static bool messagePending = false;
static bool messageSending = true;

static char *connectionString;
static char *ssid;
static char *pass;

static int interval = INTERVAL;

unsigned long enterDeepSleepAfterSeconds = 0;

void blinkLED()
{
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
}

void initTime()
{
    time_t epochTime;
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");

    while (true)
    {
        epochTime = time(NULL);

        if (epochTime == 0)
        {
            Serial.println("Fetching NTP epoch time failed! Waiting 2 seconds to retry.");
            delay(2000);
        }
        else
        {
            Serial.printf("Fetched NTP epoch time is: %lu.\r\n", epochTime);
            break;
        }
    }
}

static IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle = NULL;
void setup()
{
    pinMode(LED_PIN, OUTPUT);
    pinMode(POWER_PIN, OUTPUT);

    initSerial();
    setupConfiguration(&configuration, "DEVICE_SETUP");

    initTime();
    initSensor();

    reportConfiguration(&configuration);

    while (iotHubClientHandle == NULL)
    {
        reconfigure_via_serial();
        iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(configuration.mqtt_server_url, MQTT_Protocol);

        if (iotHubClientHandle == NULL)
        {
            Serial.println("Failed on IoTHubClient_CreateFromConnectionString.");
            reportConfiguration(&configuration);
            delay(interval);
        }
    }

    IoTHubClient_LL_SetOption(iotHubClientHandle, "product_info", "ttwemos");
    IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, receiveMessageCallback, NULL);
    IoTHubClient_LL_SetDeviceMethodCallback(iotHubClientHandle, deviceMethodCallback, NULL);
    IoTHubClient_LL_SetDeviceTwinCallback(iotHubClientHandle, twinCallback, NULL);
}

void reconfigure_via_serial()
{
    if (Serial && Serial.available())
    {
        char new_configuration_json[CONFIG_SIZE + 1];
        Serial.readBytesUntil('\n', new_configuration_json, CONFIG_SIZE);
        reconfigure(&configuration, new_configuration_json);
        reportConfiguration(&configuration);
    }
}

void loop()
{
    if (!messagePending && messageSending)
    {
        char messagePayload[MESSAGE_MAX_LEN];
        readMessage(messagePayload);
        sendMessage(iotHubClientHandle, messagePayload);

        if (configuration.sleep_seconds > 0)
        {
            if (!enterDeepSleepAfterSeconds)
                enterDeepSleepAfterSeconds = configuration.stay_awake_seconds;
        }
        else
        {
            Serial.println("Sleep time not set, will not sleep.");
        }
    }

    IoTHubClient_LL_DoWork(iotHubClientHandle);
    delay(interval);

    reconfigure_via_serial();

    if (enterDeepSleepAfterSeconds)
    {
        --enterDeepSleepAfterSeconds;
        Serial.print("Time is now ");
        Serial.print(millis());
        Serial.print("; will sleep in ");
        Serial.println(enterDeepSleepAfterSeconds);

        if (!enterDeepSleepAfterSeconds)
        {
            Serial.println("Will sleep now.");
            WiFi.mode(WIFI_OFF);
            ESP.deepSleep(1000000 * configuration.sleep_seconds, WAKE_RF_DEFAULT);
        }
    }
}
