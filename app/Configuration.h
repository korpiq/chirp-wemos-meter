#include <ESP8266WiFi.h>      //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#include <DNSServer.h>        //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h> //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>      //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <ArduinoJson.h>
#include "FS.h"

#pragma once

typedef struct configuration
{
    char *mqtt_server_url;
    unsigned int sleep_seconds;
    unsigned int stay_awake_seconds;
    bool configured;
} configuration_t;

extern configuration_t configuration;

void setupConfiguration(configuration_t *configuration, const char *setup_wlan_name);
void reconfigure(configuration_t *configuration, const char *json);
void reportConfiguration(configuration_t *configuration);

#define PARAM_LEN 200
#define PARAM_NAME_LEN 20
#define NUM_PARAMS 1
#define CONFIG_SIZE (PARAM_LEN + PARAM_NAME_LEN + 6) * NUM_PARAMS
