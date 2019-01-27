// Physical device information for board and sensor
#define DEVICE_ID "wemostt"

// Pin layout configuration
#define LED_PIN BUILTIN_LED
#define POWER_PIN D8

// Deep sleep seconds, if 0 no sleep // 900;  // 1h 3600;
#define DSSECONDS 10

#define TEMPERATURE_ALERT 30

// Interval time(ms) for sending message to IoT Hub
#define INTERVAL 2000

// If don't have a physical DHT sensor, can send simulated data to IoT hub
#define SIMULATED_DATA false

// EEPROM address configuration
#define EEPROM_SIZE 512

// SSID and SSID password's length should < 32 bytes
// http://serverfault.com/a/45509
#define SSID_LEN 32
#define PASS_LEN 32
#define CONNECTION_STRING_LEN 256

#define MESSAGE_MAX_LEN 256
