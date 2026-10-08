#include "../clipboard_manager.h"

bool ClipboardManager::key_help_hide()
{
    helpDialogVisible = false;
    drawConsole();
    return true;
}

bool ClipboardManager::key_help_scroll_down()
{
    updateHelpDialogScrollOffset(-1);
    drawConsole();
    return true;
}

bool ClipboardManager::key_help_scroll_up()
{
    if (helpDialogScrollOffset > 0)
    {
        updateHelpDialogScrollOffset(1);
        drawConsole();
    }
    return true;
}

bool ClipboardManager::key_help_scroll_top()
{
    helpDialogScrollOffset = 0;
    drawConsole();
    return true;
}
