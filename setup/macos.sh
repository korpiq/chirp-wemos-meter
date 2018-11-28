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

test_usb_driver () {
    ls -d /{System,}/Library/Extensions/SiLabsUSBDriver*.kext 2>/dev/null | grep -q .
}

install_usb_driver () {
    ensure_brew cask install homebrew/cask-drivers/silicon-labs-vcp-driver
}

find_library () {
    for FILE in "$ARDUINO_LIBDIR"/*/library.properties
    do
        if grep -q "^name=$1$" < "$FILE" &&
            grep -q "^version=$2" < "$FILE"
        then
            dirname "$FILE"
            return 0
        fi
    done
}

list_missing_libraries () {
    sed 's/#.*//' "$THIS_DIR/arduino-libraries.txt" |
    grep . |
    while IFS=: read LIB VER REST
    do
        echo -n "Library '$LIB' version $VER: " >&2
        LIB_DIR=$(find_library "$LIB" "$VER")
        if [ -n "$LIB_DIR" ]
        then
            echo "already installed at: $LIB_DIR" >&2
        else
            echo "will be installed." >&2
            echo -n "$LIB:$VER,"
        fi
    done

    echo ""
}

test_libraries () {
    INSTALL_LIBS=$(list_missing_libraries | sed 's/,$//') 2>&1

    [ -z "$INSTALL_LIBS" ]
}

install_libraries () {
    echo "Installing libraries: $INSTALL_LIBS"

    $ARDUINO_CLI --install-library "$INSTALL_LIBS"
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
    "ESP8266 Core for Arduino to build for target hardware" \
    "usb_driver to connect to target hardware" \
    "libraries that provide features for the program"

