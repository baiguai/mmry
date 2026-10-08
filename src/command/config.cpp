#include "../clipboard_manager.h"

bool ClipboardManager::key_config_cancel()
{
    cmd_configSelectMode = false;
    commandMode = true;
    commandText = "";
    availableConfigs.clear();
    selectedItem = 0;
    drawConsole();
    return true;
}

bool ClipboardManager::key_config_select()
{
    if (selectedConfig < availableConfigs.size())
    {
        std::string configKey = availableConfigs[selectedConfig];
        std::string currentValue = config.getConfigValue(configKey);
        cmd_configSelectMode = false;
        commandText = "config " + configKey + " " + currentValue;
        commandMode = true;
        availableConfigs.clear();
        drawConsole();
    }
    return true;
}

bool ClipboardManager::key_config_down()
{
    if (selectedConfig < availableConfigs.size() - 1)
    {
        selectedConfig++;
        updateConfigSelectScrollOffset();
        drawConsole();
    }
    return true;
}

bool ClipboardManager::key_config_up()
{
    if (selectedConfig > 0)
    {
        selectedConfig--;
        updateConfigSelectScrollOffset();
        drawConsole();
    }
    return true;
}

bool ClipboardManager::key_config_top()
{
    selectedConfig = 0;
    configSelectScrollOffset = 0;
    drawConsole();
    return true;
}

bool ClipboardManager::key_config_bottom()
{
    if (!availableConfigs.empty())
    {
        selectedConfig = availableConfigs.size() - 1;
        updateConfigSelectScrollOffset();
        drawConsole();
    }
    return true;
}
