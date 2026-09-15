#include "../clipboard_manager.h"

bool ClipboardManager::key_global_escape()
{
    if (editDialogVisible)
    {
        if (key_edit_escape()) return true;
    }
    if (filterBookmarksMode)
    {
        filterBookmarksMode = false;
        filterBookmarksText.clear();
        drawConsole();
        return true;
    }
    if (filterAddBookmarksMode)
    {
        filterAddBookmarksMode = false;
        filterAddBookmarksText.clear();
        drawConsole();
        return true;
    }
    if (filterBookmarkClipsMode)
    {
        filterBookmarkClipsMode = false;
        filterBookmarkClipsText.clear();
        drawConsole();
        return true;
    }
    if (pinnedDialogVisible)
    {
        pinnedDialogVisible = false;
        drawConsole();
    }
    else if (bookmarkDialogVisible)
    {
        // Escape hides dialog but not window
        bookmarkDialogVisible = false;
        drawConsole();
    }
    else if (addToBookmarkDialogVisible)
    {
        // Escape hides dialog but not window
        addToBookmarkDialogVisible = false;
        drawConsole();
    }
    else if (helpDialogVisible)
    {
        // Escape hides help dialog but not window
        if (helpFilterMode)
        {
            helpFilterMode = false;
            helpFilterText.clear();
            helpDialogScrollOffset = 0;
            drawConsole();
            return true;
        }
        helpDialogVisible = false;
        helpDialogScrollOffset = 0;
        drawConsole();
    }
    else if (viewBookmarksDialogVisible)
    {
        // Escape hides view bookmarks dialog but not window
        viewBookmarksDialogVisible = false;
        drawConsole();
    }
    else if (filterMode)
    {
        // Escape exits filter mode but doesn't hide window
        filterMode = false;
        filterText = "";
        filteredItems.clear();
        selectedItem = 0;
        drawConsole();
    }
    else if (commandMode)
    {
        // Escape exits command mode but doesn't hide window
        commandMode = false;
        commandText = "";
        selectedItem = 0;
        drawConsole();
    }
    else if (cmd_themeSelectMode)
    {
        // Restore original theme and exit theme selection mode but doesn't hide window
        if (!config.originalTheme.empty())
        {
            config.switchTheme(config.originalTheme);
        }
        cmd_themeSelectMode = false;
        availableThemes.clear();
        config.originalTheme.clear();
        selectedItem = 0;
        drawConsole();
    }
    else if (cmd_configSelectMode)
    {
        // Exit config selection mode and return to command mode
        cmd_configSelectMode = false;
        commandMode = true;
        commandText = "";
        availableConfigs.clear();
        selectedItem = 0;
        drawConsole();
    }
    else
    {
        // Normal escape behavior - hide window
        hideWindow();
    }

    return true;
}