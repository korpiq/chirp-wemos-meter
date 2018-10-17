#!/bin/bash

set -e

ARDUINO_CLI="/Applications/Arduino.app/Contents/MacOS/Arduino"
ESP8266_URL="http://arduino.esp8266.com/stable/package_esp8266com_index.json"
BOARDS_PREF="boardsmanager.additional.urls"

ensure_brew () {
    # ensure we have brew if required for installing missing software

    if ! which brew >/dev/null
    then
        echo "Installing brew to run '$*'"
        /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
        export PATH="/usr/local/bin:$PATH"
    fi

    brew "$@"
}

test_Arduino () {
    [ -x "$ARDUINO_CLI" ]
}

install_Arduino () {
    ensure_brew cask install arduino
}

test_ESP8266 () {
    BOARDS_SET="$($ARDUINO_CLI --get-pref "$BOARDS_PREF" 2>/dev/null)"
    grep -q "$ESP8266_URL" <<<"$BOARDS_SET"
}

install_ESP8266 () {
    # Configure Arduino to support ESP8266
    # Core to install: https://github.com/esp8266/Arduino
    # Arduino CLI options: https://github.com/arduino/Arduino/blob/master/build/shared/manpage.adoc

    $ARDUINO_CLI --pref "$BOARDS_PREF=$ESP8266_URL${BOARDS_SET:+,$BOARDS_SET}" --save-prefs
    $ARDUINO_CLI --install-boards esp8266:esp8266 --save-prefs
}

ensure_installed () {
    for STEP in "$@"
    do
        if test_$STEP
        then
            echo "Already installed: $STEP"
        else
            echo "Installing: $STEP"
            install_$STEP
            echo "Installed: $STEP"
        fi
    done
}

ensure_installed \
    "Arduino for compiling and installing software" \
    "ESP8266 Core for Arduino to support target hardware"

