#include <AzureIoTHub.h>

const char * setupIotHub(const char * mqtt_server_url);
static void sendMessage(IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle, char *buffer);
void readMessage(char *payload);

static bool messagePending = false;
static bool messageSending = true;
static IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;
