#include "../clipboard_manager.h"

bool ClipboardManager::key_command_execute()
{
    if (!commandText.empty())
    {
        executeCommand(commandText);
    }
    commandMode = false;
    commandText = "";
    drawConsole();
    return true;
}

bool ClipboardManager::key_command_down()
{
    size_t displayCount = getDisplayItemCount();
    if (selectedItem < displayCount - 1)
    {
        selectedItem++;
        updateConsoleScrollOffset();
        drawConsole();
    }
    return true;
}

bool ClipboardManager::key_command_up()
{
    if (selectedItem > 0)
    {
        selectedItem--;
        updateConsoleScrollOffset();
        drawConsole();
    }
    return true;
}

bool ClipboardManager::key_command_detect()
{
    if (commandText == "theme")
    {
        // Enter theme selection mode
        commandMode = false;
        cmd_themeSelectMode = true;
        availableThemes = config.discoverThemes();
        // Store original theme and apply first theme for preview
        if (!availableThemes.empty())
        {
            config.originalTheme = config.theme;
            selectedTheme = 0;
            config.switchTheme(availableThemes[0]);
        }
        drawConsole();
        return true;
    }
    if (commandText == "config")
    {
        // Enter config selection mode
        commandMode = false;
        cmd_configSelectMode = true;
        availableConfigs = config.discoverConfigs();
        drawConsole();
        return true;
    }

    // Add space to command text for other commands
    commandText += " ";
    drawConsole();

    return true;
}
