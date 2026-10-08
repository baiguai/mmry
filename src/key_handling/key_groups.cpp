#include "../clipboard_manager.h"

bool ClipboardManager::key_addgroup_add()
{
    // Enter creates/selects bookmark group using input text only
    if (!bookmarkDialogInput.empty())
    {
        // Check if this is a new group
        bool groupExists = false;
        for (const auto& group : bookmarkGroups)
        {
            if (group == bookmarkDialogInput)
            {
                groupExists = true;
                break;
            }
        }
        
        if (!groupExists)
        {
            // Create new group and add current clip
            bookmarkGroups.push_back(bookmarkDialogInput);
            saveBookmarkGroups();
            
            // Add current clip to bookmark
            if (!items.empty() && selectedItem < getDisplayItemCount())
            {
                size_t actualIndex = getActualItemIndex(selectedItem);
                addClipToBookmarkGroup(bookmarkDialogInput, items[actualIndex].content);
                std::cout << "Added clip to bookmark group: " << bookmarkDialogInput << "\n";
            }
        }
        else
        {
            // Add current clip to existing group
            if (!items.empty() && selectedItem < getDisplayItemCount())
            {
                size_t actualIndex = getActualItemIndex(selectedItem);
                addClipToBookmarkGroup(bookmarkDialogInput, items[actualIndex].content);
                std::cout << "Added clip to bookmark group: " << bookmarkDialogInput << "\n";
            }
        }
        
        bookmarkDialogVisible = false;
        drawConsole();
    }
    return true;
}

bool ClipboardManager::key_addgroup_back()
{
    if (!bookmarkDialogInput.empty())
    {
        bookmarkDialogInput.pop_back();
        drawConsole();
    }
    return true;
}
