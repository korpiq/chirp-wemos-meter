#include "Configuration.h"

configuration_t configuration = {
    (char *)"", // mqtt_server_url
    (char *)"", // pump_actuator_url
    0,          // sleep_seconds
    10,         // stay_awake_seconds
    false       // are we configured yet?
};

const char *config_filename = "settings.json";
const char *mqtt_server_url_name = "mqtt_server_url";
const char *pump_actuator_url_name = "pump_actuator_url";
const char *sleep_seconds_name = "sleep_seconds";
const char *stay_awake_seconds_name = "stay_awake_seconds";

WiFiManager wifiManager;
bool pleaseSaveConfig = false;

void saveConfigCallback()
{
    pleaseSaveConfig = true;
}

char *copy_string_realloc_when_longer(char *target, const char *source, size_t max_length)
{
    size_t source_size = source ? strnlen(source, max_length) : 0;
    size_t target_size = target ? strnlen(target, max_length) : 0;

    if (source_size > target_size)
    {
        target = (char *)(target_size ? realloc(target, source_size + 1) : malloc(source_size + 1));
    }

    if (source && target)
    {
        strncpy(target, source, source_size);
        target[source_size] = 0;
    }

    return target;
}

void serializeConfiguration(const configuration_t *configuration, char *buffer, size_t bufsiz)
{
    DynamicJsonBuffer json_buffer;
    JsonObject &json = json_buffer.createObject();
    json[mqtt_server_url_name] = configuration->mqtt_server_url;
    json[pump_actuator_url_name] = configuration->pump_actuator_url;
    json[sleep_seconds_name] = configuration->sleep_seconds;
    json[stay_awake_seconds_name] = configuration->stay_awake_seconds;

    json.printTo(buffer, bufsiz);
}

void deserializeConfiguration(configuration_t *configuration, const char *json)
{

    DynamicJsonBuffer json_buffer;
    JsonObject &jsonObject = json_buffer.parseObject(json);

    configuration->configured = jsonObject.success();
    if (configuration->configured)
    {
        if (jsonObject.containsKey(mqtt_server_url_name))
        {
            configuration->mqtt_server_url =
                copy_string_realloc_when_longer(configuration->mqtt_server_url, jsonObject[mqtt_server_url_name], PARAM_LEN);
        }

        if (jsonObject.containsKey(pump_actuator_url_name))
        {
            configuration->pump_actuator_url =
                copy_string_realloc_when_longer(configuration->mqtt_server_url, jsonObject[pump_actuator_url_name], PARAM_LEN);
        }

        if (jsonObject.containsKey(sleep_seconds_name))
        {
            configuration->sleep_seconds = jsonObject[sleep_seconds_name];
        }

        if (jsonObject.containsKey(stay_awake_seconds_name))
        {
            configuration->stay_awake_seconds = jsonObject[stay_awake_seconds_name];
            if (configuration->stay_awake_seconds < 10)
            {
                configuration->stay_awake_seconds = 10;
            }
        }
    }
}

void saveConfiguration(configuration_t *configuration)
{
    char buffer[CONFIG_SIZE];
    serializeConfiguration(configuration, buffer, CONFIG_SIZE);
    File f = SPIFFS.open(config_filename, "w");
    for (int i = 0; i < CONFIG_SIZE && buffer[i]; ++i)
    {
        f.write(buffer[i]);
    }
    f.close();
}

void loadConfiguration(configuration_t *configuration)
{
    char config_string[CONFIG_SIZE];

    if (SPIFFS.exists(config_filename))
    {
        File f = SPIFFS.open(config_filename, "r");

        int config_string_length = f.readBytes(config_string, CONFIG_SIZE);
        f.close();
        config_string[config_string_length] = '\0';

        deserializeConfiguration(configuration, config_string);
    }
}

void setupWifi(configuration_t *configuration, const char *setup_wlan_name)
{
    WiFiManagerParameter mqtt_server_parameter(mqtt_server_url_name, "MQTT Server", configuration->mqtt_server_url, PARAM_LEN);
    wifiManager.addParameter(&mqtt_server_parameter);
    WiFiManagerParameter pump_actuator_parameter(pump_actuator_url_name, "Water pump", configuration->pump_actuator_url, PARAM_LEN);
    wifiManager.addParameter(&pump_actuator_parameter);
    wifiManager.setSaveConfigCallback(saveConfigCallback);

    wifiManager.autoConnect(setup_wlan_name);

    if (pleaseSaveConfig)
    {
        configuration->mqtt_server_url =
            copy_string_realloc_when_longer(configuration->mqtt_server_url, mqtt_server_parameter.getValue(), PARAM_LEN);
        configuration->pump_actuator_url =
            copy_string_realloc_when_longer(configuration->pump_actuator_url, pump_actuator_parameter.getValue(), PARAM_LEN);
        configuration->configured = true;

        saveConfiguration(configuration);
    }
}

void setupConfiguration(configuration_t *configuration, const char *setup_wlan_name)
{

    if (SPIFFS.begin())
    {

        loadConfiguration(configuration);
    }
    else
    {

        SPIFFS.format();
    }

    setupWifi(configuration, setup_wlan_name);
}

void reconfigure(configuration_t *configuration, const char *json)
{
    deserializeConfiguration(configuration, json);
    if (configuration->configured)
    {
        saveConfiguration(configuration);
    }
}

void reportConfiguration(configuration_t *configuration)
{
    if (configuration->configured)
    {
        char buffer[CONFIG_SIZE];
        serializeConfiguration(configuration, buffer, CONFIG_SIZE);

        Serial.println(buffer);
    }
    else
    {
        Serial.println("\"unconfigured\"");
    }
}
