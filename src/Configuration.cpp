#include "Configuration.h"

configuration_t configuration = {
    (char *) "",
    false
};

const char * config_filename = "/config.json";
const char * mqtt_server_url_name = "mqtt_server_url";

WiFiManager wifiManager;
bool pleaseSaveConfig = false;

void log(const char * message_part, ...)
{
    va_list arg;

    Serial.printf("[%04d]", millis()/1000);

    va_start(arg, message_part);
    Serial.print(' ');
    Serial.print(message_part);
    va_end(arg);

    Serial.println();
    Serial.flush();
}

void saveConfigCallback ()
{
    pleaseSaveConfig = true;
}

char * copy_string_realloc_when_longer(char * target, const char * source)
{
    size_t source_size = source ? strlen(source) : 0;
    size_t target_size = target ? strlen(target) : 0;

    if (source_size > target_size)
    {
        target = (char *) (target_size ? realloc(target, source_size + 1) : malloc(source_size + 1));
    }

    if (source && target)
    {
        strncpy(target, source, source_size + 1);
    }

    return target;
}

void serializeConfiguration (const configuration_t * configuration, char * buffer, size_t bufsiz)
{
    StaticJsonBuffer<CONFIG_SIZE> json_buffer;
    JsonObject& json = json_buffer.createObject();
    json[mqtt_server_url_name] = configuration->mqtt_server_url;

    json.printTo(buffer, bufsiz);
}

void deserializeConfiguration(configuration_t * configuration, const char * json)
{
    log("Reading new configuration:", json);

    StaticJsonBuffer<CONFIG_SIZE> json_buffer;
    JsonObject &jsonObject = json_buffer.parseObject(json, 0);

    configuration->configured = jsonObject.success();
    if (configuration->configured)
    {
        configuration->mqtt_server_url =
            copy_string_realloc_when_longer(configuration->mqtt_server_url, jsonObject[mqtt_server_url_name]);
        log("Configuration ok:", json);
    }
    else
    {
        log("Failed to parse:", json);
    }
}

void saveConfiguration (configuration_t * configuration)
{
    char buffer[CONFIG_SIZE];
    serializeConfiguration(configuration, buffer, CONFIG_SIZE);
    log("Saving to file:", config_filename, "configuration:", buffer);

    File f = SPIFFS.open(config_filename, "w");
    for (int i=0; i < CONFIG_SIZE && buffer[i]; ++i)
    {
        f.write(buffer[i]);
    }
    f.close();

    log("Saving to file:", config_filename, "configuration:", buffer);
}

void loadConfiguration (configuration_t * configuration)
{
    char config_string[CONFIG_SIZE];
    log("Load configuration from file", config_filename);

    if (SPIFFS.exists(config_filename))
    {
        File f = SPIFFS.open(config_filename, "r");
        log("Reading file");

        int config_string_length = f.readBytes(config_string, CONFIG_SIZE);
        f.close();
        config_string[config_string_length] = '\0';
        log(config_string);

        deserializeConfiguration(configuration, config_string);
    }
    else
    {
        log("Configuration file does not exist.");
    }
}

void setupWifi (configuration_t * configuration, const char * setup_wlan_name)
{
    WiFiManagerParameter mqtt_server_parameter(mqtt_server_url_name, "MQTT Server", configuration->mqtt_server_url, PARAM_LEN);
    wifiManager.addParameter(&mqtt_server_parameter);
    wifiManager.setSaveConfigCallback(saveConfigCallback);

    log("Wifi setup starting with fallback WLAN:", setup_wlan_name);

    wifiManager.autoConnect(setup_wlan_name);

    if (pleaseSaveConfig)
    {
        log("Saving configuration given over setup WLAN");
        configuration->mqtt_server_url =
            copy_string_realloc_when_longer(configuration->mqtt_server_url, mqtt_server_parameter.getValue());
        configuration->configured = true;

        saveConfiguration(configuration);
    }

    log("Wifi setup done");
}

void setupConfiguration (configuration_t * configuration, const char * setup_wlan_name)
{
    log("Mounting filesystem");

    if (SPIFFS.begin())
    {
        log("Filesystem mounted");

        loadConfiguration(configuration);
    }
    else
    {
        log("Formatting new filesystem");

        SPIFFS.format();
    }

    setupWifi(configuration, setup_wlan_name);
}

void reconfigure(configuration_t * configuration, const char * json)
{
    deserializeConfiguration(configuration, json);
}

void reportConfiguration (configuration_t * configuration)
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
