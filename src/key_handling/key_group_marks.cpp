#include "../clipboard_manager.h"

bool ClipboardManager::key_addmarks_add()
{
    std::vector<std::string> displayedGroups;
    if (filterAddBookmarksMode)
    {
        for (const auto& group : bookmarkGroups)
        {
            if (group.find(filterAddBookmarksText) != std::string::npos)
            {
                displayedGroups.push_back(group);
            }
        }
    }
    else
    {
        displayedGroups = bookmarkGroups;
    }

    if (!displayedGroups.empty() && selectedAddBookmarkGroup < displayedGroups.size())
    {
        std::string selectedGroup = displayedGroups[selectedAddBookmarkGroup];
        
        // Check if current clip is already in this group
        if (!items.empty() && selectedItem < getDisplayItemCount())
        {
            size_t actualIndex = getActualItemIndex(selectedItem);
            std::string clipContent = items[actualIndex].content;
            
            // Read existing bookmarks in this group
            std::string bookmarkFile = config.bookmarksDir + "/bookmarks_" + selectedGroup + ".txt";
            std::ifstream file(bookmarkFile);
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
                addClipToBookmarkGroup(selectedGroup, clipContent);
                std::cout << "Added clip to bookmark group: " << selectedGroup << "\n";
            }
            else
            {
                std::cout << "Clip already exists in bookmark group: " << selectedGroup << "\n";
            }
        }
        
        addToBookmarkDialogVisible = false;
        filterAddBookmarksMode = false;
        filterAddBookmarksText.clear();
        drawConsole();
    }
    return true;
}

bool ClipboardManager::key_addmarks_down()
{
    if (!bookmarkGroups.empty() && selectedAddBookmarkGroup < bookmarkGroups.size() - 1)
    {
        selectedAddBookmarkGroup++;
        updateAddBookmarkScrollOffset();
        drawConsole();
    }
    return true;
}

bool ClipboardManager::key_addmarks_up()
{
    if (selectedAddBookmarkGroup > 0)
    {
        selectedAddBookmarkGroup--;
        updateAddBookmarkScrollOffset();
        drawConsole();
    }
    return true;
}

bool ClipboardManager::key_addmarks_top()
{
    selectedAddBookmarkGroup = 0;
    addBookmarkScrollOffset = 0;
    drawConsole();
    return true;
}

bool ClipboardManager::key_addmarks_bottom()
{
    if (!bookmarkGroups.empty())
    {
        selectedAddBookmarkGroup = bookmarkGroups.size() - 1;
        updateAddBookmarkScrollOffset();
        drawConsole();
    }
    return true;
}
