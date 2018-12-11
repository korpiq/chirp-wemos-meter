#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <ArduinoJson.h>
#include "FS.h"

#pragma once

typedef struct configuration {
    const char * mqtt_server_url;
} configuration_t;

extern configuration_t configuration;

void setupConfiguration (configuration_t * configuration, char * const setup_wlan_name);
void reportConfiguration (configuration_t * configuration);
