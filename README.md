# chirp_wemos_azure

Software for a plant status reporting device.

## Development Setup

Setup to compile software to run on a Wemos D1 Mini board to send Chirp sensor metrics to Azure IoT hub.

## On MacOS

Script `setup/macos.sh` contains commands to install everything necessary to build and install this software from a MacOS machine.

## Device Setup

1. Install software on the device as usual with Arduino/Wemos.
2. Join WLAN named `DEVICE_SETUP`.
3. A window should pop up. If it doesn't, open browser and go to any address, eg. http://set.up/
4. Fill in requested information
    - name and password of the WLAN to use for access to Internet
    - address of MQTT server for uploading the metrics.

## Usage

You can listen in on the serial port for debug messages.
    - Send a one-letter message to get configuration report.
    - Send a JSON object to update configuration
        - "mqtt_server_url": an unused string for now
