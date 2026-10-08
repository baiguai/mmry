#include "../clipboard_manager.h"

void ClipboardManager::updateScrollOffset()
{
    DialogDimensions dims = calculateDialogDimensions(windowWidth, windowHeight, 600, 500);
    const int ITEM_LINE_HEIGHT = 25; // Now uses LINE_HEIGHT
    
    // This 'y' is the starting point of the list within the dialog
    // This needs to match the y = dims.y + 60 in drawViewBookmarksDialog
    const int LIST_START_OFFSET_Y = 60; 

    // Calculate how many items can actually fit in the scrollable area
    // dims.contentHeight is the total content area. Subtract the space taken by the header.
    int availableHeightForScrollableItems = dims.contentHeight - LIST_START_OFFSET_Y;
    
    int dynamicVisibleItems = std::max(1, availableHeightForScrollableItems / ITEM_LINE_HEIGHT);

    if (viewBookmarksShowingGroups)
    {
        // Scrolling for groups
        if (selectedViewBookmarkGroup < viewBookmarksScrollOffset)
        {
            viewBookmarksScrollOffset = selectedViewBookmarkGroup;
        }
        else if (selectedViewBookmarkGroup >= viewBookmarksScrollOffset + dynamicVisibleItems)
        {
            viewBookmarksScrollOffset = selectedViewBookmarkGroup - dynamicVisibleItems + 1;
        }
    }
    else
    {
        // Scrolling for clips
        size_t currentItemCount = filterBookmarkClipsMode ? filteredBookmarkClips.size() : getBookmarkItemCount();
        
        if (selectedViewBookmarkItem < viewBookmarksScrollOffset)
        {
            viewBookmarksScrollOffset = selectedViewBookmarkItem;
        }
        else if (selectedViewBookmarkItem >= viewBookmarksScrollOffset + dynamicVisibleItems)
        {
            viewBookmarksScrollOffset = selectedViewBookmarkItem - dynamicVisibleItems + 1;
        }

        // Ensure scroll offset does not exceed available items
        if (currentItemCount == 0)
        {
            viewBookmarksScrollOffset = 0;
        }
        else if (viewBookmarksScrollOffset + dynamicVisibleItems > currentItemCount)
        {
            viewBookmarksScrollOffset = std::max(0, (int)currentItemCount - dynamicVisibleItems);
        }
    }
}

void ClipboardManager::updateEditDialogScrollOffset()
{
    DialogDimensions dims = calculateDialogDimensions(windowWidth, windowHeight, 600, 400);
    const int lineHeight = 15;
    const int charWidth = 8;
    int maxCharsPerLine = (dims.width - 50) / charWidth;
    if (maxCharsPerLine < 1) maxCharsPerLine = 1;

    // Calculate the visual line number of the cursor
    int cursorVisualLine = 0;
    std::istringstream iss(editDialogInput);
    std::string logicalLine;
    for (int i = 0; i < (int)editDialogCursorLine; ++i)
    {
        if (!std::getline(iss, logicalLine)) break;
        if (logicalLine.empty())
        {
            cursorVisualLine++;
        }
        else
        {
            cursorVisualLine += (logicalLine.length() + maxCharsPerLine - 1) / maxCharsPerLine;
        }
    }
    // Add visual lines from the current logical line, up to the cursor
    cursorVisualLine += editDialogCursorPos / maxCharsPerLine;

    // Calculate max visible lines
    int maxVisibleLines = (dims.height - 70 - 15) / lineHeight; // 70 for header/footer, 15 for some padding
    if (maxVisibleLines < 1) maxVisibleLines = 1;

    // Adjust scroll offset
    if (cursorVisualLine < editDialogScrollOffset)
    {
        editDialogScrollOffset = cursorVisualLine;
    }
    else if (cursorVisualLine >= editDialogScrollOffset + maxVisibleLines)
    {
        editDialogScrollOffset = cursorVisualLine - maxVisibleLines + 1;
    }

    // Clamp scroll offset
    if (editDialogScrollOffset < 0)
    {
        editDialogScrollOffset = 0;
    }
}

void ClipboardManager::updateConsoleScrollOffset()
{
    const int SCROLL_INDICATOR_HEIGHT = 15; // Height reserved for scroll indicator
    
    // Calculate starting Y position (accounting for filter, command, or theme selection mode)
    int startY = (filterMode || commandMode || cmd_themeSelectMode || cmd_configSelectMode) ? 45 : 20;
    
    // Calculate available height for items
    int availableHeight = windowHeight - startY - 10; // 10px bottom margin
    
    size_t displayCount = getDisplayItemCount();
    
    // Calculate how many items can fit
    int maxVisibleItems = availableHeight / LINE_HEIGHT;
    // logfile << "updateConsoleScrollOffset: windowHeight=" << windowHeight << ", availableHeight=" << availableHeight << ", LINE_HEIGHT=" << LINE_HEIGHT << ", maxVisibleItems (initial)=" << maxVisibleItems << std::endl;
    
    // If we have more items than fit, reserve space for scroll indicator
    if (static_cast<int>(displayCount) > maxVisibleItems)
    {
        availableHeight -= SCROLL_INDICATOR_HEIGHT;
        maxVisibleItems = availableHeight / LINE_HEIGHT;
    }

    if (maxVisibleItems > 0) maxVisibleItems += 1;
    
    // Ensure we show at least 1 item
    if (maxVisibleItems < 1)
    {
        maxVisibleItems = 1;
    }
    // logfile << "updateConsoleScrollOffset: displayCount=" << displayCount << ", maxVisibleItems (final)=" << maxVisibleItems << ", selectedItem=" << selectedItem << ", consoleScrollOffset=" << consoleScrollOffset << std::endl;
    
    // Update scroll offset to keep selected item visible
    if (selectedItem < consoleScrollOffset)
    {
        consoleScrollOffset = selectedItem;
    }
    else if (selectedItem >= consoleScrollOffset + maxVisibleItems)
    {
        consoleScrollOffset = selectedItem - maxVisibleItems + 1;
    }
}

void ClipboardManager::updateBookmarkMgmtScrollOffset()
{
    const int VISIBLE_ITEMS = 10; // Number of groups visible in bookmark management dialog
    
    // Filter groups for scroll calculation
    std::vector<std::string> filteredGroups;
    for (const auto& group : bookmarkGroups)
    {
        if (bookmarkDialogInput.empty() || group.find(bookmarkDialogInput) != std::string::npos)
        {
            filteredGroups.push_back(group);
        }
    }
    
    if (selectedBookmarkGroup < bookmarkMgmtScrollOffset)
    {
        bookmarkMgmtScrollOffset = selectedBookmarkGroup;
    }
    else if (selectedBookmarkGroup >= bookmarkMgmtScrollOffset + VISIBLE_ITEMS)
    {
        bookmarkMgmtScrollOffset = selectedBookmarkGroup - VISIBLE_ITEMS + 1;
    }
}

void ClipboardManager::updatePinnedScrollOffset()
{
    if (selectedViewPinnedItem < viewPinnedScrollOffset)
    {
        viewPinnedScrollOffset = selectedViewPinnedItem;
    }
    else if (selectedViewPinnedItem >= viewPinnedScrollOffset + m_maxVisiblePinnedItems)
    {
        viewPinnedScrollOffset = selectedViewPinnedItem - m_maxVisiblePinnedItems + 1;
    }
}

void ClipboardManager::updateAddBookmarkScrollOffset()
{
    const int VISIBLE_ITEMS = 10; // Number of groups visible in add bookmark dialog
    
    if (selectedAddBookmarkGroup < addBookmarkScrollOffset)
    {
        addBookmarkScrollOffset = selectedAddBookmarkGroup;
    }
    else if (selectedAddBookmarkGroup >= addBookmarkScrollOffset + VISIBLE_ITEMS)
    {
        addBookmarkScrollOffset = selectedAddBookmarkGroup - VISIBLE_ITEMS + 1;
    }
}

void ClipboardManager::updateThemeSelectScrollOffset()
{
    const int VISIBLE_ITEMS = 10; // Number of themes visible in theme selection
    
    if (selectedTheme < themeSelectScrollOffset)
    {
        themeSelectScrollOffset = selectedTheme;
    }
    else if (selectedTheme >= themeSelectScrollOffset + VISIBLE_ITEMS)
    {
        themeSelectScrollOffset = selectedTheme - VISIBLE_ITEMS + 1;
    }
}

void ClipboardManager::updateConfigSelectScrollOffset()
{
    const int VISIBLE_ITEMS = 10; // Number of configs visible in config selection
    
    if (selectedConfig < configSelectScrollOffset)
    {
        configSelectScrollOffset = selectedConfig;
    }
    else if (selectedConfig >= configSelectScrollOffset + VISIBLE_ITEMS)
    {
        configSelectScrollOffset = selectedConfig - VISIBLE_ITEMS + 1;
    }
}

void ClipboardManager::updateHelpDialogScrollOffset(int adjustment)
{
    const int STEP = 10;
   
    helpDialogScrollOffset = helpDialogScrollOffset + (adjustment * STEP);

    if (static_cast<int>(helpDialogScrollOffset) > -1)
    {
        helpDialogScrollOffset = 0;
    }
}
