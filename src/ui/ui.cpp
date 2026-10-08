#include "../clipboard_manager.h"

// Linux UI Methods
// !@!
#ifdef __linux__
    void ClipboardManager::drawConsole()
    {
        if (!visible) return;
        
        // Clear window with theme background
        XSetWindowBackground(display, window, config.backgroundColor);
        XClearWindow(display, window);
        
        // Build console draw data
        ConsoleDrawData data;
        data.filterMode = filterMode;
        data.filterText = filterText;
        data.commandMode = commandMode;
        data.commandText = commandText;
        data.themeSelectMode = cmd_themeSelectMode;
        data.configSelectMode = cmd_configSelectMode;
        
        if (cmd_themeSelectMode)
        {
            data.themeItems = availableThemes;
            data.selectedTheme = selectedTheme;
            data.themeScrollOffset = themeSelectScrollOffset;
        }
        if (cmd_configSelectMode)
        {
            data.configItems = availableConfigs;
            data.selectedConfig = selectedConfig;
            data.configScrollOffset = configSelectScrollOffset;
        }
        
        data.startY = 20;
        data.windowWidth = windowWidth;
        data.windowHeight = windowHeight;
        data.lineHeight = LINE_HEIGHT;
        data.clipListWidth = clipListWidth;
        data.bgColor = config.backgroundColor;
        data.textColor = config.textColor;
        data.selColor = config.selectionColor;
        
        // Build clip display lines
        if (!cmd_themeSelectMode && !cmd_configSelectMode)
        {
            size_t displayCount = filterMode ? filteredItems.size() : items.size();
            data.totalClipCount = displayCount;
            data.selectedItem = selectedItem;
            data.clipScrollOffset = consoleScrollOffset;
            
            int availableHeight = windowHeight - data.startY - 10;
            const int SCROLL_INDICATOR_HEIGHT = 15;
            
            if (static_cast<int>(displayCount) > (availableHeight / LINE_HEIGHT))
            {
                availableHeight -= SCROLL_INDICATOR_HEIGHT;
            }
            
            int maxItems = availableHeight / LINE_HEIGHT;
            if (maxItems > 0) maxItems += 1;
            if (maxItems < 1) maxItems = 1;
            
            size_t endIdx = std::min(consoleScrollOffset + maxItems, displayCount);
            
            for (size_t i = consoleScrollOffset; i < endIdx; ++i)
            {
                size_t actualIndex = filterMode ? filteredItems[i] : i;
                const auto& item = items[actualIndex];
                
                std::string line;
                if (i == selectedItem)
                {
                    line = "> ";
                }
                else
                {
                    line = "  ";
                }
                
                if (config.verboseMode)
                {
                    auto time_t = std::chrono::system_clock::to_time_t(item.timestamp);
                    auto tm = *std::localtime(&time_t);
                    
                    std::ostringstream timeStream;
                    timeStream << std::put_time(&tm, "%H:%M:%S");
                    
                    size_t lineCount = 1;
                    for (char c : item.content)
                    {
                        if (c == '\n') lineCount++;
                    }
                    
                    line += timeStream.str() + " | " + std::to_string(lineCount) + " lines | ";
                    
                    std::string content = item.content;
                    int maxContentLength = calculateMaxContentLength(clipListWidth, true);
                    if (static_cast<int>(content.length()) > maxContentLength)
                    {
                        content = smartTrim(content, maxContentLength);
                    }
                    
                    for (char& c : content)
                    {
                        if (c == '\n' || c == '\r') c = ' ';
                    }
                    
                    line += content;
                }
                else
                {
                    size_t lineCount = 1;
                    for (char c : item.content)
                    {
                        if (c == '\n') lineCount++;
                    }
                    
                    std::string content = item.content;
                    int maxContentLength = calculateMaxContentLength(clipListWidth, false);
                    if (static_cast<int>(content.length()) > maxContentLength)
                    {
                        content = smartTrim(content, maxContentLength);
                    }
                    
                    for (char& c : content)
                    {
                        if (c == '\n' || c == '\r') c = ' ';
                    }
                    
                    line += content;
                    
                    if (lineCount > 1)
                    {
                        line += " (" + std::to_string(lineCount) + " lines)";
                    }
                }
                
                data.clipLines.push_back(line);
            }
        }
        
        ::drawConsole(display, window, gc, data);
        
        // Draw dialogs if visible
        if (bookmarkDialogVisible)
        {
            DialogDimensions dims = calculateDialogDimensions(windowWidth, windowHeight, 400, 300);
            std::vector<std::string> filteredGroups;
            for (const auto& group : bookmarkGroups)
            {
                if (bookmarkDialogInput.empty() || group.find(bookmarkDialogInput) != std::string::npos)
                {
                    filteredGroups.push_back(group);
                }
            }
            drawBookmarkDialog(display, window, gc, font, dims,
                             bookmarkDialogInput, filteredGroups,
                             selectedBookmarkGroup, bookmarkMgmtScrollOffset,
                             config.backgroundColor, config.textColor, config.selectionColor, config.borderColor);
        }
        if (addToBookmarkDialogVisible)
        {
            DialogDimensions dims = calculateDialogDimensions(windowWidth, windowHeight, 400, 300);
            std::vector<std::string> displayedGroups;
            if (filterAddBookmarksMode)
            {
                std::string lowerFilterText = stringToLower(filterAddBookmarksText);
                for (const auto& group : bookmarkGroups)
                {
                    if (stringToLower(group).find(lowerFilterText) != std::string::npos)
                    {
                        displayedGroups.push_back(group);
                    }
                }
            }
            else
            {
                displayedGroups = bookmarkGroups;
            }
            if (selectedAddBookmarkGroup >= displayedGroups.size() && !displayedGroups.empty())
            {
                selectedAddBookmarkGroup = displayedGroups.size() - 1;
            }

            drawAddToBookmarkDialog(display, window, gc, font, dims,
                                  displayedGroups,
                                  selectedAddBookmarkGroup, addBookmarkScrollOffset,
                                  filterAddBookmarksMode, filterAddBookmarksText,
                                  config.backgroundColor, config.textColor, config.selectionColor, config.borderColor);
        }
        if (viewBookmarksDialogVisible)
        {
            DialogDimensions dims = calculateDialogDimensions(windowWidth, windowHeight, 600, 500);
            std::string title;
            std::vector<std::string> items;
            size_t selItem = 0;
            size_t scrollOff = 0;
            bool filterActive = false;
            std::string filterTxt;
            int itemLH = 18;
            std::string emptyMsg;
            if (viewBookmarksShowingGroups)
            {
                title = "Select Bookmark Group";
                if (filterBookmarksMode)
                {
                    std::string lowerFilter = stringToLower(filterBookmarksText);
                    for (const auto& group : bookmarkGroups)
                    {
                        if (stringToLower(group).find(lowerFilter) != std::string::npos)
                        {
                            items.push_back(group);
                        }
                    }
                    filterActive = true;
                    filterTxt = filterBookmarksText;
                }
                else
                {
                    items = bookmarkGroups;
                }
                if (selectedViewBookmarkGroup >= items.size() && !items.empty())
                {
                    selectedViewBookmarkGroup = items.size() - 1;
                }
                selItem = selectedViewBookmarkGroup;
                scrollOff = viewBookmarksScrollOffset;
            }
            else
            {
                if (selectedViewBookmarkGroup < bookmarkGroups.size())
                {
                    title = "View Bookmarks: " + bookmarkGroups[selectedViewBookmarkGroup];
                }
                else
                {
                    title = "View Bookmarks";
                }
                if (filterBookmarkClipsMode)
                {
                    items = filteredBookmarkClips;
                    filterActive = true;
                    filterTxt = filterBookmarkClipsText;
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
                            while (std::getline(file, line))
                            {
                                size_t pos = line.find('|');
                                if (pos != std::string::npos && pos > 0)
                                {
                                    std::string content = line.substr(pos + 1);
                                    try
                                    {
                                        items.push_back(decrypt(content, config));
                                    }
                                    catch (...)
                                    {
                                        items.push_back(content);
                                    }
                                }
                            }
                            file.close();
                        }
                    }
                }
                int maxContentLength = calculateDialogContentLength(dims);
                for (auto& item : items)
                {
                    if (static_cast<int>(item.length()) > maxContentLength)
                    {
                        item = smartTrim(item, maxContentLength);
                    }
                    for (char& c : item)
                    {
                        if (c == '\n' || c == '\r') c = ' ';
                    }
                }
                if (selectedViewBookmarkItem >= items.size() && !items.empty())
                {
                    selectedViewBookmarkItem = items.size() - 1;
                }
                selItem = selectedViewBookmarkItem;
                scrollOff = viewBookmarksScrollOffset;
                itemLH = LINE_HEIGHT;
                emptyMsg = "No bookmarks in this group";
            }

            drawViewBookmarksDialog(display, window, gc, font, dims,
                                  title, items, selItem, scrollOff,
                                  filterActive, filterTxt, itemLH, emptyMsg,
                                  config.backgroundColor, config.textColor, config.selectionColor, config.borderColor);
        }
        if (pinnedDialogVisible)
        {
            auto sortedItems = getSortedPinnedItems(config.pinnedFile);
            std::vector<std::pair<long long, std::string>> displayItems;
            for (const auto& line : sortedItems)
            {
                size_t pos = line.find('|');
                if (pos != std::string::npos && pos > 0)
                {
                    std::string timestampStr = line.substr(0, pos);
                    std::string content = line.substr(pos + 1);
                    try
                    {
                        std::string decryptedContent = decrypt(content, config);
                        long long timestamp = std::stoll(timestampStr);
                        displayItems.push_back({timestamp, decryptedContent});
                    }
                    catch (...)
                    {
                        try
                        {
                            long long timestamp = std::stoll(timestampStr);
                            displayItems.push_back({timestamp, content});
                        }
                        catch (...)
                        {
                            long long timestamp = std::chrono::system_clock::now().time_since_epoch().count();
                            displayItems.push_back({timestamp, content});
                        }
                    }
                }
            }
            int numItems = displayItems.empty() ? 1 : displayItems.size();
            int preferredHeight = (numItems * LINE_HEIGHT) + 80;
            DialogDimensions dims = calculateDialogDimensions(windowWidth, windowHeight, windowWidth - 40, preferredHeight);
            int maxContentLength = calculateDialogContentLength(dims);
            for (auto& entry : displayItems)
            {
                if (static_cast<int>(entry.second.length()) > maxContentLength)
                {
                    entry.second = smartTrim(entry.second, maxContentLength);
                }
                for (char& c : entry.second)
                {
                    if (c == '\n' || c == '\r') c = ' ';
                }
            }

            drawPinnedDialog(display, window, gc, font, displayItems, dims,
                             selectedViewPinnedItem, viewPinnedScrollOffset, m_maxVisiblePinnedItems,
                             config.backgroundColor, config.textColor, config.selectionColor, config.borderColor, LINE_HEIGHT);
        }
        if (helpDialogVisible)
        {
            DialogDimensions dims = calculateDialogDimensions(windowWidth, windowHeight, 600, 500);

            drawHelpDialog(display, window, gc, dims,
                           helpFilterMode, helpFilterText, helpDialogScrollOffset,
                           config.backgroundColor, config.textColor, config.borderColor);
        }
        if (editDialogVisible)
        {
            DialogDimensions dims = calculateDialogDimensions(windowWidth, windowHeight, 600, 400);

            drawEditDialog(display, window, gc, font, dims,
                           editDialogInput, editDialogCursorLine, editDialogCursorPos,
                           editDialogScrollOffset,
                           config.backgroundColor, config.textColor, config.borderColor);
        }
    }
#endif
// End Linux UI Methods

// Wayland UI Methods
// End Wayland UI Methods


// Windows UI Methods
#ifdef _WIN32
    void ClipboardManager::drawConsole()
    {
        if (!visible) return;
        
        HDC hdc = GetDC(hwnd);
        if (!hdc) return;
        
        // Set up font and colors
        HFONT hFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                               DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                               CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Consolas");
        HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
        
        // Clear window with theme background
        RECT clientRect;
        GetClientRect(hwnd, &clientRect);
        
        HBRUSH hBgBrush = CreateSolidBrush(config.backgroundColor);
        FillRect(hdc, &clientRect, hBgBrush);
        DeleteObject(hBgBrush);
        
        SetTextColor(hdc, config.textColor);
        SetBkMode(hdc, TRANSPARENT);
        
        // Build console draw data
        ConsoleDrawData data;
        data.filterMode = filterMode;
        data.filterText = filterText;
        data.commandMode = commandMode;
        data.commandText = commandText;
        data.themeSelectMode = cmd_themeSelectMode;
        data.configSelectMode = cmd_configSelectMode;
        
        if (cmd_themeSelectMode)
        {
            data.themeItems = availableThemes;
            data.selectedTheme = selectedTheme;
            data.themeScrollOffset = themeSelectScrollOffset;
        }
        if (cmd_configSelectMode)
        {
            data.configItems = availableConfigs;
            data.selectedConfig = selectedConfig;
            data.configScrollOffset = configSelectScrollOffset;
        }
        
        data.startY = 20;
        data.windowWidth = windowWidth;
        data.windowHeight = windowHeight;
        data.lineHeight = LINE_HEIGHT;
        data.clipListWidth = clipListWidth;
        data.bgColor = config.backgroundColor;
        data.textColor = config.textColor;
        data.selColor = config.selectionColor;
        
        // Build clip display lines
        if (!cmd_themeSelectMode && !cmd_configSelectMode)
        {
            size_t displayCount = filterMode ? filteredItems.size() : items.size();
            data.totalClipCount = displayCount;
            data.selectedItem = selectedItem;
            data.clipScrollOffset = consoleScrollOffset;
            
            int availableHeight = windowHeight - data.startY - 10;
            const int SCROLL_INDICATOR_HEIGHT = 15;
            
            if (static_cast<int>(displayCount) > (availableHeight / LINE_HEIGHT))
            {
                availableHeight -= SCROLL_INDICATOR_HEIGHT;
            }
            
            int maxItems = availableHeight / LINE_HEIGHT;
            if (maxItems > 0) maxItems += 1;
            if (maxItems < 1) maxItems = 1;
            
            size_t endIdx = std::min(consoleScrollOffset + maxItems, displayCount);
            
            for (size_t i = consoleScrollOffset; i < endIdx; ++i)
            {
                size_t actualIndex = filterMode ? filteredItems[i] : i;
                const auto& item = items[actualIndex];
                
                std::string line;
                if (i == selectedItem)
                {
                    line = "> ";
                }
                else
                {
                    line = "  ";
                }
                
                if (config.verboseMode)
                {
                    auto time_t = std::chrono::system_clock::to_time_t(item.timestamp);
                    auto tm = *std::localtime(&time_t);
                    
                    std::ostringstream timeStream;
                    timeStream << std::put_time(&tm, "%H:%M:%S");
                    
                    size_t lineCount = 1;
                    for (char c : item.content)
                    {
                        if (c == '\n') lineCount++;
                    }
                    
                    line += timeStream.str() + " | " + std::to_string(lineCount) + " lines | ";
                    
                    std::string content = item.content;
                    int maxContentLength = calculateMaxContentLength(clipListWidth, true);
                    if (static_cast<int>(content.length()) > maxContentLength)
                    {
                        content = smartTrim(content, maxContentLength);
                    }
                    
                    for (char& c : content)
                    {
                        if (c == '\n' || c == '\r') c = ' ';
                    }
                    
                    line += content;
                }
                else
                {
                    size_t lineCount = 1;
                    for (char c : item.content)
                    {
                        if (c == '\n') lineCount++;
                    }
                    
                    std::string content = item.content;
                    int maxContentLength = calculateMaxContentLength(clipListWidth, false);
                    if (static_cast<int>(content.length()) > maxContentLength)
                    {
                        content = smartTrim(content, maxContentLength);
                    }
                    
                    for (char& c : content)
                    {
                        if (c == '\n' || c == '\r') c = ' ';
                    }
                    
                    line += content;
                    
                    if (lineCount > 1)
                    {
                        line += " (" + std::to_string(lineCount) + " lines)";
                    }
                }
                
                data.clipLines.push_back(line);
            }
        }
        
        ::drawConsole(hdc, data, WIN_SEL_RECT_HEIGHT, WIN_SEL_RECT_OFFSET_Y);
        
        // Draw dialogs if visible
        if (bookmarkDialogVisible)
        {
            DialogDimensions dims = calculateDialogDimensions(windowWidth, windowHeight, 400, 300);
            std::vector<std::string> filteredGroups;
            for (const auto& group : bookmarkGroups)
            {
                if (bookmarkDialogInput.empty() || group.find(bookmarkDialogInput) != std::string::npos)
                {
                    filteredGroups.push_back(group);
                }
            }

            drawBookmarkDialog(hdc, dims,
                             bookmarkDialogInput, filteredGroups,
                             selectedBookmarkGroup, bookmarkMgmtScrollOffset,
                             config.backgroundColor, config.textColor, config.selectionColor, config.borderColor,
                             WIN_SEL_RECT_HEIGHT, WIN_SEL_RECT_OFFSET_Y);
        }

        if (addToBookmarkDialogVisible)
        {
            DialogDimensions dims = calculateDialogDimensions(windowWidth, windowHeight, 400, 300);
            std::vector<std::string> displayedGroups;
            if (filterAddBookmarksMode)
            {
                std::string lowerFilterText = stringToLower(filterAddBookmarksText);
                for (const auto& group : bookmarkGroups)
                {
                    if (stringToLower(group).find(lowerFilterText) != std::string::npos)
                    {
                        displayedGroups.push_back(group);
                    }
                }
            }
            else
            {
                displayedGroups = bookmarkGroups;
            }
            if (selectedAddBookmarkGroup >= displayedGroups.size() && !displayedGroups.empty())
            {
                selectedAddBookmarkGroup = displayedGroups.size() - 1;
            }

            drawAddToBookmarkDialog(hdc, dims,
                                  displayedGroups,
                                  selectedAddBookmarkGroup, addBookmarkScrollOffset,
                                  filterAddBookmarksMode, filterAddBookmarksText,
                                  config.backgroundColor, config.textColor, config.selectionColor, config.borderColor,
                                  WIN_SEL_RECT_HEIGHT, WIN_SEL_RECT_OFFSET_Y);
        }

        if (viewBookmarksDialogVisible)
        {
            DialogDimensions dims = calculateDialogDimensions(windowWidth, windowHeight, 600, 500);
            std::string title;
            std::vector<std::string> items;
            size_t selItem = 0;
            size_t scrollOff = 0;
            bool filterActive = false;
            std::string filterTxt;
            int itemLH = LINE_HEIGHT;
            std::string emptyMsg;
            if (viewBookmarksShowingGroups)
            {
                title = "Select Bookmark Group";
                if (filterBookmarksMode)
                {
                    std::string lowerFilter = stringToLower(filterBookmarksText);
                    for (const auto& group : bookmarkGroups)
                    {
                        if (stringToLower(group).find(lowerFilter) != std::string::npos)
                        {
                            items.push_back(group);
                        }
                    }
                    filterActive = true;
                    filterTxt = filterBookmarksText;
                }
                else
                {
                    items = bookmarkGroups;
                }
                if (selectedViewBookmarkGroup >= items.size() && !items.empty())
                {
                    selectedViewBookmarkGroup = items.size() - 1;
                }
                selItem = selectedViewBookmarkGroup;
                scrollOff = viewBookmarksScrollOffset;
            }
            else
            {
                if (selectedViewBookmarkGroup < bookmarkGroups.size())
                {
                    title = "View Bookmarks: " + bookmarkGroups[selectedViewBookmarkGroup];
                }
                else
                {
                    title = "View Bookmarks";
                }
                if (filterBookmarkClipsMode)
                {
                    items = filteredBookmarkClips;
                    filterActive = true;
                    filterTxt = filterBookmarkClipsText;
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
                            while (std::getline(file, line))
                            {
                                size_t pos = line.find('|');
                                if (pos != std::string::npos && pos > 0)
                                {
                                    std::string content = line.substr(pos + 1);
                                    try
                                    {
                                        items.push_back(decrypt(content, config));
                                    }
                                    catch (...)
                                    {
                                        items.push_back(content);
                                    }
                                }
                            }
                            file.close();
                        }
                    }
                }
                int maxContentLength = calculateDialogContentLength(dims);
                for (auto& item : items)
                {
                    if (static_cast<int>(item.length()) > maxContentLength)
                    {
                        item = smartTrim(item, maxContentLength);
                    }
                    for (char& c : item)
                    {
                        if (c == '\n' || c == '\r') c = ' ';
                    }
                }
                if (selectedViewBookmarkItem >= items.size() && !items.empty())
                {
                    selectedViewBookmarkItem = items.size() - 1;
                }
                selItem = selectedViewBookmarkItem;
                scrollOff = viewBookmarksScrollOffset;
                emptyMsg = "No bookmarks in this group";
            }

            drawViewBookmarksDialog(hdc, dims,
                                  title, items, selItem, scrollOff,
                                  filterActive, filterTxt, itemLH, emptyMsg,
                                  config.backgroundColor, config.textColor, config.selectionColor, config.borderColor,
                                  WIN_SEL_RECT_HEIGHT, WIN_SEL_RECT_OFFSET_Y);
        }

        if (pinnedDialogVisible)
        {
            auto sortedItems = getSortedPinnedItems(config.pinnedFile);
            std::vector<std::pair<long long, std::string>> displayItems;
            for (const auto& line : sortedItems)
            {
                size_t pos = line.find('|');
                if (pos != std::string::npos && pos > 0)
                {
                    std::string timestampStr = line.substr(0, pos);
                    std::string content = line.substr(pos + 1);
                    try
                    {
                        std::string decryptedContent = decrypt(content, config);
                        long long timestamp = std::stoll(timestampStr);
                        displayItems.push_back({timestamp, decryptedContent});
                    }
                    catch (...)
                    {
                        try
                        {
                            long long timestamp = std::stoll(timestampStr);
                            displayItems.push_back({timestamp, content});
                        }
                        catch (...)
                        {
                            long long timestamp = std::chrono::system_clock::now().time_since_epoch().count();
                            displayItems.push_back({timestamp, content});
                        }
                    }
                }
            }
            int numItems = displayItems.empty() ? 1 : displayItems.size();
            int preferredHeight = (numItems * LINE_HEIGHT) + 80;
            DialogDimensions dims = calculateDialogDimensions(windowWidth, windowHeight, windowWidth - 40, preferredHeight);
            int maxContentLength = calculateDialogContentLength(dims);
            for (auto& entry : displayItems)
            {
                if (static_cast<int>(entry.second.length()) > maxContentLength)
                {
                    entry.second = smartTrim(entry.second, maxContentLength);
                }
                for (char& c : entry.second)
                {
                    if (c == '\n' || c == '\r') c = ' ';
                }
            }

            drawPinnedDialog(hdc, displayItems, dims,
                             selectedViewPinnedItem, viewPinnedScrollOffset, m_maxVisiblePinnedItems,
                             config.backgroundColor, config.textColor, config.selectionColor, config.borderColor,
                             LINE_HEIGHT, WIN_SEL_RECT_HEIGHT, WIN_SEL_RECT_OFFSET_Y);
        }

        if (helpDialogVisible)
        {
            DialogDimensions dims = calculateDialogDimensions(windowWidth, windowHeight, 600, 500);

            drawHelpDialog(hdc, dims,
                           helpFilterMode, helpFilterText, helpDialogScrollOffset,
                           config.backgroundColor, config.textColor, config.borderColor);
        }

        if (editDialogVisible)
        {
            DialogDimensions dims = calculateDialogDimensions(windowWidth, windowHeight, 600, 400);

            drawEditDialog(hdc, dims,
                           editDialogInput, editDialogCursorLine, editDialogCursorPos,
                           editDialogScrollOffset,
                           config.backgroundColor, config.textColor, config.borderColor);
        }

        // Cleanup
        SelectObject(hdc, hOldFont);
        DeleteObject(hFont);
        ReleaseDC(hwnd, hdc);
    }
#endif
// End Windows UI Methods


// MacOs UI Methods
#ifdef __APPLE__
#endif
// End MacOs UI Methods
