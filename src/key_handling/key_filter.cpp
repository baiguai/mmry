#include "../clipboard_manager.h"

bool ClipboardManager::key_filter_delete()
{
    if (!items.empty() && selectedItem < getDisplayItemCount())
    {
        size_t actualIndex = getActualItemIndex(selectedItem);
        items.erase(items.begin() + actualIndex);
        
        // Update filtered items after deletion
        updateFilteredItems();
        
        // Adjust selection if needed
        if (selectedItem >= getDisplayItemCount() && selectedItem > 0)
        {
            selectedItem--;
        }
        
        // Save changes and redraw
        saveToFile();
        drawConsole();
    }
    return true;
}

bool ClipboardManager::key_filter_copy()
{
    if (!items.empty() && selectedItem < getDisplayItemCount())
    {
        size_t actualIndex = getActualItemIndex(selectedItem);
        copyToClipboard(items[actualIndex].content);
        int lines = countLines(items[actualIndex].content);
        if (lines > 1)
        {
            std::cout << "Copied " << lines << " lines to clipboard" << "\n";
        }
        else
        {
            std::cout << "Copied to clipboard: " << items[actualIndex].content.substr(0, 50) << "...\n";
        }
        filterMode = false;
        filterText = "";
        filteredItems.clear();
        hideWindow();
    }
    return true;
}

bool ClipboardManager::key_filter_down()
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

bool ClipboardManager::key_filter_up()
{
    if (selectedItem > 0)
    {
        selectedItem--;
        updateConsoleScrollOffset();
        drawConsole();
    }
    return true;
}
