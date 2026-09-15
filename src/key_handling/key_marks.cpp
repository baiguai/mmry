#include "../clipboard_manager.h"

bool ClipboardManager::key_marks_show()
{
    if (!viewBookmarksShowingGroups)
    {
        // If viewing clips, go back to groups
        viewBookmarksShowingGroups = true;
        selectedViewBookmarkItem = 0;
        viewBookmarksScrollOffset = 0; // Reset scroll when going back
    }
    else
    {
        // If viewing groups, close dialog
        viewBookmarksDialogVisible = false;
    }
    drawConsole();

    return true;
}

bool ClipboardManager::key_marks_groups_down()
{
    if (selectedViewBookmarkGroup < bookmarkGroups.size() - 1)
    {
        selectedViewBookmarkGroup++;
        updateScrollOffset();
        drawConsole();
    }
    return true;
}

bool ClipboardManager::key_marks_groups_up()
{
    if (selectedViewBookmarkGroup > 0)
    {
        selectedViewBookmarkGroup--;
        updateScrollOffset();
        drawConsole();
    }
    return true;
}

bool ClipboardManager::key_marks_groups_top()
{
    selectedViewBookmarkGroup = 0;
    viewBookmarksScrollOffset = 0;
    drawConsole();
    return true;
}

bool ClipboardManager::key_marks_groups_bottom()
{
    selectedViewBookmarkGroup = bookmarkGroups.size() - 1;
    updateScrollOffset();
    drawConsole();
    return true;
}

bool ClipboardManager::key_marks_groups_delete()
{
    // Determine which context is active: the "Add to bookmark" dialog
    // tracks selection with selectedAddBookmarkGroup, while the
    // view-bookmarks groups view uses selectedViewBookmarkGroup.
    std::vector<std::string> displayedGroups;
    size_t selectedIndex { 0 };

    if (addToBookmarkDialogVisible)
    {
        if (filterAddBookmarksMode)
        {
            std::string lowerFilter { stringToLower(filterAddBookmarksText) };
            for (const auto& group : bookmarkGroups)
            {
                if (stringToLower(group).find(lowerFilter) != std::string::npos)
                {
                    displayedGroups.push_back(group);
                }
            }
        }
        else
        {
            displayedGroups = bookmarkGroups;
        }
        selectedIndex = selectedAddBookmarkGroup;
    }
    else
    {
        if (filterBookmarksMode)
        {
            std::string lowerFilter { stringToLower(filterBookmarksText) };
            for (const auto& group : bookmarkGroups)
            {
                if (stringToLower(group).find(lowerFilter) != std::string::npos)
                {
                    displayedGroups.push_back(group);
                }
            }
        }
        else
        {
            displayedGroups = bookmarkGroups;
        }
        selectedIndex = selectedViewBookmarkGroup;
    }

    if (displayedGroups.empty() || selectedIndex >= displayedGroups.size())
    {
        return true;
    }

    // Map the displayed selection back to the actual group entry
    std::string groupToDelete { displayedGroups[selectedIndex] };
    auto it = std::find(bookmarkGroups.begin(), bookmarkGroups.end(), groupToDelete);
    if (it == bookmarkGroups.end())
    {
        return true;
    }
    size_t actualIndex { static_cast<size_t>(std::distance(bookmarkGroups.begin(), it)) };

    // Remove group from list
    bookmarkGroups.erase(bookmarkGroups.begin() + actualIndex);
    saveBookmarkGroups();

    // Delete bookmark file
    std::string bookmarkFile { config.bookmarksDir + "/bookmarks_" + groupToDelete + ".txt" };
    unlink(bookmarkFile.c_str());

    std::cout << "Deleted bookmark group and all clips: " << groupToDelete << "\n";

    // Adjust selection
    if (selectedIndex > 0 && selectedIndex >= bookmarkGroups.size())
    {
        size_t lastIndex = bookmarkGroups.empty() ? 0 : bookmarkGroups.size() - 1;
        if (addToBookmarkDialogVisible)
        {
            selectedAddBookmarkGroup = lastIndex;
        }
        else
        {
            selectedViewBookmarkGroup = lastIndex;
        }
    }

    // Close dialog if no groups left
    if (bookmarkGroups.empty())
    {
        if (addToBookmarkDialogVisible)
        {
            addToBookmarkDialogVisible = false;
        }
        else
        {
            viewBookmarksDialogVisible = false;
        }
    }

    drawConsole();
    return true;
}

bool ClipboardManager::key_marks_groups_clips()
{
    std::vector<std::string> displayedGroups;
    if (filterBookmarksMode)
    {
        std::string lowerFilterBookmarksText = stringToLower(filterBookmarksText);
        for (const auto& group : bookmarkGroups)
        {
            if (stringToLower(group).find(lowerFilterBookmarksText) != std::string::npos)
            {
                displayedGroups.push_back(group);
            }
        }
    }
    else
    {
        displayedGroups = bookmarkGroups;
    }

    if (selectedViewBookmarkGroup < displayedGroups.size())
    {
        // Find the actual index in the original bookmarkGroups vector
        std::string selectedGroupName = displayedGroups[selectedViewBookmarkGroup];
        auto it = std::find(bookmarkGroups.begin(), bookmarkGroups.end(), selectedGroupName);
        if (it != bookmarkGroups.end())
        {
            selectedViewBookmarkGroup = std::distance(bookmarkGroups.begin(), it);
            
            viewBookmarksShowingGroups = false;
            selectedViewBookmarkItem = 0;
            viewBookmarksScrollOffset = 0; // Reset scroll when switching modes
            
            // Exit filter mode
            filterBookmarksMode = false;
            filterBookmarksText.clear();

            drawConsole();
        }
    }
    return true;
}

bool ClipboardManager::key_marks_clips_down()
{
    size_t currentItemCount = filterBookmarkClipsMode ? filteredBookmarkClips.size() : getBookmarkItemCount();
    if (selectedViewBookmarkItem < currentItemCount - 1)
    {
        selectedViewBookmarkItem++;
        updateScrollOffset();
        drawConsole();
    }
    return true;
}

bool ClipboardManager::key_marks_clips_up()
{
    if (selectedViewBookmarkItem > 0)
    {
        selectedViewBookmarkItem--;
        updateScrollOffset();
        drawConsole();
    }
    return true;
}

bool ClipboardManager::key_marks_clips_top()
{
    selectedViewBookmarkItem = 0;
    viewBookmarksScrollOffset = 0;
    drawConsole();
    return true;
}

bool ClipboardManager::key_marks_clips_bottom()
{
    size_t currentItemCount = filterBookmarkClipsMode ? filteredBookmarkClips.size() : getBookmarkItemCount();
    if (currentItemCount > 0)
    {
        selectedViewBookmarkItem = currentItemCount - 1;
        updateScrollOffset();
        drawConsole();
    }
    return true;
}

bool ClipboardManager::key_marks_clips_delete()
{
    if (selectedViewBookmarkGroup < bookmarkGroups.size())
    {
        std::string selectedGroup = bookmarkGroups[selectedViewBookmarkGroup];
        std::string bookmarkFile = config.bookmarksDir + "/bookmarks_" + selectedGroup + ".txt";
        std::ifstream file(bookmarkFile);
        
        if (file.is_open())
        {
            std::string line;
            std::vector<std::string> lines;
            std::vector<std::string> actualBookmarkItems; // To hold decrypted items

            // Read all lines and decrypt for comparison/deletion
            while (std::getline(file, line))
            {
                lines.push_back(line); // Store original line with timestamp
                size_t pos = line.find('|');
                if (pos != std::string::npos && pos > 0)
                {
                    std::string content = line.substr(pos + 1);
                    try
                    {
                        actualBookmarkItems.push_back(decrypt(content, config));
                    }
                    catch (...)
                    {
                        actualBookmarkItems.push_back(content);
                    }
                }
            }
            file.close();

            std::string itemToDeleteContent;
            size_t actualIndexToDelete = -1;

            if (filterBookmarkClipsMode)
            {
                if (selectedViewBookmarkItem < filteredBookmarkClips.size())
                {
                    itemToDeleteContent = filteredBookmarkClips[selectedViewBookmarkItem];
                    // Find the index of this item in the original actualBookmarkItems
                    for (size_t i = 0; i < actualBookmarkItems.size(); ++i)
                    {
                        if (actualBookmarkItems[i] == itemToDeleteContent)
                        {
                            actualIndexToDelete = i;
                            break;
                        }
                    }
                }
            }
            else
            {
                if (selectedViewBookmarkItem < actualBookmarkItems.size())
                {
                    actualIndexToDelete = selectedViewBookmarkItem;
                }
            }
            
            // Remove the selected item if valid
            if (actualIndexToDelete != (size_t)-1 && actualIndexToDelete < lines.size())
            {
                lines.erase(lines.begin() + actualIndexToDelete);
                
                // Write back remaining lines
                std::ofstream outFile(bookmarkFile);
                if (outFile.is_open())
                {
                    for (const auto& l : lines)
                    {
                        outFile << l << "\n";
                    }
                    outFile.close();
                    
                    std::cout << "Deleted bookmark item from group: " << selectedGroup << "\n";
                    
                    // Update filter if active
                    if (filterBookmarkClipsMode)
                    {
                        updateFilteredBookmarkClips();
                    }

                    // Adjust selection
                    size_t currentItemCount = filterBookmarkClipsMode ? filteredBookmarkClips.size() : getBookmarkItemCount();
                    if (selectedViewBookmarkItem > 0 && selectedViewBookmarkItem >= currentItemCount)
                    {
                        selectedViewBookmarkItem = currentItemCount - 1;
                    }
                    if (currentItemCount == 0)
                    {
                        selectedViewBookmarkItem = 0; // Reset if list becomes empty
                    }
                    
                    drawConsole();
                }
            }
        }
    }
    return true;
}

bool ClipboardManager::key_marks_clips_copy()
{
    if (filterBookmarkClipsMode)
    {
        if (selectedViewBookmarkItem < filteredBookmarkClips.size())
        {
            copyToClipboard(filteredBookmarkClips[selectedViewBookmarkItem]);
            int lines = countLines(filteredBookmarkClips[selectedViewBookmarkItem]);
            if (lines > 1)
            {
                std::cout << "Copied " << lines << " lines from bookmark" << "\n";
            }
            else
            {
                std::cout << "Copied from bookmark: " << filteredBookmarkClips[selectedViewBookmarkItem].substr(0, 50) << "..." << "\n";
            }
            viewBookmarksDialogVisible = false;
            hideWindow();
        }
        // Clear filter if active
        filterBookmarkClipsMode = false;
        filterBookmarkClipsText.clear();
        return true;
    }
    else
    {
        if (selectedViewBookmarkGroup < bookmarkGroups.size())
        {
            std::string selectedGroup = bookmarkGroups[selectedViewBookmarkGroup];
            std::string bookmarkFile = config.bookmarksDir + "/bookmarks_" + selectedGroup + ".txt";
            std::ifstream file(bookmarkFile);
            
            if (file.is_open())
            {
                std::string line;
                std::vector<std::string> bookmarkItems;
                
                while (std::getline(file, line))
                {
                    size_t pos = line.find('|');
                    if (pos != std::string::npos && pos > 0)
                    {
                        std::string content = line.substr(pos + 1);
                        try
                        {
                            std::string decryptedContent = decrypt(content, config);
                            bookmarkItems.push_back(decryptedContent);
                        }
                        catch (...)
                        {
                            bookmarkItems.push_back(content);
                        }
                    }
                }
                file.close();
                
                if (selectedViewBookmarkItem < bookmarkItems.size())
                {
                    copyToClipboard(bookmarkItems[selectedViewBookmarkItem]);
                    int lines = countLines(bookmarkItems[selectedViewBookmarkItem]);
                    if (lines > 1)
                    {
                        std::cout << "Copied " << lines << " lines from bookmark" << "\n";
                    }
                    else
                    {
                        std::cout << "Copied from bookmark: " << bookmarkItems[selectedViewBookmarkItem].substr(0, 50) << "..." << "\n";
                    }
                    viewBookmarksDialogVisible = false;
                    hideWindow();
                }
            }
        }
        return true;
    }
}

bool ClipboardManager::key_marks_clips_groups()
{
    viewBookmarksShowingGroups = true;
    drawConsole();
    return true;
}
