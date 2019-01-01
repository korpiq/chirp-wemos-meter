#include "Configuration.h"

configuration_t configuration = {
    (char *) "",
    false
};

const char * config_filename = "settings.json";
const char * mqtt_server_url_name = "mqtt_server_url";

WiFiManager wifiManager;
bool pleaseSaveConfig = false;

template <typename Arg>
void log_append(Arg message)
{
    Serial.print(" ");
    Serial.print(message);
}

template <typename Arg, typename... Args>
void log_append(Arg message, Args... message_parts)
{
    Serial.print(" ");
    Serial.print(message);
    log_append(message_parts...);
}

template <typename... Args>
void log(Args... message_parts)
{
    Serial.printf("[%04lu]", millis()/1000);
    log_append(message_parts...);
    Serial.println();
    Serial.flush();
}

void saveConfigCallback ()
{
    pleaseSaveConfig = true;
}

char * copy_string_realloc_when_longer(char * target, const char * source, size_t max_length)
{
    size_t source_size = source ? strnlen(source, max_length) : 0;
    size_t target_size = target ? strnlen(target, max_length) : 0;

    if (source_size > target_size)
    {
        target = (char *) (target_size ? realloc(target, source_size + 1) : malloc(source_size + 1));
    }

    if (source && target)
    {
        strncpy(target, source, source_size);
        target[source_size] = 0;
    }

    return target;
}

void serializeConfiguration (const configuration_t * configuration, char * buffer, size_t bufsiz)
{
    DynamicJsonBuffer json_buffer;
    JsonObject& json = json_buffer.createObject();
    json[mqtt_server_url_name] = configuration->mqtt_server_url;

    json.printTo(buffer, bufsiz);
}

void deserializeConfiguration(configuration_t * configuration, const char * json)
{
    log("Reading new configuration:", json);

    DynamicJsonBuffer json_buffer;
    JsonObject &jsonObject = json_buffer.parseObject(json);

    configuration->configured = jsonObject.success();
    if (configuration->configured)
    {
        configuration->mqtt_server_url =
            copy_string_realloc_when_longer(configuration->mqtt_server_url, jsonObject[mqtt_server_url_name], PARAM_LEN);
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
    log("Saving configuration:", buffer);

    File f = SPIFFS.open(config_filename, "w");
    for (int i=0; i < CONFIG_SIZE && buffer[i]; ++i)
    {
        f.write(buffer[i]);
    }
    f.close();

    log("Saved to file:", config_filename);
}

void loadConfiguration (configuration_t * configuration)
{
    char config_string[CONFIG_SIZE];
    log("Load configuration from file", config_filename);

    if (SPIFFS.exists(config_filename))
    {
        File f = SPIFFS.open(config_filename, "r");
        log("Reading file:", config_filename);

        int config_string_length = f.readBytes(config_string, CONFIG_SIZE);
        f.close();
        config_string[config_string_length] = '\0';
        log("Read configuration:", config_string);

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
            copy_string_realloc_when_longer(configuration->mqtt_server_url, mqtt_server_parameter.getValue(), PARAM_LEN);
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
    saveConfiguration(configuration);
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
