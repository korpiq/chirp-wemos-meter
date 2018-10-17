#!/bin/bash

set -e

THIS_DIR=$(cd -- $(dirname "$BASH_SOURCE"); pwd)
ARDUINO_CLI="/Applications/Arduino.app/Contents/MacOS/Arduino"
ARDUINO_LIBDIR="${HOME}/Documents/Arduino/libraries"
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

list_missing_libraries () {
    sed -ne 's/:/ /gp' < "$THIS_DIR/arduino-libraries.txt" |
    while read LIB VER REST
    do
        if [ -e "$ARDUINO_LIBDIR/$LIB/library.properties" ] &&
            grep -q "version=$VER" "$ARDUINO_LIBDIR/$LIB/library.properties"
        then
            echo "Library already installed: $LIB:$VER" >&2
        else
            echo "Library to be installed: $LIB:$VER" >&2
            echo "$LIB:$VER"
        fi
    done
}

test_libraries () {
    INSTALL_LIBS=$(list_missing_libraries) 2>&1

    [ -z "$INSTALL_LIBS" ]
}

install_libraries () {
    for LIB in $INSTALL_LIBS
    do
        echo "Installing library: $LIB"
        $ARDUINO_CLI --install-library $LIB
    done
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
    "Arduino for compiling and installing the program" \
    "ESP8266 Core for Arduino to support target hardware" \
    "libraries that provide features for the program"

