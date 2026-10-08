#include "../clipboard_manager.h"

bool ClipboardManager::key_main_down()
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

bool ClipboardManager::key_main_up()
{
    if (selectedItem > 0)
    {
        selectedItem--;
        updateConsoleScrollOffset();
        drawConsole();
    }
    return true;
}

bool ClipboardManager::key_main_top()
{
    selectedItem = 0;
    updateConsoleScrollOffset();
    drawConsole();

    return true;
}

bool ClipboardManager::key_main_bottom()
{
    size_t displayCount = getDisplayItemCount();
    if (displayCount > 0)
    {
        selectedItem = displayCount - 1;
        updateConsoleScrollOffset();
        drawConsole();
    }
    return true;
}
