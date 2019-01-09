#include <Adafruit_Sensor.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <Wire.h>

#if SIMULATED_DATA

void initSensor()
{
    // use SIMULATED_DATA, no sensor need to be inited
}

float readTemperature()
{
    return random(20, 30);
}

float readHumidity()
{
    return random(30, 40);
}

#else

void initSensor()
{
    digitalWrite(POWER_PIN, HIGH);
    Wire.begin();
    writeI2CRegister8bit(0x20, 6); //reset
    delay(100);
}

float readTemperature()
{
    delay(100);
    return readI2CRegister16bit(0x20, 5);
}

float readMoisture()
{
    return readI2CRegister16bit(0x20, 0);
}

float readLight()
{
    writeI2CRegister8bit(0x20, 3); //request light measurement
    delay(500);                    //this can take a while
    return readI2CRegister16bit(0x20, 4);
}

#endif

unsigned int readI2CRegister16bit(int addr, int reg)
{
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.endTransmission();
    delay(20);
    Wire.requestFrom(addr, 2);
    unsigned int t = Wire.read() << 8;
    t = t | Wire.read();
    return t;
}

void writeI2CRegister8bit(int addr, int value)
{
    Wire.beginTransmission(addr);
    Wire.write(value);
    Wire.endTransmission();
}

bool readMessage(int messageId, char *payload)
{
    float temperature = readTemperature() / 10;
    float moisture = (readMoisture() - 300) / 4;
    float light = (65535 - readLight()) / 65535 * 100;
    StaticJsonBuffer<MESSAGE_MAX_LEN> jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["deviceId"] = DEVICE_ID;
    root["messageId"] = messageId;
    bool temperatureAlert = false;

    // NAN is not the valid json, change it to NULL
    if (std::isnan(temperature))
    {
        root["temperature"] = NULL;
    }
    else
    {
        root["temperature"] = temperature;
        if (temperature > TEMPERATURE_ALERT)
        {
            temperatureAlert = true;
        }
    }

    if (std::isnan(moisture))
    {
        root["moisture"] = NULL;
    }
    else
    {
        root["moisture"] = moisture;
    }

    if (std::isnan(light))
    {
        root["light"] = NULL;
    }
    else
    {
        root["light"] = light;
    }
    root.printTo(payload, MESSAGE_MAX_LEN);
    return temperatureAlert;
}

void parseTwinMessage(char *message)
{
    StaticJsonBuffer<MESSAGE_MAX_LEN> jsonBuffer;
    JsonObject &root = jsonBuffer.parseObject(message);
    if (!root.success())
    {
        Serial.printf("Parse %s failed.\r\n", message);
        return;
    }

    if (root["desired"]["interval"].success())
    {
        interval = root["desired"]["interval"];
    }
    else if (root.containsKey("interval"))
    {
        interval = root["interval"];
    }
}
