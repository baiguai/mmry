#!/bin/bash

# Central configuration - edit values here, all scripts pick them up

APP_NAME="mmry"

SOURCES=(
    "src/config.cpp"
    "src/help.cpp"
    "src/key_translation.cpp"
    "src/main.cpp"
    "src/ui_linux.cpp"
    "src/ui_win32.cpp"
    "src/utils.cpp"
)

HEADERS=(
    "src/config.h"
    "src/help.h"
    "src/key_translation.h"
    "src/main.h"
    "src/ui.h"
    "src/utils.h"
)
