#include "../clipboard_manager.h"

bool ClipboardManager::key_main_delete()
{
    if (!items.empty() && selectedItem < getDisplayItemCount())
    {
        size_t actualIndex = getActualItemIndex(selectedItem);
        items.erase(items.begin() + actualIndex);
        
        // Adjust selection
        size_t displayCount = getDisplayItemCount();
        if (selectedItem >= displayCount && selectedItem > 0)
        {
            selectedItem--;
        }
        
        // Update filtered items if in filter mode
        if (filterMode)
        {
            updateFilteredItems();
        }
        
        saveToFile();
        drawConsole();
    }
    return true;
}

bool ClipboardManager::key_main_filter_start()
{
    filterMode = true;
    filterText = "";
    regexSubmitted = false;
    updateFilteredItems();
    selectedItem = 0;
    drawConsole();

    return true;
}

bool ClipboardManager::key_main_command_start()
{
    commandMode = true;
    commandText = "";
    selectedItem = 0;
    drawConsole();

    return true;
}

bool ClipboardManager::key_main_copy()
{
    if (!items.empty() && selectedItem < getDisplayItemCount())
    {
        size_t actualIndex = getActualItemIndex(selectedItem);
        std::string clipContent = items[actualIndex].content;

        copyToClipboard(clipContent);

        // Update timestamp and move to top if not already at top
        if (actualIndex != 0)
        {
            // Remove from current position
            items.erase(items.begin() + actualIndex);

            // Create new item with current timestamp and insert at top
            items.emplace(items.begin(), ClipboardItem(clipContent));

            // Reset selection to top
            selectedItem = 0;

            // Update filtered items if in filter mode
            if (filterMode)
            {
                updateFilteredItems();
            }

            // Save to file with updated timestamp
            saveToFile();

            std::cout << "Clip moved to top after copying\n";
        }

        int lines = countLines(clipContent);

        if (lines > 1)
        {
            std::cout << "Copied " << lines << " lines to clipboard\n";
        }
        else
        {
            std::cout << "Copied to clipboard: " << clipContent.substr(0, 50) << "...\n";
        }
        hideWindow();
    }
    return true;
}

bool ClipboardManager::key_main_addgroup_start()
{
    bookmarkDialogVisible = true;
    bookmarkDialogInput = "";
    selectedBookmarkGroup = 0;
    bookmarkMgmtScrollOffset = 0; // Reset scroll when opening
    drawConsole();

    return true;
}

bool ClipboardManager::key_main_addclip_start()
{
    if (!bookmarkGroups.empty())
    {
        addToBookmarkDialogVisible = true;
        selectedAddBookmarkGroup = 0;
        addBookmarkScrollOffset = 0; // Reset scroll when opening
        drawConsole();
    }
    return true;
}

bool ClipboardManager::key_main_help_start()
{
    helpDialogVisible = true;
    helpDialogScrollOffset = 0;
    helpFilterMode = true;
    helpFilterText.clear();
    drawConsole();

    return true;
}

bool ClipboardManager::key_main_accessmarks_start()
{
    if (!bookmarkGroups.empty())
    {
        viewBookmarksDialogVisible = true;
        viewBookmarksShowingGroups = true; // Start with group selection
        selectedViewBookmarkGroup = 0;
        selectedViewBookmarkItem = 0;
        viewBookmarksScrollOffset = 0; // Reset scroll when opening
        drawConsole();
    }
    return true;
}

bool ClipboardManager::key_main_pin_clip()
{
    if (!items.empty() && selectedItem < getDisplayItemCount())
    {
        size_t actualIndex = getActualItemIndex(selectedItem);
        std::string clipContent = items[actualIndex].content;
        
        // Read existing bookmarks in this group
        std::ifstream file(config.pinnedFile);
        std::string line;
        bool alreadyExists = false;
        
        while (std::getline(file, line))
        {
            std::string decrypted = decrypt(line, config);
            if (decrypted == clipContent)
            {
                alreadyExists = true;
                break;
            }
        }
        file.close();
        
        if (!alreadyExists)
        {
            addClipToPinned(clipContent);
            std::cout << "Added clip to pinned\n";
        }
        else
        {
            std::cout << "Clip is already pinned\n";
        }
    }
    return true;
}

bool ClipboardManager::key_main_pins_start()
{
    pinnedDialogVisible = true;
    selectedViewPinnedItem = 0;
    viewPinnedScrollOffset = 0; // Reset scroll when opening
    drawConsole();

    return true;
}
