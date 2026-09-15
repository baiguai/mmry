#include "../clipboard_manager.h"

bool ClipboardManager::key_theme_cancel()
{
    if (!config.originalTheme.empty())
    {
        config.switchTheme(config.originalTheme);
    }
    cmd_themeSelectMode = false;
    availableThemes.clear();
    config.originalTheme.clear();
    drawConsole();

    return true;
}

bool ClipboardManager::key_theme_apply()
{
    if (selectedTheme < availableThemes.size())
    {
        config.switchTheme(availableThemes[selectedTheme]);
        // Save to config
        config.saveConfig();
    }
    cmd_themeSelectMode = false;
    availableThemes.clear();
    config.originalTheme.clear();
    drawConsole();

    return true;
}

bool ClipboardManager::key_theme_down()
{
    if (selectedTheme < availableThemes.size() - 1)
    {
        selectedTheme++;
        updateThemeSelectScrollOffset();
        // Apply live preview
        if (selectedTheme < availableThemes.size())
        {
            config.switchTheme(availableThemes[selectedTheme]);
        }
        drawConsole();
    }
    return true;
}

bool ClipboardManager::key_theme_up()
{
    if (selectedTheme > 0)
    {
        selectedTheme--;
        updateThemeSelectScrollOffset();
        // Apply live preview
        if (selectedTheme < availableThemes.size())
        {
            config.switchTheme(availableThemes[selectedTheme]);
        }
        drawConsole();
    }
    return true;
}

bool ClipboardManager::key_theme_top()
{
    selectedTheme = 0;
    themeSelectScrollOffset = 0;
    drawConsole();

    return true;
}

bool ClipboardManager::key_theme_bottom()
{
    if (!availableThemes.empty())
    {
        selectedTheme = availableThemes.size() - 1;
        updateThemeSelectScrollOffset();
        drawConsole();
    }
    return true;
}
