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
    "src/key_handling/key_press_common.cpp"
    "src/key_handling/key_escape.cpp"
    "src/key_handling/key_edit.cpp"
    "src/key_handling/key_help.cpp"
    "src/key_handling/key_groups.cpp"
    "src/key_handling/key_marks.cpp"
    "src/key_handling/key_pinned.cpp"
    "src/key_handling/key_group_marks.cpp"
    "src/key_handling/key_filter.cpp"
    "src/command/command_general.cpp"
    "src/command/theme.cpp"
    "src/command/config.cpp"
    "src/main_list/navigation.cpp"
    "src/main_list/manage.cpp"
    "src/main_list/edit.cpp"
    "src/helpers/ui/general.cpp"
)

HEADERS=(
    "src/config.h"
    "src/help.h"
    "src/key_translation.h"
    "src/main.h"
    "src/ui.h"
    "src/utils.h"
)
