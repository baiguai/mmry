#include "main.h"

class ClipboardManager {
public:
    ~ClipboardManager() {
        std::cout << "ClipboardManager destructor called - cleaning up resources" << std::endl;
        stop();
    }

private:
    std::atomic<bool> hotkeyGrabbed{false};

public:



    //// Key Press /////////////////////////////////////////////////////////////
#ifdef __linux__
    void handleKeyPress(XEvent* event) {
        handleKeyPressCommon(event);
    }
#endif

    void handleKeyPressCommon(void* eventPtr) {
        // !@!
#ifdef __linux__
        XEvent* event = (XEvent*)eventPtr;
        KeySym keysym;
        char buffer[10];
        XKeyEvent* keyEvent = (XKeyEvent*)event;
        
        XLookupString(keyEvent, buffer, sizeof(buffer), &keysym, nullptr);
#endif
#ifdef _WIN32
        MSG* msg = (MSG*)eventPtr;
        std::cout << "Key received: " << (int)msg->wParam << std::endl;
        int virtualKey = (int)msg->wParam; 
        BYTE keyboardState[256] = {0};
        GetKeyboardState(keyboardState);
#endif




#ifdef __linux__
        if (keysym == XK_Q && (keyEvent->state & ShiftMask)) {
#endif
#ifdef _WIN32
        if (msg->wParam == 'Q' && (GetKeyState(VK_SHIFT) & 0x8000)) {
#endif
            // Shift+Q quits application even from dialog
            std::cout << "Quitting MMRY..." << std::endl;
            running = false; // Let main loop exit naturally to avoid deadlock
        }

#ifdef __linux__
        if (keysym == XK_Escape) {
#endif
#ifdef _WIN32
        if (msg->wParam == VK_ESCAPE) {
            // TODO: Implement [specific feature] for Windows
#endif
            if (pinnedDialogVisible) {
                pinnedDialogVisible = false;
                drawConsole();
            } else if (bookmarkDialogVisible) {
                // Escape hides dialog but not window
                bookmarkDialogVisible = false;
                drawConsole();
            } else if (addToBookmarkDialogVisible) {
                // Escape hides dialog but not window
                addToBookmarkDialogVisible = false;
                drawConsole();
            } else if (helpDialogVisible) {
                // Escape hides help dialog but not window
                helpDialogVisible = false;
                drawConsole();
            } else if (viewBookmarksDialogVisible) {
                // Escape hides view bookmarks dialog but not window
                viewBookmarksDialogVisible = false;
                drawConsole();
            } else if (filterMode) {
                // Escape exits filter mode but doesn't hide window
                filterMode = false;
                filterText = "";
                filteredItems.clear();
                selectedItem = 0;
                drawConsole();
            } else if (commandMode) {
                // Escape exits command mode but doesn't hide window
                commandMode = false;
                commandText = "";
                selectedItem = 0;
                drawConsole();
            } else if (cmd_themeSelectMode) {
                // Restore original theme and exit theme selection mode but doesn't hide window
                if (!originalTheme.empty()) {
                    switchTheme(originalTheme);
                }
                cmd_themeSelectMode = false;
                availableThemes.clear();
                originalTheme.clear();
                selectedItem = 0;
                drawConsole();
            } else {
                // Normal escape behavior - hide window
                hideWindow();
            }

            return;
        }

        if (helpDialogVisible) {
#ifdef __linux__
            if (keysym == XK_Escape || keysym == XK_question) {
#endif
#ifdef _WIN32
            if (msg->wParam == VK_ESCAPE || (msg->wParam == VK_OEM_2 && (GetKeyState(VK_SHIFT) & 0x8000))) {
#endif
                // Escape or '?' closes help dialog
                helpDialogVisible = false;
                drawConsole();
                return;
            }

#ifdef __linux__
            if (keysym == XK_j || keysym == XK_Down) {
#endif
#ifdef _WIN32
            if (msg->wParam == 'J' || msg->wParam == VK_DOWN) {
#endif
                // Scroll down
                updateHelpDialogScrollOffset(-1);
                drawConsole();
                return;
            }

#ifdef __linux__
            if (keysym == XK_k || keysym == XK_Up) {
#endif
#ifdef _WIN32
            if (msg->wParam == 'K' || msg->wParam == VK_UP) {
#endif
                // Scroll up
                if (helpDialogScrollOffset > 0) {
                    updateHelpDialogScrollOffset(1);
                    drawConsole();
                }
                return;
            }

#ifdef __linux__
            if (keysym == XK_g) {
#endif
#ifdef _WIN32
            if (msg->wParam == 'G') {
#endif
                // Scroll up
                helpDialogScrollOffset = 0;
                drawConsole();
                return;
            }

            return;
        }

        // ---------------------------------------------------------------------


        // Adding bookmark groups
        //
        if (bookmarkDialogVisible && !addToBookmarkDialogVisible) {
#ifdef __linux__
            if (keysym == XK_Return) {
#endif
#ifdef _WIN32
            if (msg->wParam == VK_RETURN) {
#endif
                // Enter creates/selects bookmark group using input text only
                if (!bookmarkDialogInput.empty()) {
                    // Check if this is a new group
                    bool groupExists = false;
                    for (const auto& group : bookmarkGroups) {
                        if (group == bookmarkDialogInput) {
                            groupExists = true;
                            break;
                        }
                    }
                    
                    if (!groupExists) {
                        // Create new group and add current clip
                        bookmarkGroups.push_back(bookmarkDialogInput);
                        saveBookmarkGroups();
                        
                        // Add current clip to bookmark
                        if (!items.empty() && selectedItem < getDisplayItemCount()) {
                            size_t actualIndex = getActualItemIndex(selectedItem);
                            addClipToBookmarkGroup(bookmarkDialogInput, items[actualIndex].content);
                            std::cout << "Added clip to bookmark group: " << bookmarkDialogInput << std::endl;
                        }
                    } else {
                        // Add current clip to existing group
                        if (!items.empty() && selectedItem < getDisplayItemCount()) {
                            size_t actualIndex = getActualItemIndex(selectedItem);
                            addClipToBookmarkGroup(bookmarkDialogInput, items[actualIndex].content);
                            std::cout << "Added clip to bookmark group: " << bookmarkDialogInput << std::endl;
                        }
                    }
                    
                    bookmarkDialogVisible = false;
                    drawConsole();
                }
                return;
            }

#ifdef __linux__
            if (keysym == XK_BackSpace) {
#endif
#ifdef _WIN32
            if (msg->wParam == VK_BACK) {
#endif
                // Backspace in input field
                if (!bookmarkDialogInput.empty()) {
                    bookmarkDialogInput.pop_back();
                    drawConsole();
                }
                return;
            }

            // Text input for bookmark dialog - exclude vim navigation keys
            char buffer[10];
#ifdef __linux__
            int count = XLookupString(keyEvent, buffer, sizeof(buffer), nullptr, nullptr);
#endif
#ifdef _WIN32
            int count = ToUnicode(virtualKey, msg->wParam, keyboardState, (LPWSTR)buffer, sizeof(buffer) / sizeof(buffer[0]), 0);
#endif
            if (count > 0) {
                bookmarkDialogInput += std::string(buffer, count);
                drawConsole();
            }

            return;
        }


        // Accessing bookmarked clips
        //
        if (viewBookmarksDialogVisible) {
            // View bookmarks dialog is visible - handle dialog-specific keys
#ifdef __linux__
            if (keysym == XK_Escape || keysym == XK_grave) {
#endif
#ifdef _WIN32
            if (msg->wParam == VK_ESCAPE || msg->wParam == VK_OEM_3 && (GetKeyState(VK_SHIFT) & 0x8000)) {
#endif
                // Escape or '`' closes view bookmarks dialog or goes back to groups
                if (!viewBookmarksShowingGroups) {
                    // If viewing clips, go back to groups
                    viewBookmarksShowingGroups = true;
                    selectedViewBookmarkItem = 0;
                    viewBookmarksScrollOffset = 0; // Reset scroll when going back
                } else {
                    // If viewing groups, close dialog
                    viewBookmarksDialogVisible = false;
                }
                drawConsole();

                return;
            }


            // Groups view
            //
            if (viewBookmarksShowingGroups) {
                // In group selection mode
#ifdef __linux__
                if (keysym == XK_j || keysym == XK_Down) {
#endif
#ifdef _WIN32
                if (msg->wParam == 'J' || msg->wParam == VK_DOWN) {
#endif
                    // Move down in groups
                    if (selectedViewBookmarkGroup < bookmarkGroups.size() - 1) {
                        selectedViewBookmarkGroup++;
                        updateScrollOffset();
                        drawConsole();
                    }
                    return;
                }

#ifdef __linux__
                if (keysym == XK_k || keysym == XK_Up) {
#endif
#ifdef _WIN32
                if (msg->wParam == 'K' || msg->wParam == VK_UP) {
#endif
                    // Move up in groups
                    if (selectedViewBookmarkGroup > 0) {
                        selectedViewBookmarkGroup--;
                        updateScrollOffset();
                        drawConsole();
                    }
                    return;
                }

#ifdef __linux__
                if (keysym == XK_g) {
#endif
#ifdef _WIN32
                if (msg->wParam == 'G') {
#endif
                    // Go to top
                    selectedViewBookmarkGroup = 0;
                    viewBookmarksScrollOffset = 0;
                    drawConsole();
                    return;
                }

#ifdef __linux__
                if (keysym == XK_G && (keyEvent->state & ShiftMask)) {
#endif
#ifdef _WIN32
                if (msg->wParam == 'G' && (GetKeyState(VK_SHIFT) & 0x8000)) {
#endif
                    // Go to bottom
                    selectedViewBookmarkGroup = bookmarkGroups.size() - 1;
                    updateScrollOffset();
                    drawConsole();
                    return;
                }

#ifdef __linux__
                if (keysym == XK_D && (keyEvent->state & ShiftMask)) {
#endif
#ifdef _WIN32
                if (msg->wParam == 'D' && (GetKeyState(VK_SHIFT) & 0x8000)) {
#endif
                    // Shift+D deletes selected group and its clips
                    if (selectedViewBookmarkGroup < bookmarkGroups.size()) {
                        std::string groupToDelete = bookmarkGroups[selectedViewBookmarkGroup];
                        
                        // Remove group from list
                        bookmarkGroups.erase(bookmarkGroups.begin() + selectedViewBookmarkGroup);
                        saveBookmarkGroups();
                        
                        // Delete bookmark file
                        std::string bookmarkFile = bookmarksDir + "/bookmarks_" + groupToDelete + ".txt";
                        unlink(bookmarkFile.c_str());
                        
                        std::cout << "Deleted bookmark group and all clips: " << groupToDelete << std::endl;
                        
                        // Adjust selection
                        if (selectedViewBookmarkGroup > 0 && selectedViewBookmarkGroup >= bookmarkGroups.size()) {
                            selectedViewBookmarkGroup = bookmarkGroups.size() - 1;
                        }
                        
                        // Close dialog if no groups left
                        if (bookmarkGroups.empty()) {
                            viewBookmarksDialogVisible = false;
                        }
                        
                        drawConsole();
                    }
                    return;
                }

#ifdef __linux__
                if (keysym == XK_Return) {
#endif
#ifdef _WIN32
                if (msg->wParam == VK_RETURN) {
#endif
                    // Select this group and show its clips
                    if (selectedViewBookmarkGroup < bookmarkGroups.size()) {
                        viewBookmarksShowingGroups = false;
                        selectedViewBookmarkItem = 0;
                        viewBookmarksScrollOffset = 0; // Reset scroll when switching modes
                        drawConsole();
                    }
                    return;
                }
            }

            // Clips are being shown
            //
            else {
#ifdef __linux__
                if (keysym == XK_j || keysym == XK_Down) {
#endif
#ifdef _WIN32
                if (msg->wParam == 'J' || msg->wParam == VK_DOWN) {
#endif
                    // Move down in bookmark items
                    selectedViewBookmarkItem++;
                    updateScrollOffset();
                    drawConsole();
                    return;
                }

#ifdef __linux__
                if (keysym == XK_k || keysym == XK_Up) {
#endif
#ifdef _WIN32
                if (msg->wParam == 'K' || msg->wParam == VK_UP) {
#endif
                    // Move up in bookmark items
                    if (selectedViewBookmarkItem > 0) {
                        selectedViewBookmarkItem--;
                        updateScrollOffset();
                        drawConsole();
                    }
                    return;
                }

#ifdef __linux__
                if (keysym == XK_g) {
#endif
#ifdef _WIN32
                if (msg->wParam == 'G') {
#endif
                    // Go to top
                    selectedViewBookmarkItem = 0;
                    viewBookmarksScrollOffset = 0;
                    drawConsole();
                    return;
                }

#ifdef __linux__
                if (keysym == XK_G && (keyEvent->state & ShiftMask)) {
#endif
#ifdef _WIN32
                if (msg->wParam == 'G' && (GetKeyState(VK_SHIFT) & 0x8000)) {
#endif
                    // Go to bottom
                    if (selectedViewBookmarkGroup < bookmarkGroups.size()) {
                        std::string selectedGroup = bookmarkGroups[selectedViewBookmarkGroup];
                        std::string bookmarkFile = bookmarksDir + "/bookmarks_" + selectedGroup + ".txt";
                        std::ifstream file(bookmarkFile);
                        
                        if (file.is_open()) {
                            std::string line;
                            size_t itemCount = 0;
                            while (std::getline(file, line)) {
                                size_t pos = line.find('|');
                                if (pos != std::string::npos && pos > 0) {
                                    itemCount++;
                                }
                            }
                            file.close();
                            
                            if (itemCount > 0) {
                                selectedViewBookmarkItem = itemCount - 1;
                                updateScrollOffset();
                                drawConsole();
                            }
                        }
                    }
                    return;
                }

#ifdef __linux__
                if (keysym == XK_D && (keyEvent->state & ShiftMask)) {
#endif
#ifdef _WIN32
                if (msg->wParam == 'D' && (GetKeyState(VK_SHIFT) & 0x8000)) {
#endif
                    // Shift+D deletes selected clip
                    if (selectedViewBookmarkGroup < bookmarkGroups.size()) {
                        std::string selectedGroup = bookmarkGroups[selectedViewBookmarkGroup];
                        std::string bookmarkFile = bookmarksDir + "/bookmarks_" + selectedGroup + ".txt";
                        std::ifstream file(bookmarkFile);
                        
                        if (file.is_open()) {
                            std::string line;
                            std::vector<std::string> lines;
                            
                            // Read all lines
                            while (std::getline(file, line)) {
                                lines.push_back(line);
                            }
                            file.close();
                            
                            // Remove the selected item if valid
                            if (selectedViewBookmarkItem < lines.size()) {
                                lines.erase(lines.begin() + selectedViewBookmarkItem);
                                
                                // Write back remaining lines
                                std::ofstream outFile(bookmarkFile);
                                if (outFile.is_open()) {
                                    for (const auto& line : lines) {
                                        outFile << line << "\n";
                                    }
                                    outFile.close();
                                    
                                    std::cout << "Deleted bookmark item from group: " << selectedGroup << std::endl;
                                    
                                    // Adjust selection
                                    if (selectedViewBookmarkItem > 0 && selectedViewBookmarkItem >= lines.size()) {
                                        selectedViewBookmarkItem = lines.size() - 1;
                                    }
                                    
                                    drawConsole();
                                }
                            }
                        }
                    }
                    return;
                }

#ifdef __linux__
                if (keysym == XK_Return) {
#endif
#ifdef _WIN32
                if (msg->wParam == VK_RETURN) {
#endif
                    // Copy selected bookmark item to clipboard
                    if (selectedViewBookmarkGroup < bookmarkGroups.size()) {
                        std::string selectedGroup = bookmarkGroups[selectedViewBookmarkGroup];
                        std::string bookmarkFile = bookmarksDir + "/bookmarks_" + selectedGroup + ".txt";
                        std::ifstream file(bookmarkFile);
                        
                        if (file.is_open()) {
                            std::string line;
                            std::vector<std::string> bookmarkItems;
                            
                            while (std::getline(file, line)) {
                                size_t pos = line.find('|');
                                if (pos != std::string::npos && pos > 0) {
                                    std::string content = line.substr(pos + 1);
                                    try {
                                        std::string decryptedContent = decrypt(content);
                                        bookmarkItems.push_back(decryptedContent);
                                    } catch (...) {
                                        bookmarkItems.push_back(content);
                                    }
                                }
                            }
                            file.close();
                            
                            if (selectedViewBookmarkItem < bookmarkItems.size()) {
                                copyToClipboard(bookmarkItems[selectedViewBookmarkItem]);
                                int lines = countLines(bookmarkItems[selectedViewBookmarkItem]);
                                if (lines > 1) {
                                    std::cout << "Copied " << lines << " lines from bookmark" << std::endl;
                                } else {
                                    std::cout << "Copied from bookmark: " << bookmarkItems[selectedViewBookmarkItem].substr(0, 50) << "..." << std::endl;
                                }
                                viewBookmarksDialogVisible = false;
                                hideWindow();
                            }
                        }
                    }
                    return;
                }

#ifdef __linux__
                if (keysym == XK_h) {
#endif
#ifdef _WIN32
                if (msg->wParam == 'H') {
#endif
                    // Go back to bookmark groups
                    viewBookmarksShowingGroups = true;
                    drawConsole();
                    return;
                }
            }
            return;
        }


        // Accessing pinned clips
        //
        if (pinnedDialogVisible) {
            // View pinned dialog is visible - handle dialog-specific keys
#ifdef __linux__
            if (keysym == XK_Escape) {
#endif
#ifdef _WIN32
            if (msg->wParam == VK_ESCAPE) {
#endif
                pinnedDialogVisible = false;
                drawConsole();
                return;
            }

#ifdef __linux__
            if (keysym == XK_j || keysym == XK_Down) {
#endif
#ifdef _WIN32
            if (msg->wParam == 'J' || msg->wParam == VK_DOWN) {
#endif
                // Move down in pinned items
                std::ifstream file(pinnedFile);
                if (file.is_open()) {
                    std::string line;
                    size_t itemCount = 0;
                    while (std::getline(file, line)) {
                        size_t pos = line.find('|');
                        if (pos != std::string::npos && pos > 0) {
                            itemCount++;
                        }
                    }
                    file.close();
                    
                    if (selectedViewPinnedItem < itemCount - 1) {
                        selectedViewPinnedItem++;
                        updatePinnedScrollOffset();
                        drawConsole();
                    }
                }
                return;
            }

#ifdef __linux__
            if (keysym == XK_k || keysym == XK_Up) {
#endif
#ifdef _WIN32
            if (msg->wParam == 'K' || msg->wParam == VK_UP) {
#endif
                // Move up in pinned items
                if (selectedViewPinnedItem > 0) {
                    selectedViewPinnedItem--;
                    updatePinnedScrollOffset();
                    drawConsole();
                }
                return;
            }

#ifdef __linux__
            if (keysym == XK_g) {
#endif
#ifdef _WIN32
            if (msg->wParam == 'G') {
#endif
                // Go to top
                selectedViewPinnedItem = 0;
                viewPinnedScrollOffset = 0;
                drawConsole();
                return;
            }

#ifdef __linux__
            if (keysym == XK_G && (keyEvent->state & ShiftMask)) {
#endif
#ifdef _WIN32
            if (msg->wParam == 'G' && (GetKeyState(VK_SHIFT) & 0x8000)) {
#endif
                // Go to bottom
                std::ifstream file(pinnedFile);
                if (file.is_open()) {
                    std::string line;
                    size_t itemCount = 0;
                    while (std::getline(file, line)) {
                        size_t pos = line.find('|');
                        if (pos != std::string::npos && pos > 0) {
                            itemCount++;
                        }
                    }
                    file.close();
                    
                    if (itemCount > 0) {
                        selectedViewPinnedItem = itemCount - 1;
                        updatePinnedScrollOffset();
                        drawConsole();
                    }
                }
                return;
            }

#ifdef __linux__
            if (keysym == XK_D && (keyEvent->state & ShiftMask)) {
#endif
#ifdef _WIN32
            if (msg->wParam == 'D' && (GetKeyState(VK_SHIFT) & 0x8000)) {
#endif
                // Shift+D deletes selected pinned clip
                std::ifstream inFile(pinnedFile);
                if (inFile.is_open()) {
                    std::vector<std::pair<long long, std::string>> pinnedItems;
                    std::string line;

                    // Read and parse lines
                    while (std::getline(inFile, line)) {
                        size_t pos = line.find('|');
                        if (pos != std::string::npos && pos > 0) {
                            try {
                                long long timestamp = std::stoll(line.substr(0, pos));
                                pinnedItems.push_back({timestamp, line});
                            } catch (...) {
                                // Fallback for parsing error: use current time, keep original line
                                long long timestamp = std::chrono::system_clock::now().time_since_epoch().count();
                                pinnedItems.push_back({timestamp, line});
                            }
                        }
                    }
                    inFile.close();

                    // Sort by timestamp to match display order
                    std::sort(pinnedItems.begin(), pinnedItems.end(),
                              [](const auto& a, const auto& b) { return a.first > b.first; });

                    // Remove the selected item if valid
                    if (selectedViewPinnedItem < pinnedItems.size()) {
                        pinnedItems.erase(pinnedItems.begin() + selectedViewPinnedItem);

                        // Write back remaining lines
                        std::ofstream outFile(pinnedFile);
                        if (outFile.is_open()) {
                            for (const auto& item : pinnedItems) {
                                outFile << item.second << "\n";
                            }
                            outFile.close();
                            
                            std::cout << "Deleted pinned clip" << std::endl;
                            
                            // Adjust selection
                            if (selectedViewPinnedItem > 0 && selectedViewPinnedItem >= pinnedItems.size()) {
                                selectedViewPinnedItem = pinnedItems.size() - 1;
                            }
                            
                            // Close dialog if no pinned clips left
                            if (pinnedItems.empty()) {
                                pinnedDialogVisible = false;
                            }
                            
                            drawConsole();
                        }
                    }
                }
                return;
            }

#ifdef __linux__
            if (keysym == XK_Return) {
#endif
#ifdef _WIN32
            if (msg->wParam == VK_RETURN) {
#endif
                // Copy selected pinned clip to clipboard and move to top
                std::vector<std::string> lines;
                std::string line;
                std::ifstream inFile(pinnedFile);
                while (std::getline(inFile, line)) {
                    lines.push_back(line);
                }
                inFile.close();

                if (selectedViewPinnedItem < lines.size()) {
                    std::string selectedLine = lines[selectedViewPinnedItem];
                    size_t pos = selectedLine.find('|');
                    if (pos != std::string::npos) {
                        std::string contentToCopy;
                        std::string contentToSave = selectedLine.substr(pos + 1);
                        try {
                            contentToCopy = decrypt(contentToSave);
                        } catch (...) {
                            contentToCopy = contentToSave;
                        }

                        copyToClipboard(contentToCopy);

                        int lineCount = countLines(contentToCopy);
                        if (lineCount > 1) {
                            std::cout << "Copied " << lineCount << " lines from pinned clips" << std::endl;
                        } else {
                            std::cout << "Copied from pinned clips: " << contentToCopy.substr(0, 50) << "..." << std::endl;
                        }

                        // Remove the old line
                        lines.erase(lines.begin() + selectedViewPinnedItem);

                        // Add the new line at the beginning
                        auto newTimestamp = std::chrono::system_clock::now().time_since_epoch().count();
                        std::string newLine = std::to_string(newTimestamp) + "|" + contentToSave;
                        lines.insert(lines.begin(), newLine);
                        
                        // Write the updated lines back to the file
                        std::ofstream outFile(pinnedFile);
                        for (const auto& l : lines) {
                            outFile << l << std::endl;
                        }
                        outFile.close();
                    }

                    pinnedDialogVisible = false;
                    hideWindow();
                }
                return;
            }

            return;
        }


        // Adding the current clip to a bookmark group
        //
        if (addToBookmarkDialogVisible) {
            // Add to bookmark dialog is visible - handle dialog-specific keys
#ifdef __linux__
            if (keysym == XK_Escape) {
#endif
#ifdef _WIN32
            if (msg->wParam == VK_ESCAPE) {
#endif
                // Escape closes dialog but not window
                addToBookmarkDialogVisible = false;
                drawConsole();
                return;
            }

#ifdef __linux__
            if (keysym == XK_Return) {
#endif
#ifdef _WIN32
            if (msg->wParam == VK_RETURN) {
#endif
                // std::cout << "TEST: hit return" << std::endl;
                // Enter adds clip to selected bookmark group
                if (!bookmarkGroups.empty() && selectedAddBookmarkGroup < bookmarkGroups.size()) {
                    std::string selectedGroup = bookmarkGroups[selectedAddBookmarkGroup];
                    
                    // Check if current clip is already in this group
                    if (!items.empty() && selectedItem < getDisplayItemCount()) {
                        size_t actualIndex = getActualItemIndex(selectedItem);
                        std::string clipContent = items[actualIndex].content;
                        
                        // Read existing bookmarks in this group
                        std::string bookmarkFile = bookmarksDir + "/bookmarks_" + selectedGroup + ".txt";
                        std::ifstream file(bookmarkFile);
                        std::string line;
                        bool alreadyExists = false;
                        
                        while (std::getline(file, line)) {
                            std::string decrypted = decrypt(line);
                            if (decrypted == clipContent) {
                                alreadyExists = true;
                                break;
                            }
                        }
                        file.close();
                        
                        if (!alreadyExists) {
                            addClipToBookmarkGroup(selectedGroup, clipContent);
                            std::cout << "Added clip to bookmark group: " << selectedGroup << std::endl;
                        } else {
                            std::cout << "Clip already exists in bookmark group: " << selectedGroup << std::endl;
                        }
                    }
                    
                    addToBookmarkDialogVisible = false;
                    drawConsole();
                }
                return;
            }

#ifdef __linux__
            if (keysym == XK_j || keysym == XK_Down) {
#endif
#ifdef _WIN32
            if (msg->wParam == 'J' || msg->wParam == VK_DOWN) {
#endif
                // Move down in bookmark groups
                if (!bookmarkGroups.empty() && selectedAddBookmarkGroup < bookmarkGroups.size() - 1) {
                    selectedAddBookmarkGroup++;
                    updateAddBookmarkScrollOffset();
                    drawConsole();
                }
                return;
            }

#ifdef __linux__
            if (keysym == XK_k || keysym == XK_Up) {
#endif
#ifdef _WIN32
            if (msg->wParam == 'K' || msg->wParam == VK_UP) {
#endif
                // Move up in bookmark groups
                if (selectedAddBookmarkGroup > 0) {
                    selectedAddBookmarkGroup--;
                    updateAddBookmarkScrollOffset();
                    drawConsole();
                }
                return;
            }

#ifdef __linux__
            if (keysym == XK_g) {
#endif
#ifdef _WIN32
            if (msg->wParam == 'G') {
#endif
                // Go to top
                selectedAddBookmarkGroup = 0;
                addBookmarkScrollOffset = 0;
                drawConsole();
                return;
            }

#ifdef __linux__
            if (keysym == XK_G && (keyEvent->state & ShiftMask)) {
#endif
#ifdef _WIN32
            if (msg->wParam == 'G' && (GetKeyState(VK_SHIFT) & 0x8000)) {
#endif
                // Go to bottom
                if (!bookmarkGroups.empty()) {
                    selectedAddBookmarkGroup = bookmarkGroups.size() - 1;
                    updateAddBookmarkScrollOffset();
                    drawConsole();
                }
                return;
            }

            return;
        }


        // Filter mode
        //
        if (filterMode) {
#ifdef __linux__
            if (keysym == XK_BackSpace) {
#endif
#ifdef _WIN32
            if (msg->wParam == VK_BACK) {
#endif
                // Remove last character from filter
                if (!filterText.empty()) {
                    filterText.pop_back();
                    updateFilteredItems();
                    selectedItem = 0;
                    drawConsole();
                }
                return;
            }

#ifdef __linux__
            if (keysym == XK_Delete) {
#endif
#ifdef _WIN32
            if (msg->wParam == VK_DELETE) {
#endif
                if (!items.empty() && selectedItem < getDisplayItemCount()) {
                    size_t actualIndex = getActualItemIndex(selectedItem);
                    items.erase(items.begin() + actualIndex);
                    
                    // Update filtered items after deletion
                    updateFilteredItems();
                    
                    // Adjust selection if needed
                    if (selectedItem >= getDisplayItemCount() && selectedItem > 0) {
                        selectedItem--;
                    }
                    
                    // Save changes and redraw
                    saveToFile();
                    drawConsole();
                }
                return;
            }

#ifdef __linux__
            if (keysym == XK_Return) {
#endif
#ifdef _WIN32
            if (msg->wParam == VK_RETURN) {
#endif
                // Copy selected item to clipboard and hide window
                if (!items.empty() && selectedItem < getDisplayItemCount()) {
                    size_t actualIndex = getActualItemIndex(selectedItem);
                    copyToClipboard(items[actualIndex].content);
                    int lines = countLines(items[actualIndex].content);
                    if (lines > 1) {
                        std::cout << "Copied " << lines << " lines to clipboard" << std::endl;
                    } else {
                        std::cout << "Copied to clipboard: " << items[actualIndex].content.substr(0, 50) << "..." << std::endl;
                    }
                    filterMode = false;
                    filterText = "";
                    filteredItems.clear();
                    hideWindow();
                }
                return;
            }

#ifdef __linux__
            if (keysym == XK_Down) {
#endif
#ifdef _WIN32
            if (msg->wParam == VK_DOWN) {
#endif
                // Move down
                size_t displayCount = getDisplayItemCount();
                if (selectedItem < displayCount - 1) {
                    selectedItem++;
                    updateConsoleScrollOffset();
                    drawConsole();
                }
                return;
            }

#ifdef __linux__
            if (keysym == XK_Up) {
#endif
#ifdef _WIN32
            if (msg->wParam == VK_UP) {
#endif
                // Move up
                if (selectedItem > 0) {
                    selectedItem--;
                    updateConsoleScrollOffset();
                    drawConsole();
                }
                return;
            }


            // Handle text input in filter mode - exclude vim navigation keys
            char buffer[10];
#ifdef __linux__
            int count = XLookupString(keyEvent, buffer, sizeof(buffer), nullptr, nullptr);
#endif
#ifdef _WIN32
            int count = ToUnicode(virtualKey, msg->wParam, keyboardState, (LPWSTR)buffer, sizeof(buffer) / sizeof(buffer[0]), 0);
#endif

            filterText += std::string(buffer, count);
            updateFilteredItems();
            selectedItem = 0;
            drawConsole();

            return;
        }

        // Command mode
        //
        if (commandMode) {
#ifdef __linux__
            if (keysym == XK_BackSpace) {
#endif
#ifdef _WIN32
            if (msg->wParam == VK_BACK) {
#endif
                // Remove last character from command
                if (!commandText.empty()) {
                    commandText.pop_back();
                    drawConsole();
                }
                return;
            }

#ifdef __linux__
            if (keysym == XK_Escape) {
#endif
#ifdef _WIN32
            if (msg->wParam == VK_ESCAPE) {
#endif
                // Exit command mode
                commandMode = false;
                commandText = "";
                drawConsole();
                return;
            }

#ifdef __linux__
            if (keysym == XK_Return) {
#endif
#ifdef _WIN32
            if (msg->wParam == VK_RETURN) {
#endif
                // Execute command and exit command mode
                if (!commandText.empty()) {
                    executeCommand(commandText);
                }
                commandMode = false;
                commandText = "";
                drawConsole();
                return;
            }

#ifdef __linux__
            if (keysym == XK_Down) {
#endif
#ifdef _WIN32
            if (msg->wParam == VK_DOWN) {
#endif
                // Move down
                size_t displayCount = getDisplayItemCount();
                if (selectedItem < displayCount - 1) {
                    selectedItem++;
                    updateConsoleScrollOffset();
                    drawConsole();
                }
                return;
            }

#ifdef __linux__
            if (keysym == XK_Up) {
#endif
#ifdef _WIN32
            if (msg->wParam == VK_UP) {
#endif
                // Move up
                if (selectedItem > 0) {
                    selectedItem--;
                    updateConsoleScrollOffset();
                    drawConsole();
                }
                return;
            }

#ifdef __linux__
            if (keysym == XK_space) {
#endif
#ifdef _WIN32
            if (msg->wParam == VK_SPACE) {
#endif
                // Handle space key - check if it's for theme command
                if (commandText == "theme") {
                    // Enter theme selection mode
                    commandMode = false;
                    cmd_themeSelectMode = true;
                    discoverThemes();
                    // Store original theme and apply first theme for preview
                    if (!availableThemes.empty()) {
                        originalTheme = theme;
                        selectedTheme = 0;
                        switchTheme(availableThemes[0]);
                    }
                    drawConsole();
                    return;
                }

                // Add space to command text for other commands
                commandText += " ";
                drawConsole();
                return;
            }

            // Handle text input in command mode - exclude vim navigation keys
            char buffer[10];
#ifdef __linux__
            int count = XLookupString(keyEvent, buffer, sizeof(buffer), nullptr, nullptr);
#endif
#ifdef _WIN32
            int count = ToUnicode(virtualKey, msg->wParam, keyboardState, (LPWSTR)buffer, sizeof(buffer) / sizeof(buffer[0]), 0);
#endif
            commandText += std::string(buffer, count);
            drawConsole();

            return;
        }

        // Theme selection mode
        //
        if (cmd_themeSelectMode) {
#ifdef __linux__
            if (keysym == XK_Escape) {
#endif
#ifdef _WIN32
            if (msg->wParam == VK_ESCAPE) {
#endif
                // Restore original theme and exit theme selection mode
                if (!originalTheme.empty()) {
                    switchTheme(originalTheme);
                }
                cmd_themeSelectMode = false;
                availableThemes.clear();
                originalTheme.clear();
                drawConsole();
                return;
            }

#ifdef __linux__
            if (keysym == XK_Return) {
#endif
#ifdef _WIN32
            if (msg->wParam == VK_RETURN) {
#endif
                // Save current theme to config and exit theme selection mode
                if (selectedTheme < availableThemes.size()) {
                    switchTheme(availableThemes[selectedTheme]);
                    // Save to config
                    saveConfig();
                }
                cmd_themeSelectMode = false;
                availableThemes.clear();
                originalTheme.clear();
                drawConsole();
                return;
            }

#ifdef __linux__
            if (keysym == XK_j || keysym == XK_Down) {
#endif
#ifdef _WIN32
            if (msg->wParam == 'J' || msg->wParam == VK_DOWN) {
#endif
                // Move down in theme list and apply live preview
                if (selectedTheme < availableThemes.size() - 1) {
                    selectedTheme++;
                    updateThemeSelectScrollOffset();
                    // Apply live preview
                    if (selectedTheme < availableThemes.size()) {
                        switchTheme(availableThemes[selectedTheme]);
                    }
                    drawConsole();
                }
                return;
            }

#ifdef __linux__
            if (keysym == XK_k || keysym == XK_Up) {
#endif
#ifdef _WIN32
            if (msg->wParam == 'K' || msg->wParam == VK_UP) {
#endif
                // Move up in theme list and apply live preview
                if (selectedTheme > 0) {
                    selectedTheme--;
                    updateThemeSelectScrollOffset();
                    // Apply live preview
                    if (selectedTheme < availableThemes.size()) {
                        switchTheme(availableThemes[selectedTheme]);
                    }
                    drawConsole();
                }
                return;
            }

#ifdef __linux__
            if (keysym == XK_g) {
#endif
#ifdef _WIN32
            if (msg->wParam == 'G') {
#endif
                // Go to top
                selectedTheme = 0;
                themeSelectScrollOffset = 0;
                drawConsole();
                return;
            }

#ifdef __linux__
            if (keysym == XK_G && (keyEvent->state & ShiftMask)) {
#endif
#ifdef _WIN32
            if (msg->wParam == 'G' && (GetKeyState(VK_SHIFT) & 0x8000)) {
#endif
                // Go to bottom
                if (!availableThemes.empty()) {
                    selectedTheme = availableThemes.size() - 1;
                    updateThemeSelectScrollOffset();
                    drawConsole();
                }
                return;
            }

            return;
        }


        // General keys - main clips list
        //
#ifdef __linux__
        if (keysym == XK_j || keysym == XK_Down) {
#endif
#ifdef _WIN32
        if (msg->wParam == 'J' || msg->wParam == VK_DOWN) {
#endif
            // Move down
            size_t displayCount = getDisplayItemCount();
            if (selectedItem < displayCount - 1) {
                selectedItem++;
                updateConsoleScrollOffset();
                drawConsole();
            }
            return;
        }

#ifdef __linux__
        if (keysym == XK_k || keysym == XK_Up) {
#endif
#ifdef _WIN32
        if (msg->wParam == 'K' || msg->wParam == VK_UP) {
#endif
            // Move up
            if (selectedItem > 0) {
                selectedItem--;
                updateConsoleScrollOffset();
                drawConsole();
            }
            return;
        }

#ifdef __linux__
        if (keysym == XK_g) {
#endif
#ifdef _WIN32
        if (msg->wParam == 'G') {
#endif
            // Go to top
            selectedItem = 0;
            updateConsoleScrollOffset();
            drawConsole();
            return;
        }

#ifdef __linux__
        if (keysym == XK_G && (keyEvent->state & ShiftMask)) {
#endif
#ifdef _WIN32
        if (msg->wParam == 'G' && (GetKeyState(VK_SHIFT) & 0x8000)) {
#endif
            // Go to bottom
            size_t displayCount = getDisplayItemCount();
            if (displayCount > 0) {
                selectedItem = displayCount - 1;
                updateConsoleScrollOffset();
                drawConsole();
            }
            return;
        }

#ifdef __linux__
        if (keysym == XK_D && (keyEvent->state & ShiftMask)) {
#endif
#ifdef _WIN32
        if (msg->wParam == 'D' && (GetKeyState(VK_SHIFT) & 0x8000)) {
#endif
            // Delete selected item (Shift+D)
            if (!items.empty() && selectedItem < getDisplayItemCount()) {
                size_t actualIndex = getActualItemIndex(selectedItem);
                items.erase(items.begin() + actualIndex);
                
                // Adjust selection
                size_t displayCount = getDisplayItemCount();
                if (selectedItem >= displayCount && selectedItem > 0) {
                    selectedItem--;
                }
                
                // Update filtered items if in filter mode
                if (filterMode) {
                    updateFilteredItems();
                }
                
                saveToFile();
                drawConsole();
            }
            return;
        }

#ifdef __linux__
        if (keysym == XK_slash) {
#endif
#ifdef _WIN32
        if (msg->wParam == VK_OEM_2) {
#endif
            // Enter filter mode
            filterMode = true;
            filterText = "";
            updateFilteredItems();
            selectedItem = 0;
            drawConsole();
            return;
        }

#ifdef __linux__
        if (keysym == XK_colon) {
#endif
#ifdef _WIN32
        if (msg->wParam == VK_OEM_1 && (GetKeyState(VK_SHIFT) & 0x8000)) {
#endif
            // Enter command mode
            commandMode = true;
            commandText = "";
            selectedItem = 0;
            drawConsole();
            return;
        }

#ifdef __linux__
        if (keysym == XK_Return) {
#endif
#ifdef _WIN32
        if (msg->wParam == VK_RETURN) {
#endif
            // Copy selected item to clipboard and hide window
            if (!items.empty() && selectedItem < getDisplayItemCount()) {
                size_t actualIndex = getActualItemIndex(selectedItem);
                std::string clipContent = items[actualIndex].content;

                copyToClipboard(clipContent);

                // Update timestamp and move to top if not already at top
                if (actualIndex != 0) {
                    // Remove from current position
                    items.erase(items.begin() + actualIndex);

                    // Create new item with current timestamp and insert at top
                    items.emplace(items.begin(), ClipboardItem(clipContent));

                    // Reset selection to top
                    selectedItem = 0;

                    // Update filtered items if in filter mode
                    if (filterMode) {
                        updateFilteredItems();
                    }

                    // Save to file with updated timestamp
                    saveToFile();

                    std::cout << "Clip moved to top after copying" << std::endl;
                }

                int lines = countLines(clipContent);

                if (lines > 1) {
                    std::cout << "Copied " << lines << " lines to clipboard" << std::endl;
                } else {
                    std::cout << "Copied to clipboard: " << clipContent.substr(0, 50) << "..." << std::endl;
                }
                hideWindow();
            }
            return;
        }

#ifdef __linux__
        if (keysym == XK_M && (keyEvent->state & ShiftMask)) {
#endif
#ifdef _WIN32
        if (msg->wParam == 'M' && (GetKeyState(VK_SHIFT) & 0x8000)) {
#endif
            // Shift+M to show bookmark dialog
            bookmarkDialogVisible = true;
            bookmarkDialogInput = "";
            selectedBookmarkGroup = 0;
            bookmarkMgmtScrollOffset = 0; // Reset scroll when opening
            drawConsole();
            return;
        }

#ifdef __linux__
        if (keysym == XK_m) {
#endif
#ifdef _WIN32
        if (msg->wParam == 'M') {
#endif
            // Lowercase m to show add-to-bookmark dialog
            if (!bookmarkGroups.empty()) {
                addToBookmarkDialogVisible = true;
                selectedAddBookmarkGroup = 0;
                addBookmarkScrollOffset = 0; // Reset scroll when opening
                drawConsole();
            }
            return;
        }

#ifdef __linux__
        if (keysym == XK_question) {
#endif
#ifdef _WIN32
        if (msg->wParam == VK_OEM_2 && (GetKeyState(VK_SHIFT) & 0x8000)) {
#endif
            // '?' to show help dialog
            helpDialogVisible = true;
            helpDialogScrollOffset = 0; // Reset scroll offset when opening help dialog
            drawConsole();
            return;
        }

#ifdef __linux__
        if (keysym == XK_grave) {
#endif
#ifdef _WIN32
        if (msg->wParam == VK_OEM_3) {
#endif
            // '`' to show view bookmarks dialog
            if (!bookmarkGroups.empty()) {
                viewBookmarksDialogVisible = true;
                viewBookmarksShowingGroups = true; // Start with group selection
                selectedViewBookmarkGroup = 0;
                selectedViewBookmarkItem = 0;
                viewBookmarksScrollOffset = 0; // Reset scroll when opening
                drawConsole();
            }
            return;
        }

        // Pin current clip
#ifdef __linux__
        if (keysym == XK_p) {
#endif
#ifdef _WIN32
        if (msg->wParam == 'P') {
#endif
            // Check if current clip is already pinned
            if (!items.empty() && selectedItem < getDisplayItemCount()) {
                size_t actualIndex = getActualItemIndex(selectedItem);
                std::string clipContent = items[actualIndex].content;
                
                // Read existing bookmarks in this group
                std::ifstream file(pinnedFile);
                std::string line;
                bool alreadyExists = false;
                
                while (std::getline(file, line)) {
                    std::string decrypted = decrypt(line);
                    if (decrypted == clipContent) {
                        alreadyExists = true;
                        break;
                    }
                }
                file.close();
                
                if (!alreadyExists) {
                    addClipToPinned(clipContent);
                    std::cout << "Added clip to pinned" << std::endl;
                } else {
                    std::cout << "Clip is already pinned" << std::endl;
                }
            }
            return;
        }

        // Pinned clips dialog
#ifdef __linux__
        if (keysym == XK_apostrophe) {
#endif
#ifdef _WIN32
        if (msg->wParam == VK_OEM_7) {
#endif
            pinnedDialogVisible = true;
            selectedViewPinnedItem = 0;
            viewPinnedScrollOffset = 0; // Reset scroll when opening
            drawConsole();
        }
    }
    //// End Key Press /////////////////////////////////////////////////////////





    void run() {
        running = true;
        visible = false;
        
        // Initialize X11
#ifdef __linux__
        display = XOpenDisplay(nullptr);
        if (!display) {
            std::cerr << "Cannot open display" << std::endl;
            return;
        }
        screen = DefaultScreen(display);
        root = RootWindow(display, screen);

        if (!XFixesQueryExtension(display, &xfixes_event_base, &xfixes_error_base)) {
            std::cerr << "XFixes extension not available" << std::endl;
            // Fallback to polling or exit? For now, we exit.
            XCloseDisplay(display);
            return;
        }

        clipboardAtom = XInternAtom(display, "CLIPBOARD", False);
        utf8Atom = XInternAtom(display, "UTF8_STRING", False);
        textAtom = XInternAtom(display, "TEXT", False);
        
        // Create window
        createWindow();
        
        // Setup hotkey
        setupHotkeys();
#endif
        
        // Initialize configuration and theme
        setupConfigDir();
        loadConfig();
        loadTheme();
        loadFromFile();
        loadBookmarkGroups();


        #ifdef _WIN32
            std::cout << "Running on Windows." << std::endl;
            // Add Windows-specific auto-start code here
        #elif __APPLE__
            std::cout << "Running on macOS." << std::endl;
            // Add macOS-specific auto-start code here
        #elif __linux__
            std::string dir = std::string(getenv("HOME")) + "/.config/autostart";
            std::string filePath = dir + "/mmry.desktop";

            if (autoStart) {
                std::string appName = "mmry";
                std::string appLabel = "Mmry";

                // ensure directory exists
                std::string mkdirCmd = "mkdir -p " + dir;
                int mkdirResult = system(mkdirCmd.c_str());
                (void)mkdirResult; // Suppress unused result warning


                char result[PATH_MAX];
                ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
                if (count != -1) {
                    result[count] = '\0'; // Null-terminate the string
                    std::cout << "Full path: " << result << std::endl;
                } else {
                    std::cerr << "Error getting path" << std::endl;
                }

                std::ofstream file(filePath);
                file <<
R"([Desktop Entry]
Type=Application
Exec=)" << result << R"(
Hidden=false
NoDisplay=false
X-GNOME-Autostart-enabled=true
Name=)" << appLabel << R"(
Comment=Autostart for )" << appLabel << R"(
)";
                file.close();
            } else {
                if (remove(filePath.c_str()) != 0) {
                    std::cout << "Autostart already disabled" << std::endl;
                } else {
                    std::cout << "Autostart disabled" << std::endl;
                }
            }
        #else
            std::cout << "Unknown operating system." << std::endl;
        #endif

        
        std::cout << "MMRY Clipboard Manager started" << std::endl;
        std::cout << "Config directory: " << configDir << std::endl;
        std::cout << "Press Ctrl+Alt+C to show window, Escape to hide" << std::endl;
        std::cout << "Press Shift+Q in window to quit application" << std::endl;
        std::cout << "Press Ctrl+C in terminal to exit" << std::endl;
        
#ifdef __linux__
        // --- Reliable X11 global hotkey setup for Ctrl+Alt+C ----------
        auto grab_global_hotkey = [&](Display* dpy, Window rootWin, KeySym keysym) {
            if (!dpy) {
                std::cout << "!dpy - returning" << std::endl;
                return;
            }
            // Ensure root receives KeyPress events
            XSelectInput(dpy, rootWin, KeyPressMask);
            XFlush(dpy);

            KeyCode kc = XKeysymToKeycode(dpy, keysym);

            const unsigned int baseMods[] = {
                ControlMask | Mod1Mask,                 // Ctrl + Alt
                ControlMask | Mod1Mask | Mod2Mask,     // + NumLock
                ControlMask | Mod1Mask | LockMask,     // + CapsLock
                ControlMask | Mod1Mask | Mod5Mask,     // + Mod5 (often ISO Level3)
                ControlMask | Mod1Mask | Mod2Mask | LockMask,
                ControlMask | Mod1Mask | Mod2Mask | Mod5Mask,
                ControlMask | Mod1Mask | LockMask | Mod5Mask,
                ControlMask | Mod1Mask | Mod2Mask | LockMask | Mod5Mask
            };

            for (unsigned int m : baseMods) {
                // Passive grabs on the root window
                XGrabKey(dpy, kc, m, rootWin, True, GrabModeAsync, GrabModeAsync);
                // Also grab with NumLock explicitly OR'd (sometimes necessary)
                XGrabKey(dpy, kc, m | Mod2Mask, rootWin, True, GrabModeAsync, GrabModeAsync);
            }
            XFlush(dpy);
        };

        auto ungrab_global_hotkey = [&](Display* dpy, Window rootWin, KeySym keysym) {
            if (!dpy) return;
            KeyCode kc = XKeysymToKeycode(dpy, keysym);
            const unsigned int baseMods[] = {
                ControlMask | Mod1Mask,
                ControlMask | Mod1Mask | Mod2Mask,
                ControlMask | Mod1Mask | LockMask,
                ControlMask | Mod1Mask | Mod5Mask,
                ControlMask | Mod1Mask | Mod2Mask | LockMask,
                ControlMask | Mod1Mask | Mod2Mask | Mod5Mask,
                ControlMask | Mod1Mask | LockMask | Mod5Mask,
                ControlMask | Mod1Mask | Mod2Mask | LockMask | Mod5Mask
            };
            for (unsigned int m : baseMods) {
                XUngrabKey(dpy, kc, m, rootWin);
                XUngrabKey(dpy, kc, m | Mod2Mask, rootWin);
            }
            XFlush(dpy);
        };

        // Must call this once after creating display & root
        grab_global_hotkey(display, root, XK_C);

        // Helpful: determine the grabbed keycode for runtime checks
        KeyCode grabbed_keycode = XKeysymToKeycode(display, XK_C);

        // Listen for clipboard changes
        XFixesSelectSelectionInput(display, root, clipboardAtom, XFixesSetSelectionOwnerNotifyMask);
        
        // --- Event loop: blocking, waits for next event -----------
        while (running) {
            XEvent event;
            XNextEvent(display, &event);

            // Handle clipboard change event
            if (event.type == xfixes_event_base + XFixesSelectionNotify) {
                XFixesSelectionNotifyEvent *selection_event = (XFixesSelectionNotifyEvent*)&event;
                if (selection_event->selection == clipboardAtom) {
                    requestClipboardContent();
                }
                continue;
            }

            // Handle clipboard content arrival
            if (event.type == SelectionNotify) {
                handleSelectionNotify(&event);
                continue;
            }
            
            // Handle global hotkey
            if (event.type == KeyPress && event.xkey.keycode == grabbed_keycode) {
                if ((event.xkey.state & ControlMask) && (event.xkey.state & Mod1Mask)) {
                    // Hotkey triggered
                    std::cout << "Hotkey triggered: Ctrl+Alt+C" << std::endl;
                    showWindow();
                    continue;
                }
            }

            // Events for the application window
            if (event.xany.window == window) {
                switch (event.type) {
                    case Expose:
                        drawConsole();
                        break;
                    case KeyPress:
                        handleKeyPress(&event);
                        break;
                    case ConfigureNotify:
                        // Window resize event
                        updateWindowDimensions(event.xconfigure.width, event.xconfigure.height);
                        drawConsole();
                        break;
                    default:
                        break;
                }
            }
        }

        // Unregister our grabs on exit
        ungrab_global_hotkey(display, root, XK_C);
#endif

#ifdef _WIN32
        std::cout << "Windows: registering global hotkey Ctrl+Alt+C..." << std::endl;

        // Register Ctrl+Alt+C (ID: 1)
        if (!RegisterHotKey(NULL, 1, MOD_CONTROL | MOD_ALT, 'C')) {
            std::cerr << "Failed to register global hotkey." << std::endl;
        }

        // --- Win32 window class (blank window for now) ---
        WNDCLASS wc = {0};
        wc.lpfnWndProc   = MMRYWndProc;
        wc.hInstance     = GetModuleHandle(NULL);
        wc.lpszClassName = "MMRY_Window_Class";

        RegisterClass(&wc);

        // 'hwnd' is now a class member, initialized to nullptr.
        // The check '!hwnd' on first hotkey press will create it.

        // --- Windows Message Loop ---
        MSG msg;
        while (running) {
            BOOL result = GetMessage(&msg, NULL, 0, 0);
            if (result <= 0) break;
            
            if (msg.message == WM_HOTKEY && msg.wParam == 1) {
                // Hotkey handling
                if (!hwnd) {
                    // Create window with proper styles for keyboard input
                    hwnd = CreateWindowEx(
                        WS_EX_CLIENTEDGE,
                        "MMRY_Window_Class",
                        "MMRY Clipboard Window",
                        WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT, CW_USEDEFAULT,
                        400, 300,
                        NULL, NULL, GetModuleHandle(NULL), 
                        this); // Pass 'this' as lpParam
                }
                visible = true;
                ShowWindow(hwnd, SW_SHOW);
                SetForegroundWindow(hwnd);
                SetFocus(hwnd); 
                drawConsole();
            }
            
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        UnregisterHotKey(NULL, 1);
#endif

#ifdef __APPLE__
        // macOS event loop
        // This would require NSApplication setup
#endif
    }
    
    void setRunning(bool state) {
        running = state;
    }
    
    void stop() {
        running = false;
        
        // Join threads to prevent memory leaks


        
#ifdef __linux__
        // Clean up X11 resources
        if (font) {
            XFreeFont(display, font);
            font = nullptr;
        }
        if (gc) {
            XFreeGC(display, gc);
            gc = nullptr;
        }
        if (display) {
            XCloseDisplay(display);
            display = nullptr;
        }
#endif
    }
    
private:
#ifdef __linux__
    Display* display;
    Window window;
    Window root;
    int screen;
    GC gc;
    XFontStruct* font;
        Atom clipboardAtom;
        Atom utf8Atom;
        Atom textAtom;
        int xfixes_event_base;
        int xfixes_error_base;
    #endif
    
    #ifdef _WIN32
        HWND hwnd = nullptr;
        HFONT font = nullptr;
    #endif
    
        std::atomic<bool> running;
        std::atomic<bool> visible;
        
        // Window properties
        int windowWidth = 800;
        int windowHeight = 600;
        const int WINDOW_X = 100;
        const int WINDOW_Y = 100;
        const int LINE_HEIGHT = 25;
        
        // Minimum window size constraints
        const int MIN_WINDOW_WIDTH = 425;
        const int MIN_WINDOW_HEIGHT = 525;
        
        // Dynamic width adjustment for clip list
        int clipListWidth = 780; // Default width (windowWidth - 20 for margins)
        
        // Clipboard data
        std::vector<ClipboardItem> items;
        std::string lastClipboardContent;
        bool verboseMode;
    std::string configDir;
    std::string bookmarksDir;
    std::string dataFile;
    std::string pinnedFile;
    size_t maxClips = 500;
    bool encrypted = false;
    std::string encryptionKey = "";
    std::string theme = "console";
    std::string originalTheme = ""; // Store original theme for preview mode
    bool autoStart = false;
    
    // Theme colors
    unsigned long backgroundColor = 0;
    unsigned long textColor = 0;
    unsigned long selectionColor = 0;
    unsigned long borderColor = 0;
    
    // Navigation
    size_t selectedItem = 0;
    size_t consoleScrollOffset = 0; // For scrolling main clips list
    bool filterMode = false;
    std::string filterText;
    std::vector<size_t> filteredItems;
    
    // Command mode
    bool commandMode = false;
    std::string commandText;
    
    // Theme selection mode
    bool cmd_themeSelectMode = false;
    std::vector<std::string> availableThemes;
    size_t selectedTheme = 0;
    size_t themeSelectScrollOffset = 0;
    
    // Bookmark dialog
    bool bookmarkDialogVisible = false;
    std::string bookmarkDialogInput;
    std::vector<std::string> bookmarkGroups;
    size_t selectedBookmarkGroup = 0;
    size_t bookmarkMgmtScrollOffset = 0; // For scrolling long lists

    // Pinned dialog
    bool pinnedDialogVisible = false;
    size_t selectedViewPinnedItem = 0;
    size_t viewPinnedScrollOffset = 0; // For scrolling long lists
    
    // Add to bookmark dialog state
    bool addToBookmarkDialogVisible = false;
    size_t selectedAddBookmarkGroup = 0;
    size_t addBookmarkScrollOffset = 0; // For scrolling long lists
    
    // Help dialog state
    bool helpDialogVisible = false;
    size_t helpDialogScrollOffset = 0;
    
    // View bookmarks dialog state
    bool viewBookmarksDialogVisible = false;
    bool viewBookmarksShowingGroups = true; // true = groups, false = clips
    size_t selectedViewBookmarkGroup = 0;
    size_t selectedViewBookmarkItem = 0;
    size_t viewBookmarksScrollOffset = 0; // For scrolling long lists
    
    // Helper methods

    // Custom Trimming
    ////////////////////////////////////////////////////////////////////////////
    std::string smartTrim(const std::string& text, size_t maxLength) {
        if (isPath(text)) {
            return trimMiddle(text, maxLength);
        }

        return text.substr(0, maxLength - 3) + "...";
    }


    bool isPath(const std::string& text) {
        if (isUrl(text) || isFilePath(text)) {
            return true;
        }
        return false;
    }

    bool isUrl(const std::string& text) {
        // Check for common URL patterns
        return (text.substr(0, 7) == "http://" || 
                text.substr(0, 8) == "https://" ||
                text.substr(0, 6) == "ftp://" ||
                text.substr(0, 7) == "sftp://" ||
                text.substr(0, 4) == "www.");
    }
    
    class PathDetector {
    public:
        static bool isFilePath(const std::string& text) {
            if (text.empty() || text.length() > 4096) return false;
            
            // Trim whitespace
            std::string trimmed = trim(text);
            if (trimmed.empty()) return false;
            
            // Quick checks for obvious paths
            if (isAbsolutePath(trimmed)) return true;
            
            // Check for path separators
            if (hasPathSeparators(trimmed)) return true;
            
            // Check for file extensions with proper context
            if (hasValidFileExtension(trimmed)) return true;
            
            // Use regex for more complex patterns
            if (matchesPathPattern(trimmed)) return true;
            
            return false;
        }

    private:
        static std::string trim(const std::string& str) {
            size_t start = str.find_first_not_of(" \t\n\r");
            if (start == std::string::npos) return "";
            size_t end = str.find_last_not_of(" \t\n\r");
            return str.substr(start, end - start + 1);
        }
        
        static bool isAbsolutePath(const std::string& text) {
            // Unix/Linux/macOS absolute path
            if (text[0] == '/') return true;
            
            // Windows absolute path (C:\, D:\, etc.)
            if (text.length() >= 3 && 
                std::isalpha(text[0]) && 
                text[1] == ':' && 
                (text[2] == '\\' || text[2] == '/')) {
                return true;
            }
            
            // Windows UNC path (\\server\share)
            if (text.length() >= 2 && text[0] == '\\' && text[1] == '\\') {
                return true;
            }
            
            return false;
        }
        
        static bool hasPathSeparators(const std::string& text) {
            // Count separators
            size_t slashCount = std::count(text.begin(), text.end(), '/');
            size_t backslashCount = std::count(text.begin(), text.end(), '\\');
            
            // If has backslashes, likely Windows path
            if (backslashCount > 0) return true;
            
            // Unix-style relative paths (./file or ../file or dir/file)
            if (slashCount > 0) {
                if (text.find("./") == 0 || text.find("../") == 0) return true;
                // Check if it looks like a path (has directory structure)
                if (slashCount >= 1 && text.find(' ') == std::string::npos) return true;
            }
            
            return false;
        }
        
        static bool hasValidFileExtension(const std::string& text) {
            size_t lastDot = text.find_last_of('.');
            size_t lastSlash = text.find_last_of("/\\");
            
            // Extension must be after last path separator
            if (lastDot == std::string::npos || 
                lastDot == 0 || 
                lastDot == text.length() - 1) {
                return false;
            }
            
            // Make sure dot is after the last separator (not in directory name)
            if (lastSlash != std::string::npos && lastDot < lastSlash) {
                return false;
            }
            
            std::string ext = text.substr(lastDot + 1);
            
            // Extension should be reasonable length and alphanumeric
            if (ext.length() < 1 || ext.length() > 10) return false;
            
            // Check if extension is alphanumeric
            bool validExt = std::all_of(ext.begin(), ext.end(), 
                [](char c) { return std::isalnum(c); });
            
            if (!validExt) return false;
            
            // Must have at least one character before the dot
            std::string basename = text.substr(0, lastDot);
            if (lastSlash != std::string::npos) {
                basename = basename.substr(lastSlash + 1);
            }
            
            // Avoid matching things like "version 2.0" or "item.no"
            if (basename.empty() || basename.find(' ') != std::string::npos) {
                return false;
            }
            
            // Check against common extensions (expanded list)
            std::string extLower = ext;
            std::transform(extLower.begin(), extLower.end(), extLower.begin(), ::tolower);
            
            static const std::vector<std::string> commonExts = {
                // Code
                "c", "cpp", "cc", "cxx", "h", "hpp", "hxx", "cs", "java", "py", "rb", "go",
                "js", "ts", "jsx", "tsx", "php", "swift", "kt", "rs", "scala", "r", "m", "mm",
                // Web
                "html", "htm", "css", "scss", "sass", "less", "xml", "json", "yaml", "yml",
                // Documents
                "txt", "md", "doc", "docx", "pdf", "rtf", "odt", "tex", "log",
                // Spreadsheets
                "xls", "xlsx", "csv", "ods",
                // Presentations
                "ppt", "pptx", "odp",
                // Images
                "jpg", "jpeg", "png", "gif", "bmp", "svg", "ico", "webp", "tiff", "tif",
                // Audio
                "mp3", "wav", "ogg", "flac", "aac", "m4a", "wma",
                // Video
                "mp4", "avi", "mkv", "mov", "wmv", "flv", "webm",
                // Archives
                "zip", "rar", "7z", "tar", "gz", "bz2", "xz", "tgz",
                // Executables
                "exe", "dll", "so", "dylib", "app", "dmg", "deb", "rpm", "msi",
                // Config
                "ini", "cfg", "conf", "config", "properties", "toml",
                // Scripts
                "sh", "bash", "bat", "cmd", "ps1", "vbs",
                // Data
                "db", "sqlite", "sql", "dat", "bin"
            };
            
            return std::find(commonExts.begin(), commonExts.end(), extLower) != commonExts.end();
        }
        
        static bool matchesPathPattern(const std::string& text) {
            // Regex patterns for different path types
            static const std::regex patterns[] = {
                // Unix absolute: /path/to/file or /path/to/dir/
                std::regex(R"(^/([a-zA-Z0-9_\-\.]+/?)+$)"),
                
                // Unix relative: ./file, ../file, dir/file
                std::regex(R"(^\.{1,2}/([a-zA-Z0-9_\-\.]+/?)+$)"),
                
                // Windows absolute: C:\path\to\file
                std::regex(R"(^[a-zA-Z]:[/\\]([a-zA-Z0-9_\-\. ]+[/\\]?)+$)"),
                
                // Windows UNC: \\server\share\path
                std::regex(R"(^\\\\[a-zA-Z0-9_\-\.]+\\([a-zA-Z0-9_\-\. ]+\\?)+$)"),
                
                // Relative with subdirs: folder/subfolder/file.ext
                std::regex(R"(^[a-zA-Z0-9_\-\.]+(/[a-zA-Z0-9_\-\.]+)+(\.[a-zA-Z0-9]{1,10})?$)"),
                
                // Home directory: ~/path/to/file
                std::regex(R"(^~(/[a-zA-Z0-9_\-\.]+)+$)")
            };
            
            for (const auto& pattern : patterns) {
                if (std::regex_match(text, pattern)) {
                    return true;
                }
            }
            
            return false;
        }
    };

    // Convenience function
    bool isFilePath(const std::string& text) {
        return PathDetector::isFilePath(text);
    }
    
    class StringTrimmer {
    public:
        static std::string trimMiddle(const std::string& text, size_t maxLength) {
            if (text.length() <= maxLength) {
                return text;
            }
            
            // Minimum sensible length for ellipsis trimming
            if (maxLength < 4) {
                return text.substr(0, maxLength);
            }
            
            // For very short lengths, just use end trimming
            if (maxLength <= 10) {
                return text.substr(0, maxLength - 3) + "...";
            }
            
            const std::string ellipsis = "...";
            const size_t ellipsisLen = ellipsis.length();
            
            // Try smart trimming for URLs and file paths
            if (isUrl(text)) {
                return trimUrlMiddle(text, maxLength, ellipsisLen);
            } else if (isFilePath(text)) {
                return trimPathMiddle(text, maxLength, ellipsisLen);
            }
            
            // Default: balanced trim
            return trimBalanced(text, maxLength, ellipsisLen);
        }

    private:
        static std::string trimUrlMiddle(const std::string& url, size_t maxLength, size_t ellipsisLen) {
            // Try to preserve: protocol + domain + filename/endpoint
            size_t protocolEnd = url.find("://");
            
            if (protocolEnd == std::string::npos) {
                return trimBalanced(url, maxLength, ellipsisLen);
            }
            
            protocolEnd += 3; // Include "://"
            
            // Safety check
            if (protocolEnd >= url.length()) {
                return trimBalanced(url, maxLength, ellipsisLen);
            }
            
            // Find domain end (first slash after protocol)
            size_t domainEnd = url.find('/', protocolEnd);
            size_t queryStart = url.find('?');
            size_t fragmentStart = url.find('#');
            
            // Find the last meaningful part (filename or endpoint)
            size_t lastSlash = url.find_last_of('/');
            
            // Calculate important sections
            std::string protocol = url.substr(0, protocolEnd);
            std::string domain = (domainEnd != std::string::npos) 
                ? url.substr(protocolEnd, domainEnd - protocolEnd)
                : url.substr(protocolEnd);
            
            // Get last path segment (filename/endpoint)
            std::string lastSegment;
            if (lastSlash != std::string::npos && lastSlash + 1 < url.length()) {
                size_t segmentEnd = std::min({
                    queryStart != std::string::npos ? queryStart : url.length(),
                    fragmentStart != std::string::npos ? fragmentStart : url.length(),
                    url.length()
                });
                
                if (segmentEnd > lastSlash) {
                    lastSegment = url.substr(lastSlash, segmentEnd - lastSlash);
                }
            }
            
            // Strategy: protocol + domain + ... + lastSegment
            size_t prefixLen = protocol.length() + domain.length();
            size_t suffixLen = lastSegment.length();
            
            // Check if we can fit protocol + domain + ... + last segment
            if (prefixLen + ellipsisLen + suffixLen <= maxLength && 
                !lastSegment.empty() &&
                domainEnd != std::string::npos && 
                lastSlash != std::string::npos &&
                lastSlash > domainEnd) {
                return protocol + domain + "..." + lastSegment;
            }
            
            // Check if we can fit protocol + domain + ...
            if (prefixLen + ellipsisLen <= maxLength && domainEnd != std::string::npos) {
                size_t remainingSpace = maxLength - prefixLen - ellipsisLen;
                if (remainingSpace > 0 && remainingSpace <= url.length()) {
                    std::string suffix = url.substr(url.length() - remainingSpace);
                    return protocol + domain + "..." + suffix;
                }
                return protocol + domain + "...";
            }
            
            // If domain itself is too long, trim it
            if (protocol.length() + ellipsisLen < maxLength) {
                size_t domainSpace = maxLength - protocol.length() - ellipsisLen;
                
                if (domainSpace < domain.length() && domainSpace > 0) {
                    return protocol + domain.substr(0, domainSpace) + "...";
                }
            }
            
            // Fallback to balanced trim
            return trimBalanced(url, maxLength, ellipsisLen);
        }
        
        static std::string trimPathMiddle(const std::string& path, size_t maxLength, size_t ellipsisLen) {
            // Strategy: preserve root + first dir(s) + ... + last dir(s)
            
            // Get root/prefix
            std::string prefix;
            size_t prefixEnd = 0;
            
            if (path[0] == '/') {
                // Unix absolute path - keep "/"
                prefix = "/";
                prefixEnd = 1;
            } else if (path.length() >= 2 && path[1] == ':') {
                // Windows drive letter - keep "C:\" or "C:/"
                prefix = path.substr(0, std::min(size_t(3), path.length()));
                prefixEnd = prefix.length();
            } else if (path.length() >= 2 && path[0] == '\\' && path[1] == '\\') {
                // UNC path - keep "\\server"
                size_t nextSep = path.find_first_of("\\/", 2);
                if (nextSep != std::string::npos) {
                    prefix = path.substr(0, nextSep);
                    prefixEnd = nextSep;
                } else {
                    prefix = path.substr(0, std::min(size_t(20), path.length()));
                    prefixEnd = prefix.length();
                }
            } else if (path.find("./") == 0 || path.find("../") == 0) {
                // Relative path - keep "./" or "../"
                size_t firstSep = path.find_first_of("/\\");
                if (firstSep != std::string::npos) {
                    prefix = path.substr(0, firstSep + 1);
                    prefixEnd = firstSep + 1;
                } else {
                    prefix = path;
                    prefixEnd = path.length();
                }
            } else if (path.find("~/") == 0) {
                // Home directory
                prefix = "~/";
                prefixEnd = 2;
            }
            
            // Find all separators after prefix
            std::vector<size_t> separators;
            for (size_t i = prefixEnd; i < path.length(); ++i) {
                if (path[i] == '/' || path[i] == '\\') {
                    separators.push_back(i);
                }
            }
            
            if (separators.empty()) {
                // No separators after prefix, return as-is or trim balanced
                if (path.length() <= maxLength) {
                    return path;
                }
                return trimBalanced(path, maxLength, ellipsisLen);
            }
            
            // Try to preserve: prefix + first_dirs + ... + last_dirs
            // Example: /home/baiguai/...images/raw
            
            // Get first 1-2 components after prefix
            size_t firstComponentsEnd = separators[0]; // At least first component
            if (separators.size() > 1) {
                firstComponentsEnd = separators[1]; // Try to get 2 components
            }
            std::string firstPart = path.substr(0, firstComponentsEnd);
            
            // Get last 2-3 components
            size_t lastComponentsStart = separators[separators.size() - 1];
            if (separators.size() >= 2) {
                lastComponentsStart = separators[separators.size() - 2];
            }
            if (separators.size() >= 3) {
                lastComponentsStart = separators[separators.size() - 3];
            }
            std::string lastPart = path.substr(lastComponentsStart);
            
            // Try: firstPart + ... + lastPart
            if (firstPart.length() + ellipsisLen + lastPart.length() <= maxLength) {
                return firstPart + "..." + lastPart;
            }
            
            // If that's too long, try just first component + ... + last 2 components
            if (separators.size() >= 2) {
                firstComponentsEnd = separators[0];
                firstPart = path.substr(0, firstComponentsEnd);
                
                lastComponentsStart = separators[separators.size() - 2];
                lastPart = path.substr(lastComponentsStart);
                
                if (firstPart.length() + ellipsisLen + lastPart.length() <= maxLength) {
                    return firstPart + "..." + lastPart;
                }
            }
            
            // Fall back to: prefix + ... + last components
            size_t suffixStart = separators[separators.size() - 1];
            if (separators.size() >= 2) {
                suffixStart = separators[separators.size() - 2];
            }
            if (separators.size() >= 3) {
                suffixStart = separators[separators.size() - 3];
            }
            std::string suffix = path.substr(suffixStart);
            
            if (prefix.length() + ellipsisLen + suffix.length() <= maxLength) {
                return prefix + "..." + suffix;
            }
            
            // If still too long, just keep prefix + ... + last component
            suffix = path.substr(separators[separators.size() - 1]);
            if (prefix.length() + ellipsisLen + suffix.length() <= maxLength) {
                return prefix + "..." + suffix;
            }
            
            // Last resort: trim suffix to fit
            size_t remainingSpace = maxLength - prefix.length() - ellipsisLen;
            if (remainingSpace > 0 && remainingSpace <= suffix.length()) {
                return prefix + "..." + suffix.substr(suffix.length() - remainingSpace);
            }
            
            // Fallback to balanced trim
            return trimBalanced(path, maxLength, ellipsisLen);
        }
        
        static std::string trimBalanced(const std::string& text, size_t maxLength, size_t ellipsisLen) {
            size_t availableSpace = maxLength - ellipsisLen;
            size_t keepStart = availableSpace / 2;
            size_t keepEnd = availableSpace - keepStart;
            
            return text.substr(0, keepStart) + "..." + text.substr(text.length() - keepEnd);
        }
        
        // Placeholder functions - replace with your actual implementations
        static bool isUrl(const std::string& text) {
            return text.find("://") != std::string::npos;
        }
        
        static bool isFilePath(const std::string& text) {
            // Use your actual isFilePath implementation
            return text.find('/') != std::string::npos || 
                   text.find('\\') != std::string::npos ||
                   (text.length() >= 3 && text[1] == ':');
        }
    };

    // Convenience function
    std::string trimMiddle(const std::string& text, size_t maxLength) {
        return StringTrimmer::trimMiddle(text, maxLength);
    }
    ////////////////////////////////////////////////////////////////////////////


    size_t getDisplayItemCount() {
        if (filterMode) {
            return filteredItems.size();
        }
        return items.size();
    }
    
    size_t getActualItemIndex(size_t displayIndex) {
        if (filterMode && displayIndex < filteredItems.size()) {
            return filteredItems[displayIndex];
        }
        return displayIndex;
    }
    
    void updateFilteredItems() {
        filteredItems.clear();
        
        if (filterText.empty()) {
            for (size_t i = 0; i < items.size(); ++i) {
                filteredItems.push_back(i);
            }
        } else {
            std::string lowerFilter = filterText;
            for (char& c : lowerFilter) {
                c = tolower(c);
            }
            
            for (size_t i = 0; i < items.size(); ++i) {
                std::string lowerContent = items[i].content;
                for (char& c : lowerContent) {
                    c = tolower(c);
                }
                
                if (lowerContent.find(lowerFilter) != std::string::npos) {
                    filteredItems.push_back(i);
                }
            }
        }
        
        // Reset selection if no items match
        if (filteredItems.empty()) {
            selectedItem = 0;
        } else if (selectedItem >= filteredItems.size()) {
            selectedItem = filteredItems.size() - 1;
        }
    }
    
    void updateScrollOffset() {
        const int VISIBLE_ITEMS = 20; // Number of items visible in dialog
        
        if (viewBookmarksShowingGroups) {
            // Scrolling for groups
            if (selectedViewBookmarkGroup < viewBookmarksScrollOffset) {
                viewBookmarksScrollOffset = selectedViewBookmarkGroup;
            } else if (selectedViewBookmarkGroup >= viewBookmarksScrollOffset + VISIBLE_ITEMS) {
                viewBookmarksScrollOffset = selectedViewBookmarkGroup - VISIBLE_ITEMS + 1;
            }
        } else {
            // Scrolling for clips
            if (selectedViewBookmarkItem < viewBookmarksScrollOffset) {
                viewBookmarksScrollOffset = selectedViewBookmarkItem;
            } else if (selectedViewBookmarkItem >= viewBookmarksScrollOffset + VISIBLE_ITEMS) {
                viewBookmarksScrollOffset = selectedViewBookmarkItem - VISIBLE_ITEMS + 1;
            }
        }
    }
    
    void updateConsoleScrollOffset() {
        const int SCROLL_INDICATOR_HEIGHT = 15; // Height reserved for scroll indicator
        
        // Calculate starting Y position (accounting for filter, command, or theme selection mode)
        int startY = (filterMode || commandMode || cmd_themeSelectMode) ? 45 : 20;
        
        // Calculate available height for items
        int availableHeight = windowHeight - startY - 10; // 10px bottom margin
        
        size_t displayCount = getDisplayItemCount();
        
        // Calculate how many items can fit
        int maxVisibleItems = availableHeight / LINE_HEIGHT;
        
        // If we have more items than fit, reserve space for scroll indicator
        if (displayCount > maxVisibleItems) {
            availableHeight -= SCROLL_INDICATOR_HEIGHT;
            maxVisibleItems = availableHeight / LINE_HEIGHT;
        }

        if (maxVisibleItems > 0) maxVisibleItems += 1;
        
        // Ensure we show at least 1 item
        if (maxVisibleItems < 1) {
            maxVisibleItems = 1;
        }
        
        // Update scroll offset to keep selected item visible
        if (selectedItem < consoleScrollOffset) {
            consoleScrollOffset = selectedItem;
        } else if (selectedItem >= consoleScrollOffset + maxVisibleItems) {
            consoleScrollOffset = selectedItem - maxVisibleItems + 1;
        }
    }
    
    void updateBookmarkMgmtScrollOffset() {
        const int VISIBLE_ITEMS = 10; // Number of groups visible in bookmark management dialog
        
        // Filter groups for scroll calculation
        std::vector<std::string> filteredGroups;
        for (const auto& group : bookmarkGroups) {
            if (bookmarkDialogInput.empty() || group.find(bookmarkDialogInput) != std::string::npos) {
                filteredGroups.push_back(group);
            }
        }
        
        if (selectedBookmarkGroup < bookmarkMgmtScrollOffset) {
            bookmarkMgmtScrollOffset = selectedBookmarkGroup;
        } else if (selectedBookmarkGroup >= bookmarkMgmtScrollOffset + VISIBLE_ITEMS) {
            bookmarkMgmtScrollOffset = selectedBookmarkGroup - VISIBLE_ITEMS + 1;
        }
    }

    void updatePinnedScrollOffset() {
        const int VISIBLE_ITEMS = 20;
        
        if (selectedViewPinnedItem < viewPinnedScrollOffset) {
            viewPinnedScrollOffset = selectedViewPinnedItem;
        } else if (selectedViewPinnedItem >= viewPinnedScrollOffset + VISIBLE_ITEMS) {
            viewPinnedScrollOffset = selectedViewPinnedItem - VISIBLE_ITEMS + 1;
        }
    }
    
    void updateAddBookmarkScrollOffset() {
        const int VISIBLE_ITEMS = 10; // Number of groups visible in add bookmark dialog
        
        if (selectedAddBookmarkGroup < addBookmarkScrollOffset) {
            addBookmarkScrollOffset = selectedAddBookmarkGroup;
        } else if (selectedAddBookmarkGroup >= addBookmarkScrollOffset + VISIBLE_ITEMS) {
            addBookmarkScrollOffset = selectedAddBookmarkGroup - VISIBLE_ITEMS + 1;
        }
}
    
    void updateThemeSelectScrollOffset() {
        const int VISIBLE_ITEMS = 10; // Number of themes visible in theme selection
        
        if (selectedTheme < themeSelectScrollOffset) {
            themeSelectScrollOffset = selectedTheme;
        } else if (selectedTheme >= themeSelectScrollOffset + VISIBLE_ITEMS) {
            themeSelectScrollOffset = selectedTheme - VISIBLE_ITEMS + 1;
        }
    }
    
    void updateHelpDialogScrollOffset(int adjustment) {
        const int VISIBLE_ITEMS = 50; // Number of items visible in help dialog
        const int STEP = 10;
       
        helpDialogScrollOffset = helpDialogScrollOffset + (adjustment * STEP);

        if (helpDialogScrollOffset > -1)
        {
            helpDialogScrollOffset = 0;
        }
    }
    
    // Dynamic window and layout management functions
    void updateWindowDimensions(int newWidth, int newHeight) {
        // Enforce minimum window size constraints
        if (newWidth < MIN_WINDOW_WIDTH) {
            newWidth = MIN_WINDOW_WIDTH;
        }
        if (newHeight < MIN_WINDOW_HEIGHT) {
            newHeight = MIN_WINDOW_HEIGHT;
        }
        
        windowWidth = newWidth;
        windowHeight = newHeight;
        updateClipListWidth();
        updateConsoleScrollOffset();
    }
    
    void updateClipListWidth() {
        // Calculate clip list width with margins (10px on each side)
        clipListWidth = windowWidth - 20;
        
        // Ensure minimum width for usability
        if (clipListWidth < 200) {
            clipListWidth = 200;
        }
    }
    
    int getClipListWidth() const {
        return clipListWidth;
    }
    
    int getWindowWidth() const {
        return windowWidth;
    }
    
    int getWindowHeight() const {
        return windowHeight;
    }
    
    int calculateMaxContentLength(bool verboseMode) const {
        // Calculate available width for content (excluding selection indicator and margins)
        int availableWidth = getClipListWidth() - 30; // 10px left margin + 20px for selection indicator
        
        if (verboseMode) {
            // Verbose mode: "HH:MM:SS | X lines | " prefix takes about 20 chars
            availableWidth -= 20;
        }
        
        // Estimate character width (assuming monospace font, average width ~8 pixels)
        int maxChars = availableWidth / 8;
        
        // Ensure reasonable minimum and maximum
        if (maxChars < 20) maxChars = 20;
        if (maxChars > 200) maxChars = 200;
        
        return maxChars;
    }
    

    
    // Dialog positioning and sizing structure
    struct DialogDimensions {
        int width;
        int height;
        int x;
        int y;
        int contentWidth;  // Width available for content (excluding margins)
        int contentHeight; // Height available for content (excluding margins)
    };
    
    // Modular dialog positioning functions
    DialogDimensions calculateDialogDimensions(int preferredWidth, int preferredHeight) const {
        DialogDimensions dims;
        
        // Calculate maximum size that fits in window with margins
        int maxWidth = windowWidth - 40;  // 20px margin on each side
        int maxHeight = windowHeight - 40; // 20px margin on each side
        
        // Ensure minimum usable size
        int minDialogWidth = 300;
        int minDialogHeight = 200;
        
        // Use preferred size if it fits, otherwise scale down
        dims.width = std::min(preferredWidth, std::max(minDialogWidth, maxWidth));
        dims.height = std::min(preferredHeight, std::max(minDialogHeight, maxHeight));
        
        // Center dialog in window
        dims.x = (windowWidth - dims.width) / 2;
        dims.y = (windowHeight - dims.height) / 2;
        
        // Calculate content area (excluding borders and margins)
        dims.contentWidth = dims.width - 40;  // 20px margin on each side
        dims.contentHeight = dims.height - 60; // 30px margin top/bottom for title and padding
        
        // Ensure minimum content area
        if (dims.contentWidth < 200) dims.contentWidth = 200;
        if (dims.contentHeight < 100) dims.contentHeight = 100;
        
        return dims;
    }
    
    DialogDimensions getBookmarkDialogDimensions() const {
        return calculateDialogDimensions(400, 300);
    }
    
    DialogDimensions getAddBookmarkDialogDimensions() const {
        return calculateDialogDimensions(400, 300);
    }
    
    DialogDimensions getHelpDialogDimensions() const {
        return calculateDialogDimensions(600, 500);
    }
    
    DialogDimensions getViewBookmarksDialogDimensions() const {
        return calculateDialogDimensions(600, 500);
    }
    
    int calculateDialogContentLength(const DialogDimensions& dims) const {
        // Calculate available width for dialog content
        int availableWidth = dims.contentWidth;
        
        // Estimate character width (assuming monospace font, average width ~8 pixels)
        int maxChars = availableWidth / 8;
        
        // Ensure reasonable minimum and maximum
        if (maxChars < 20) maxChars = 20;
        if (maxChars > 100) maxChars = 100;
        
        return maxChars;
    }
    
    // Simple XOR encryption helper functions
    std::string encrypt(const std::string& data) {
        if (!encrypted || encryptionKey.empty()) {
            return data;
        }
        
        std::string encrypted;
        encrypted.resize(data.length());
        
        for (size_t i = 0; i < data.length(); ++i) {
            encrypted[i] = data[i] ^ encryptionKey[i % encryptionKey.length()];
        }
        
        // Simple base64 encoding
        const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string result;
        int val = 0, valb = -6;
        
        for (char c : encrypted) {
            val = (val << 8) + (c & 0xff);
            valb += 8;
            while (valb >= 0) {
                result.push_back(chars[(val >> valb) & 0x3f]);
                valb -= 6;
            }
        }
        
        if (valb > -6) {
            result.push_back(chars[((val << 8) >> (valb + 8)) & 0x3f]);
        }
        
        // Pad with '='
        while (result.length() % 4) {
            result.push_back('=');
        }
        
        return result;
    }
    
    std::string decrypt(const std::string& data) {
        if (!encrypted || encryptionKey.empty()) {
            return data;
        }
        
        // Simple base64 decoding
        const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string decoded;
        int val = 0, valb = -8;
        
        for (char c : data) {
            if (c == '=') break;
            
            size_t pos = chars.find(c);
            if (pos == std::string::npos) continue;
            
            val = (val << 6) + pos;
            valb += 6;
            
            if (valb >= 0) {
                decoded.push_back((val >> valb) & 0xff);
                valb -= 8;
            }
        }
        
        // XOR decrypt
        std::string decrypted;
        decrypted.resize(decoded.length());
        
        for (size_t i = 0; i < decoded.length(); ++i) {
            decrypted[i] = decoded[i] ^ encryptionKey[i % encryptionKey.length()];
        }
        
        return decrypted;
    }
    
    // Convert hex color string to RGB value
    unsigned long hexToRgb(const std::string& hex) {
        if (hex.length() != 7 || hex[0] != '#') {
            return 0; // Default to black if invalid
        }
        
        try {
            unsigned long r = std::stoul(hex.substr(1, 2), nullptr, 16);
            unsigned long g = std::stoul(hex.substr(3, 2), nullptr, 16);
            unsigned long b = std::stoul(hex.substr(5, 2), nullptr, 16);
            return r * 256 * 256 + g * 256 + b;
        } catch (...) {
            return 0; // Default to black if conversion fails
        }
    }
    
    void loadTheme() {
        // Set default colors (console theme)
        backgroundColor = 0x000000; // Black
        textColor = 0xFFFFFF;      // White
        selectionColor = 0x333333;  // Dark gray
        borderColor = 0x888888;    // Gray
        
        // Cross-platform path separator
#ifdef _WIN32
        const char pathSep = '\\';
#else
        const char pathSep = '/';
#endif
        
        // Try user config directory first, then fallback to local themes
        std::string themePath = configDir + pathSep + "themes" + pathSep + theme + ".json";
        std::ifstream file(themePath);
        
        if (!file.is_open()) {
            // Fallback to local themes directory
            themePath = std::string("themes") + pathSep + theme + ".json";
            file.open(themePath);
        }
        
        if (!file.is_open()) {
            std::cout << "Theme file not found for theme: " << theme << ", using default colors" << std::endl;
            return;
        }
        
        std::cout << "Loading theme from: " << themePath << std::endl;
        
        // Parse JSON theme file
        std::string line;
        bool inColorsSection = false;
        while (std::getline(file, line)) {
            // Remove whitespace
            line.erase(0, line.find_first_not_of(" \t"));
            line.erase(line.find_last_not_of(" \t") + 1);
            
            // Skip empty lines and comments
            if (line.empty() || line[0] == '/' || line[0] == '#') {
                continue;
            }
            
            // Check if we're entering the colors section
            if (line.find("\"colors\"") != std::string::npos) {
                inColorsSection = true;
                continue;
            }
            
            // Check if we're exiting the colors section
            if (inColorsSection && line.find("}") != std::string::npos) {
                break;
            }
            
            // Parse color values (only in JSON format)
            if (inColorsSection) {
                if (line.find("\"background\"") != std::string::npos) {
                    size_t start = line.find('"', line.find(':'));
                    size_t end = line.find('"', start + 1);
                    if (start != std::string::npos && end != std::string::npos) {
                        backgroundColor = hexToRgb(line.substr(start + 1, end - start - 1));
                    }
                }
                else if (line.find("\"text\"") != std::string::npos) {
                    size_t start = line.find('"', line.find(':'));
                    size_t end = line.find('"', start + 1);
                    if (start != std::string::npos && end != std::string::npos) {
                        textColor = hexToRgb(line.substr(start + 1, end - start - 1));
                    }
                }
                else if (line.find("\"selection\"") != std::string::npos) {
                    size_t start = line.find('"', line.find(':'));
                    size_t end = line.find('"', start + 1);
                    if (start != std::string::npos && end != std::string::npos) {
                        selectionColor = hexToRgb(line.substr(start + 1, end - start - 1));
                    }
                }
                else if (line.find("\"border\"") != std::string::npos) {
                    size_t start = line.find('"', line.find(':'));
                    size_t end = line.find('"', start + 1);
                    if (start != std::string::npos && end != std::string::npos) {
                        borderColor = hexToRgb(line.substr(start + 1, end - start - 1));
                    }
                }
            }
        }
        file.close();
        
        // Update graphics context with new text color
#ifdef __linux__
        if (gc) {
            XSetForeground(display, gc, textColor);
        }
#endif
    }
    
    void createDefaultThemeFile() {
        // Create themes directory if it doesn't exist
        std::string themesDir = configDir + "/themes";
        struct stat st = {0};
        if (stat(themesDir.c_str(), &st) == -1) {
#ifdef _WIN32
            mkdir(themesDir.c_str());
#else
            mkdir(themesDir.c_str(), 0755);
#endif
        }
        
        // Create default console theme file with valid JSON
        std::string themeFile = themesDir + "/console.json";
        std::ofstream outFile(themeFile);
        if (outFile.is_open()) {
            outFile << "{\n";
            outFile << "  \"name\": \"Console\",\n";
            outFile << "  \"description\": \"Default console theme with black background and green text\",\n";
            outFile << "  \"colors\": {\n";
            outFile << "    \"background\": \"#000000\",\n";
            outFile << "    \"text\": \"#00FF00\",\n";
            outFile << "    \"selection\": \"#333333\",\n";
            outFile << "    \"border\": \"#00FF00\"\n";
            outFile << "  }\n";
            outFile << "}\n";
            outFile.close();
            std::cout << "Created default theme file: " << themeFile << std::endl;
        }
    }
    
    void executeCommand(const std::string& command) {
        // Parse command and arguments
        std::istringstream iss(command);
        std::string cmd;
        std::string args;
        
        if (iss >> cmd) {
            std::getline(iss, args);
            // Trim leading whitespace from args
            if (!args.empty() && args[0] == ' ') {
                args = args.substr(1);
            }
        }
        
        if (cmd == "theme") {
            if (!args.empty()) {
                // Direct theme switch: "theme dracula"
                switchTheme(args);
            } else {
                // Enter theme selection mode: "theme"
                commandMode = false;
                cmd_themeSelectMode = true;
                discoverThemes();
                // Store original theme and apply first theme for preview
                if (!availableThemes.empty()) {
                    originalTheme = theme;
                    selectedTheme = 0;
                    switchTheme(availableThemes[0]);
                }
                drawConsole();
            }
            return;
        }
        
        // Handle other commands (for future implementation)
        std::cout << "Command executed: " << command << std::endl;
        
        // TODO: Add specific command implementations here
        // Examples:
        // - "delete" - delete selected item
        // - "pin" - pin selected item  
        // - "clear" - clear all items
        // - "export" - export clipboard history
    }
    
    void discoverThemes() {
        availableThemes.clear();
        
        // Try user themes directory first
        std::string userThemesDir = configDir + "/themes";
        std::string localThemesDir = "themes";
        
        // Function to scan a directory for theme files
        auto scanThemesDir = [&](const std::string& themesDir) {
            DIR* dir = opendir(themesDir.c_str());
            if (dir) {
                struct dirent* entry;
                while ((entry = readdir(dir)) != nullptr) {
                    std::string filename = entry->d_name;
                    // Check if it's a .json file and not a special entry
                    if (filename.length() > 5 && filename.substr(filename.length() - 5) == ".json" &&
                        filename != "." && filename != "..") {
                        // Remove .json extension
                        std::string themeName = filename.substr(0, filename.length() - 5);
                        availableThemes.push_back(themeName);
                    }
                }
                closedir(dir);
            }
        };
        
        // Scan both directories
        scanThemesDir(userThemesDir);
        scanThemesDir(localThemesDir);
        
        // Remove duplicates and sort
        std::sort(availableThemes.begin(), availableThemes.end());
        availableThemes.erase(std::unique(availableThemes.begin(), availableThemes.end()), availableThemes.end());
        
        // Reset selection
        selectedTheme = 0;
        themeSelectScrollOffset = 0;
    }
    
    void switchTheme(const std::string& themeName) {
        theme = themeName;
        loadTheme();
        std::cout << "Switched to theme: " << themeName << std::endl;
    }
    
    void loadBookmarkGroups() {
        // Cross-platform path separator
#ifdef _WIN32
        const char pathSep = '\\';
#else
        const char pathSep = '/';
#endif
        
        std::string bookmarkFile = bookmarksDir + pathSep + "bookmarks.txt";
        std::ifstream file(bookmarkFile);
        bookmarkGroups.clear();
        
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                if (!line.empty() && line.find('|') != std::string::npos) {
                    size_t pos = line.find('|');
                    std::string groupName = line.substr(0, pos);
                    if (!groupName.empty()) {
                        bookmarkGroups.push_back(groupName);
                    }
                }
            }
            file.close();
        }
        
        // Always ensure we have at least one group
        if (bookmarkGroups.empty()) {
            bookmarkGroups.push_back("default");
        }
    }
    
    void saveBookmarkGroups() {
        std::string bookmarkFile = bookmarksDir + "/bookmarks.txt";
        std::ofstream file(bookmarkFile);
        
        if (file.is_open()) {
            for (const auto& group : bookmarkGroups) {
                file << group << "|0\n"; // Group name | clip count
            }
            file.close();
        }
    }
    
    void addClipToBookmarkGroup(const std::string& groupName, const std::string& content) {
        std::string bookmarkFile = bookmarksDir + "/bookmarks_" + groupName + ".txt";
        std::ofstream file(bookmarkFile, std::ios::app);
        
        if (file.is_open()) {
            auto timestamp = std::chrono::system_clock::now().time_since_epoch().count();
            std::string contentToSave = encrypt(content);
            file << timestamp << "|" << contentToSave << "\n";
            file.close();
        }
    }

    void addClipToPinned(const std::string& content) {
        std::ofstream file(pinnedFile, std::ios::app);
        
        if (file.is_open()) {
            auto timestamp = std::chrono::system_clock::now().time_since_epoch().count();
            std::string contentToSave = encrypt(content);
            file << timestamp << "|" << contentToSave << "\n";
            file.close();
        }
    }
    
    void setupConfigDir() {
        // Cross-platform home directory detection
        const char* home = nullptr;
        
#ifdef _WIN32
        home = getenv("USERPROFILE");
        if (!home) home = getenv("APPDATA");
#elif __APPLE__
        home = getenv("HOME");
#elif __linux__
        home = getenv("HOME");
#endif
        
        if (!home) home = ".";
        
        // Cross-platform config directory path
#ifdef _WIN32
        configDir = std::string(home) + "\\mmry";
#elif __APPLE__
        configDir = std::string(home) + "/Library/Application Support/mmry";
#elif __linux__
        configDir = std::string(home) + "/.config/mmry";
#endif
        
        // Cross-platform directory creation
        auto createDirectory = [](const std::string& path) {
#ifdef _WIN32
            return mkdir(path.c_str()) == 0 || errno == EEXIST;
#else
            return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;
#endif
        };
        
        // Create config directory if it doesn't exist
        struct stat st = {0};
        if (stat(configDir.c_str(), &st) == -1) {
            if (createDirectory(configDir)) {
                std::cout << "Created config directory: " << configDir << std::endl;
            } else {
                std::cerr << "Failed to create config directory: " << configDir << std::endl;
            }
        }
        
        // Cross-platform path separator
#ifdef _WIN32
        const char pathSep = '\\';
#else
        const char pathSep = '/';
#endif
        
        // Create themes directory if it doesn't exist
        std::string themesDir = configDir + pathSep + "themes";
        if (stat(themesDir.c_str(), &st) == -1) {
            if (createDirectory(themesDir)) {
                std::cout << "Created themes directory: " << themesDir << std::endl;
            } else {
                std::cerr << "Failed to create themes directory: " << themesDir << std::endl;
            }
        }
        
        // Create bookmarks directory if it doesn't exist
        bookmarksDir = configDir + pathSep + "bookmarks";
        if (stat(bookmarksDir.c_str(), &st) == -1) {
            if (createDirectory(bookmarksDir)) {
                std::cout << "Created bookmarks directory: " << bookmarksDir << std::endl;
            } else {
                std::cerr << "Failed to create bookmarks directory: " << bookmarksDir << std::endl;
            }
        }
        
        dataFile = configDir + pathSep + "clips.txt";
        pinnedFile = configDir + pathSep + "pinned.txt";
        
        // Ensure all required files exist
        ensureRequiredFiles();
    }
    
    void ensureRequiredFiles() {
        // Check and create config.json if needed
        std::string configFile = configDir + "/config.json";
        struct stat st = {0};
        if (stat(configFile.c_str(), &st) == -1) {
            createDefaultConfig();
        }
        
        // Check and create theme file if needed
        std::string themeFile = configDir + "/themes/" + theme + ".json";
        if (stat(themeFile.c_str(), &st) == -1) {
            createDefaultThemeFile();
        }
        
        // Check and create bookmarks.txt if needed
        std::string bookmarksFile = bookmarksDir + "/bookmarks.txt";
        if (stat(bookmarksFile.c_str(), &st) == -1) {
            std::ofstream outFile(bookmarksFile);
            if (outFile.is_open()) {
                outFile << "default|0\n";
                outFile.close();
                std::cout << "Created bookmarks file: " << bookmarksFile << std::endl;
            }
        }
        
        // Check and create clips.txt if needed
        if (stat(dataFile.c_str(), &st) == -1) {
            std::ofstream outFile(dataFile);
            if (outFile.is_open()) {
                outFile.close();
                std::cout << "Created clips file: " << dataFile << std::endl;
            }
        }
        // Check and create pinned.txt if needed
        if (stat(pinnedFile.c_str(), &st) == -1) {
            std::ofstream outFile(pinnedFile);
            if (outFile.is_open()) {
                outFile.close();
                std::cout << "Created pinned clips file: " << pinnedFile << std::endl;
            }
        }
    }
    
    void createWindow() {
#ifdef __linux__
        // Create window with theme colors
        window = XCreateSimpleWindow(display, root, 
                                   WINDOW_X, WINDOW_Y, 
                                   windowWidth, windowHeight,
                                   2, borderColor, backgroundColor);
        
        // Set window properties
        XStoreName(display, window, "MMRY");
        XSelectInput(display, window, ExposureMask | KeyPressMask | StructureNotifyMask);
        
        // Set minimum window size constraints
        XSizeHints hints;
        hints.flags = PMinSize;
        hints.min_width = MIN_WINDOW_WIDTH;
        hints.min_height = MIN_WINDOW_HEIGHT;
        XSetWMNormalHints(display, window, &hints);
        
        // Create graphics context
        gc = XCreateGC(display, window, 0, nullptr);
        XSetForeground(display, gc, textColor);
        
        // Load font (try to find a monospace font)
        font = XLoadQueryFont(display, "-*-fixed-medium-r-*-*-13-*-*-*-*-*-*-*");
        if (!font) {
            font = XLoadQueryFont(display, "fixed");
        }
        if (font) {
            XSetFont(display, gc, font->fid);
        }
        
        // Set window to be always on top and skip taskbar
        XWMHints wmHints;
        wmHints.flags = InputHint | StateHint;
        wmHints.input = True;
        wmHints.initial_state = NormalState;
        XSetWMHints(display, window, &wmHints);
        
        // Set window type to dialog
        Atom atom_type = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
        Atom atom_dialog = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DIALOG", False);
        XChangeProperty(display, window, atom_type, XA_ATOM, 32, PropModeReplace, 
                       (unsigned char*)&atom_dialog, 1);
#endif

#ifdef _WIN32
        // Windows window creation
#endif

#ifdef __APPLE__
        // macOS window creation
#endif
    }
    
    void setupHotkeys() {
#ifdef __linux__
        // First, ensure X11 is fully synchronized
        XSync(display, False);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        // Install X11 error handler to catch grab failures
        auto oldHandler = XSetErrorHandler([](Display* d, XErrorEvent* e) -> int {
            if (e->error_code == BadAccess) {
                std::cerr << "X11 Error: BadAccess when trying to grab key" << std::endl;
                return 0;
            }
            return 0;
        });
        
        // Try multiple times with exponential backoff
        const int MAX_RETRIES = 5;
        bool success = false;
        
        for (int retry = 0; retry < MAX_RETRIES && !success; retry++) {
            if (retry > 0) {
                std::cerr << "Retry " << retry << " of " << MAX_RETRIES << "..." << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(500 * retry));
            }
            
            // First ungrab any existing grabs
            XUngrabKey(display, AnyKey, AnyModifier, root);
            XSync(display, False);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            // Try to grab the key
            KeyCode keycode = XKeysymToKeycode(display, XK_c);
            
            // Grab with all possible combinations of NumLock and CapsLock
            // since these can interfere with modifier detection
            unsigned int modifiers[] = {
                ControlMask | Mod1Mask,                    // Ctrl+Alt
                ControlMask | Mod1Mask | Mod2Mask,         // Ctrl+Alt+NumLock
                ControlMask | Mod1Mask | LockMask,         // Ctrl+Alt+CapsLock
                ControlMask | Mod1Mask | Mod2Mask | LockMask // Ctrl+Alt+NumLock+CapsLock
            };
            
            bool grabFailed = false;
            for (unsigned int mod : modifiers) {
                int result = XGrabKey(display, keycode, mod, root, True, 
                                     GrabModeAsync, GrabModeAsync);
                
                // Force synchronization to detect errors immediately
                XSync(display, False);
                
                if (result == BadAccess) {
                    grabFailed = true;
                    break;
                }
            }
            
            if (!grabFailed) {
                success = true;
                hotkeyGrabbed = true;
                std::cout << "Successfully grabbed Ctrl+Alt+C hotkey" << std::endl;
            }
        }
        
        // Restore old error handler
        XSetErrorHandler(oldHandler);
        
        if (!success) {
            std::cerr << "CRITICAL: Failed to grab Ctrl+Alt+C hotkey after " 
                      << MAX_RETRIES << " attempts!" << std::endl;
            std::cerr << "Another application may be using this hotkey." << std::endl;
            std::cerr << "MMRY will still work, but you'll need to focus the window manually." << std::endl;
        }
        
        // Select KeyPress events on root window
        XSelectInput(display, root, KeyPressMask | KeyReleaseMask);
        XSync(display, False);
#endif
    }

    
    void showWindow() {
        std::cout << "Visible: " << visible << std::endl;

        if (!visible) {
#ifdef __linux__
            XMapWindow(display, window);
#endif
#ifdef _WIN32
            ShowWindow(hwnd, SW_SHOW);
#endif
            visible = true;
            std::cout << "Window shown" << std::endl;
        }
    }
    
    void hideWindow() {
        if (visible) {
#ifdef __linux__
            XUnmapWindow(display, window);
#endif
#ifdef _WIN32
            if (hwnd) {
                std::cout << "Calling ShowWindow(SW_HIDE)" << std::endl;
                ShowWindow(hwnd, SW_HIDE);
            } else {
                std::cout << "hwnd is null!" << std::endl;
            }
#endif
            visible = false;
            std::cout << "Window hidden" << std::endl;
        }
    }
    
    void drawBookmarkDialog() {
#ifdef __linux__
        if (!bookmarkDialogVisible) return;
        
        // Get dynamic dialog dimensions
        DialogDimensions dims = getBookmarkDialogDimensions();
        
        // Draw dialog background
        XSetForeground(display, gc, backgroundColor);
        XFillRectangle(display, window, gc, dims.x, dims.y, dims.width, dims.height);
        XSetForeground(display, gc, borderColor);
        XDrawRectangle(display, window, gc, dims.x, dims.y, dims.width, dims.height);
        
        // Draw title
        std::string title = "Bookmark Groups";
        int titleWidth = XTextWidth(font, title.c_str(), title.length());
        XDrawString(display, window, gc, dims.x + (dims.width - titleWidth) / 2, dims.y + 25, title.c_str(), title.length());
        
        // Draw input field
        XSetForeground(display, gc, textColor);
        XDrawString(display, window, gc, dims.x + 20, dims.y + 60, "New group name:", 16);
        
        // Draw input box
        XSetForeground(display, gc, selectionColor);
        XFillRectangle(display, window, gc, dims.x + 20, dims.y + 70, dims.width - 40, 25);
        XSetForeground(display, gc, textColor);
        XDrawRectangle(display, window, gc, dims.x + 20, dims.y + 70, dims.width - 40, 25);
        
        // Draw input text
        std::string displayInput = bookmarkDialogInput + "_";
        XDrawString(display, window, gc, dims.x + 25, dims.y + 87, displayInput.c_str(), displayInput.length());
        
        // Draw existing groups
        XSetForeground(display, gc, textColor);
        XDrawString(display, window, gc, dims.x + 20, dims.y + 120, "Existing groups:", 15);
        
        // Filter and display groups
        std::vector<std::string> filteredGroups;
        for (const auto& group : bookmarkGroups) {
            if (bookmarkDialogInput.empty() || group.find(bookmarkDialogInput) != std::string::npos) {
                filteredGroups.push_back(group);
            }
        }
        
        int y = dims.y + 140;
        const int VISIBLE_ITEMS = 8;
        
        size_t startIdx = bookmarkMgmtScrollOffset;
        size_t endIdx = std::min(startIdx + VISIBLE_ITEMS, filteredGroups.size());
        
        for (size_t i = startIdx; i < endIdx; ++i) {
            std::string displayText = "  " + filteredGroups[i];
            if (i == selectedBookmarkGroup) {
                displayText = "  " + filteredGroups[i];
                // Highlight selected
                XSetForeground(display, gc, selectionColor);
                XFillRectangle(display, window, gc, dims.x + 15, y - 12, dims.width - 30, 15);
                XSetForeground(display, gc, textColor);
            }
            XDrawString(display, window, gc, dims.x + 20, y, displayText.c_str(), displayText.length());
            y += 18;
        }

#endif
    }
    
    void drawAddToBookmarkDialog() {
#ifdef __linux__
        if (!addToBookmarkDialogVisible) return;
        
        // Get dynamic dialog dimensions
        DialogDimensions dims = getAddBookmarkDialogDimensions();
        
        // Draw dialog background
        XSetForeground(display, gc, backgroundColor);
        XFillRectangle(display, window, gc, dims.x, dims.y, dims.width, dims.height);
        XSetForeground(display, gc, borderColor);
        XDrawRectangle(display, window, gc, dims.x, dims.y, dims.width, dims.height);
        
        // Draw title
        std::string title = "Add to Bookmark Group";
        int titleWidth = XTextWidth(font, title.c_str(), title.length());
        XDrawString(display, window, gc, dims.x + (dims.width - titleWidth) / 2, dims.y + 25, title.c_str(), title.length());
        
        // Draw bookmark groups
        XSetForeground(display, gc, textColor);
        
        int y = dims.y + 50;
        const int VISIBLE_ITEMS = 10;
        
        size_t startIdx = addBookmarkScrollOffset;
        size_t endIdx = std::min(startIdx + VISIBLE_ITEMS, bookmarkGroups.size());
        
        for (size_t i = startIdx; i < endIdx; ++i) {
            std::string displayText = "  " + bookmarkGroups[i];
            if (i == selectedAddBookmarkGroup) {
                displayText = "> " + bookmarkGroups[i];
                // Highlight selected
                XSetForeground(display, gc, selectionColor);
                XFillRectangle(display, window, gc, dims.x + 15, y - 12, dims.width - 30, 15);
                XSetForeground(display, gc, textColor);
            }
            XDrawString(display, window, gc, dims.x + 20, y, displayText.c_str(), displayText.length());
            y += 18;
        }
#endif
    }

    void drawHelpTopic(int x, int y, int contentTop, int contentBottom, std::string topic) {
        int topicLen = topic.length();
#ifdef __linux__
        if (y >= contentTop && y < contentBottom) { XDrawString(display, window, gc, x, y, topic.c_str(), topicLen); }
#endif
    }

    void drawHelpDialog() {
#ifdef __linux__
        if (!helpDialogVisible) return;
        
        // Get dynamic dialog dimensions
        DialogDimensions dims = getHelpDialogDimensions();
        
        // Draw dialog background
        XSetForeground(display, gc, backgroundColor);
        XFillRectangle(display, window, gc, dims.x, dims.y, dims.width, dims.height);
        XSetForeground(display, gc, borderColor);
        XDrawRectangle(display, window, gc, dims.x, dims.y, dims.width, dims.height);
        
        // Draw help content
        XSetForeground(display, gc, textColor);
        int y = dims.y + 20;
        const int contentTop = y;
        const int contentBottom = dims.y + dims.height;
        const int titleLeft = dims.x + 20;
        const int topicLeft = dims.x + 30;
        const int lineHeight = 15;
        const int gap = 10;

        y = y + helpDialogScrollOffset;

        // !@!
        // Main Window shortcuts
        drawHelpTopic(titleLeft, y, contentTop, contentBottom, "Main Window:");
        y += lineHeight;
        drawHelpTopic(topicLeft, y, contentTop, contentBottom, "j/k            - Navigate items");
        y += lineHeight;
        drawHelpTopic(topicLeft, y, contentTop, contentBottom, "g/G            - Top/bottom");
        y += lineHeight;
        drawHelpTopic(topicLeft, y, contentTop, contentBottom, "Enter          - Copy item");
        y += lineHeight;
        drawHelpTopic(topicLeft, y, contentTop, contentBottom, "/              - Filter mode");
        y += lineHeight;
        drawHelpTopic(topicLeft, y, contentTop, contentBottom, "Shift+M        - Manage bookmark groups");
        y += lineHeight;
        drawHelpTopic(topicLeft, y, contentTop, contentBottom, "m              - Add clip to group");
        y += lineHeight;
        drawHelpTopic(topicLeft, y, contentTop, contentBottom, "`              - View bookmarks");
        y += lineHeight;
        drawHelpTopic(topicLeft, y, contentTop, contentBottom, "?              - This help");
        y += lineHeight;
        drawHelpTopic(topicLeft, y, contentTop, contentBottom, "Shift+D        - Delete item");
        y += lineHeight;
        drawHelpTopic(topicLeft, y, contentTop, contentBottom, "Shift+Q        - Quit");
        y += lineHeight;
        drawHelpTopic(topicLeft, y, contentTop, contentBottom, "Escape         - Hide window");
        y += lineHeight + gap;
        
        // Help
        drawHelpTopic(titleLeft, y, contentTop, contentBottom, "Help Window:");
        y += lineHeight;
        drawHelpTopic(topicLeft, y, contentTop, contentBottom, "j/k            - Navigate topics");
        y += lineHeight;
        drawHelpTopic(topicLeft, y, contentTop, contentBottom, "g              - Top");
        y += lineHeight + gap;


        // Filter Mode shortcuts
        drawHelpTopic(titleLeft, y, contentTop, contentBottom, "Filter Mode:");
        y += lineHeight;
        drawHelpTopic(topicLeft, y, contentTop, contentBottom, "Type text      - Filter items");
        y += lineHeight;
        drawHelpTopic(topicLeft, y, contentTop, contentBottom, "Backspace      - Delete char");
        y += lineHeight;
        drawHelpTopic(topicLeft, y, contentTop, contentBottom, "Up/down arrow  - Navigate items");
        y += lineHeight;
        drawHelpTopic(topicLeft, y, contentTop, contentBottom, "Delete         - Delete item");
        y += lineHeight;
        drawHelpTopic(topicLeft, y, contentTop, contentBottom, "Enter          - Copy item");
        y += lineHeight;
        drawHelpTopic(topicLeft, y, contentTop, contentBottom, "Escape         - Exit filter");
        y += lineHeight + gap;
        
        // Add bookmark group shortcuts
        drawHelpTopic(titleLeft, y, contentTop, contentBottom, "Add Bookmark Group Dialog:");
        y += lineHeight;
        drawHelpTopic(topicLeft, y, contentTop, contentBottom, "Type text      - Define Group Name / Filter Existing");
        y += lineHeight;
        drawHelpTopic(topicLeft, y, contentTop, contentBottom, "Backspace      - Delete char");
        y += lineHeight;
        drawHelpTopic(topicLeft, y, contentTop, contentBottom, "Enter          - Create group");
        y += lineHeight;
        drawHelpTopic(topicLeft, y, contentTop, contentBottom, "Escape         - Exit dialog");
        y += lineHeight + gap;

        // Add clip to group shortcuts
        drawHelpTopic(titleLeft, y, contentTop, contentBottom, "Add Clip to Group Dialog:");
        y += lineHeight;
        drawHelpTopic(topicLeft, y, contentTop, contentBottom, "j/k            - Navigate group");
        y += lineHeight;
        drawHelpTopic(topicLeft, y, contentTop, contentBottom, "g/G            - Top/bottom");
        y += lineHeight;
        drawHelpTopic(topicLeft, y, contentTop, contentBottom, "Enter          - Add clip to group");
        y += lineHeight;
        drawHelpTopic(topicLeft, y, contentTop, contentBottom, "Escape         - Exit dialog");
        y += lineHeight + gap;

        // View/Edit/Use bookmarks
        drawHelpTopic(titleLeft, y, contentTop, contentBottom, "View/Delete/Use Bookmarks Dialog");
        y += lineHeight;
        drawHelpTopic(topicLeft, y, contentTop, contentBottom, "j/k            - Navigate groups/clips");
        y += lineHeight;
        drawHelpTopic(topicLeft, y, contentTop, contentBottom, "g/G            - Top/bottom");
        y += lineHeight;
        drawHelpTopic(topicLeft, y, contentTop, contentBottom, "Enter          - View group clips/copy clip");
        y += lineHeight;
        drawHelpTopic(topicLeft, y, contentTop, contentBottom, "h              - Back to groups list");
        y += lineHeight;
        drawHelpTopic(topicLeft, y, contentTop, contentBottom, "Escape         - Exit dialog");
        y += lineHeight + gap + 5;

        // Commands
        drawHelpTopic(titleLeft, y, contentTop, contentBottom, "Commands");
        y += lineHeight;
        drawHelpTopic(topicLeft, y, contentTop, contentBottom, ":              - Activate commands");
        y += lineHeight;
        drawHelpTopic(topicLeft, y, contentTop, contentBottom, "theme          - Select theme to apply");
        y += lineHeight;
        drawHelpTopic(topicLeft, y, contentTop, contentBottom, "Enter          - Apply command");
        y += lineHeight;
        drawHelpTopic(topicLeft, y, contentTop, contentBottom, "Escape         - Cancel command");
        y += lineHeight + gap + 5;


        // Global hotkey
        drawHelpTopic(titleLeft, y, contentTop, contentBottom, "Global Hotkey:");
        y += lineHeight;
        drawHelpTopic(topicLeft, y, contentTop, contentBottom, "Ctrl+Alt+C     - Show/hide window");
        
#endif
    }
    
    void drawViewBookmarksDialog() {
#ifdef __linux__
        if (!viewBookmarksDialogVisible) return;
        
        // Get dynamic dialog dimensions
        DialogDimensions dims = getViewBookmarksDialogDimensions();
        
        // Draw dialog background
        XSetForeground(display, gc, backgroundColor);
        XFillRectangle(display, window, gc, dims.x, dims.y, dims.width, dims.height);
        XSetForeground(display, gc, borderColor);
        XDrawRectangle(display, window, gc, dims.x, dims.y, dims.width, dims.height);
        
        // Draw title
        std::string title = viewBookmarksShowingGroups ? "Select Bookmark Group" : "View Bookmarks: " + bookmarkGroups[selectedViewBookmarkGroup];
        int titleWidth = XTextWidth(font, title.c_str(), title.length());
        XDrawString(display, window, gc, dims.x + (dims.width - titleWidth) / 2, dims.y + 25, title.c_str(), title.length());
        
        if (viewBookmarksShowingGroups) {
            // Show bookmark groups list with scrolling
            XSetForeground(display, gc, textColor);
            int y = dims.y + 60;
            const int VISIBLE_ITEMS = 20;
            
            size_t startIdx = viewBookmarksScrollOffset;
            size_t endIdx = std::min(startIdx + VISIBLE_ITEMS, bookmarkGroups.size());
            
            for (size_t i = startIdx; i < endIdx; ++i) {
                std::string displayText = bookmarkGroups[i];
                
                // Add selection indicator
                if (i == selectedViewBookmarkGroup) {
                    displayText = "> " + displayText;
                    // Highlight selected
                    XSetForeground(display, gc, selectionColor);
                    XFillRectangle(display, window, gc, dims.x + 15, y - 12, dims.width - 30, 15);
                    XSetForeground(display, gc, textColor);
                } else {
                    displayText = "  " + displayText;
                }
                
                XDrawString(display, window, gc, dims.x + 20, y, displayText.c_str(), displayText.length());
                y += 18;
            }
            
        } else {
            // Show bookmark items for selected group
                    if (selectedViewBookmarkGroup < bookmarkGroups.size()) {
                        std::string selectedGroup = bookmarkGroups[selectedViewBookmarkGroup];
                        std::string bookmarkFile = bookmarksDir + "/bookmarks_" + selectedGroup + ".txt";
                        std::ifstream file(bookmarkFile);
                
                if (file.is_open()) {
                    std::string line;
                    std::vector<std::string> bookmarkItems;
                    
                    while (std::getline(file, line)) {
                        size_t pos = line.find('|');
                        if (pos != std::string::npos && pos > 0) {
                            std::string content = line.substr(pos + 1);
                            try {
                                std::string decryptedContent = decrypt(content);
                                bookmarkItems.push_back(decryptedContent);
                            } catch (...) {
                                bookmarkItems.push_back(content);
                            }
                        }
                    }
                    file.close();
                    
                    // Draw items with scrolling
                    int itemY = dims.y + 60;
                    const int VISIBLE_ITEMS = 20;
                    
                    size_t startIdx = viewBookmarksScrollOffset;
                    size_t endIdx = std::min(startIdx + VISIBLE_ITEMS, bookmarkItems.size());
                    
                    for (size_t i = startIdx; i < endIdx; ++i) {
                        std::string displayText = bookmarkItems[i];
                        
                        // Truncate if too long
                        int maxContentLength = calculateDialogContentLength(dims);
                        if (displayText.length() > maxContentLength) {
                            displayText = smartTrim(displayText, maxContentLength);
                        }
                        
                        // Replace newlines with spaces for display
                        for (char& c : displayText) {
                            if (c == '\n' || c == '\r') c = ' ';
                        }
                        
                        // Add selection indicator
                        if (i == selectedViewBookmarkItem) {
                            displayText = "> " + displayText;
                            // Highlight selected
                            XSetForeground(display, gc, selectionColor);
                            XFillRectangle(display, window, gc, dims.x + 15, itemY - 12, dims.width - 30, 15);
                            XSetForeground(display, gc, textColor);
                        } else {
                            XSetForeground(display, gc, selectionColor);
                            displayText = "  " + displayText;
                        }
                        
                        XDrawString(display, window, gc, dims.x + 20, itemY, displayText.c_str(), displayText.length());
                        itemY += 18;
                    }
                    
                    if (bookmarkItems.empty()) {
                        XDrawString(display, window, gc, dims.x + 20, itemY, "No bookmarks in this group", 26);
                    }
                }
            }
            

        }
        
#endif
    }

    void drawPinnedDialog() {
#ifdef __linux__
        if (!pinnedDialogVisible) return;
        
        // Get dynamic dialog dimensions
        DialogDimensions dims = getViewBookmarksDialogDimensions();
        
        // Draw dialog background
        XSetForeground(display, gc, backgroundColor);
        XFillRectangle(display, window, gc, dims.x, dims.y, dims.width, dims.height);
        XSetForeground(display, gc, borderColor);
        XDrawRectangle(display, window, gc, dims.x, dims.y, dims.width, dims.height);
        
        // Draw title
        std::string title = "Pinned Clips";
        int titleWidth = XTextWidth(font, title.c_str(), title.length());
        XDrawString(display, window, gc, dims.x + (dims.width - titleWidth) / 2, dims.y + 25, title.c_str(), title.length());
        
                        std::ifstream file(pinnedFile);
        
                            
        
                        if (file.is_open()) {
        
                            std::string line;
        
                            std::vector<std::pair<long long, std::string>> pinnedItems;
        
                            
        
                            while (std::getline(file, line)) {
        
                                size_t pos = line.find('|');
        
                                if (pos != std::string::npos && pos > 0) {
        
                                    std::string timestampStr = line.substr(0, pos);
        
                                    std::string content = line.substr(line.find('|') + 1);
        
                                    try {
        
                                        std::string decryptedContent = decrypt(content);
        
                                        long long timestamp = std::stoll(timestampStr);
        
                                        pinnedItems.push_back({timestamp, decryptedContent});
        
                                    } catch (...) {
        
                                        try {
        
                                            long long timestamp = std::stoll(timestampStr);
        
                                            pinnedItems.push_back({timestamp, content});
        
                                        } catch (...) {
        
                                            // If timestamp parsing fails, use current time
        
                                            long long timestamp = std::chrono::system_clock::now().time_since_epoch().count();
        
                                            pinnedItems.push_back({timestamp, content});
        
                                        }
        
                                    }
        
                                }
        
                            }
        
                            file.close();
        
                            
        
                            // Sort by timestamp in descending order (most recent first)
        
                            std::sort(pinnedItems.begin(), pinnedItems.end(), 
        
                                     [](const auto& a, const auto& b) { return a.first > b.first; });
        
                            
        
                            // Draw items with scrolling
        
                            int itemY = dims.y + 60;
        
                            const int VISIBLE_ITEMS = 20;
        
                            
        
                            size_t startIdx = viewPinnedScrollOffset;  // Changed from viewBookmarksScrollOffset
        
                            size_t endIdx = std::min(startIdx + VISIBLE_ITEMS, pinnedItems.size());
        
                            
        
                            for (size_t i = startIdx; i < endIdx; ++i) {
        
                                std::string displayText = pinnedItems[i].second;
        
        
                
                // Truncate if too long
                int maxContentLength = calculateDialogContentLength(dims);
                if (displayText.length() > maxContentLength) {
                    displayText = smartTrim(displayText, maxContentLength);
                }
                
                // Replace newlines with spaces for display
                for (char& c : displayText) {
                    if (c == '\n' || c == '\r') c = ' ';
                }
                
                // Add selection indicator
                if (i == selectedViewPinnedItem) {  // Changed from selectedViewBookmarkItem
                    displayText = "> " + displayText;
                    // Highlight selected
                    XSetForeground(display, gc, selectionColor);
                    XFillRectangle(display, window, gc, dims.x + 15, itemY - 12, dims.width - 30, 15);
                    XSetForeground(display, gc, textColor);
                } else {
                    XSetForeground(display, gc, selectionColor);
                    displayText = "  " + displayText;
                }
                
                XDrawString(display, window, gc, dims.x + 20, itemY, displayText.c_str(), displayText.length());
                itemY += 18;
            }
            
            if (pinnedItems.empty()) {
                XDrawString(display, window, gc, dims.x + 20, itemY, "No pinned clips", 16);
            }
        }
#endif
    }

public:
    void drawConsole() {
        if (!visible) return;
        
#ifdef __linux__
        // Clear window with theme background
        XSetWindowBackground(display, window, backgroundColor);
        XClearWindow(display, window);
        
        // Draw filter or command textbox if in respective mode
        int startY = 20;
        if (filterMode) {
            // Draw filter input
            std::string filterDisplay = "/" + filterText;
            XDrawString(display, window, gc, 10, startY, filterDisplay.c_str(), filterDisplay.length());
            startY += 25;
        }
        else if (commandMode) {
            // Draw command input
            std::string commandDisplay = ":" + commandText;
            XDrawString(display, window, gc, 10, startY, commandDisplay.c_str(), commandDisplay.length());
            startY += 25;
        }
        else if (cmd_themeSelectMode) {
            // Draw theme selection header
            std::string header = "Select theme (" + std::to_string(availableThemes.size()) + " total):";
            XDrawString(display, window, gc, 10, startY, header.c_str(), header.length());
            startY += 25;
            
            // Draw theme list
            const int VISIBLE_THEMES = 10;
            size_t startIdx = themeSelectScrollOffset;
            size_t endIdx = std::min(startIdx + VISIBLE_THEMES, availableThemes.size());
            
            for (size_t i = startIdx; i < endIdx; ++i) {
                std::string themeDisplay = (i == selectedTheme ? "> " : "  ") + availableThemes[i];
                XDrawString(display, window, gc, 10, startY, themeDisplay.c_str(), themeDisplay.length());
                startY += LINE_HEIGHT;
            }
            
            // Show scroll indicator if there are more themes
            if (availableThemes.size() > VISIBLE_THEMES) {
                std::string scrollInfo = "Showing " + std::to_string(startIdx + 1) + "-" + std::to_string(endIdx) + " of " + std::to_string(availableThemes.size());
                XDrawString(display, window, gc, 10, startY, scrollInfo.c_str(), scrollInfo.length());
            }
            
            // Don't draw clipboard items in theme selection mode
            return;
        }
        
        // Draw clipboard items
        // TODO: Remove all the hardcoded values
        int y = startY;
        size_t displayCount = getDisplayItemCount();
        const int SCROLL_INDICATOR_HEIGHT = 15;
        int availableHeight = windowHeight - startY - 10;

        // If we need a scroll indicator, account for its space
        if (displayCount > (availableHeight / LINE_HEIGHT)) {
            availableHeight -= SCROLL_INDICATOR_HEIGHT;
        }

        int maxItems = availableHeight / LINE_HEIGHT;
        if (maxItems > 0) maxItems += 1;
        if (maxItems < 1) maxItems = 1;

        size_t startIdx = consoleScrollOffset;
        size_t endIdx = std::min(startIdx + maxItems, displayCount);

        // Adjust for the scroll indicator
        if (displayCount > maxItems) {
            std::string scrollText = "[" + std::to_string(selectedItem + 1) + "/" + std::to_string(displayCount) + "]";
            XDrawString(display, window, gc, windowWidth - 80, 15, scrollText.c_str(), scrollText.length());
            y = y + 15;
            maxItems = maxItems - 1;  // Reduce max items to account for scroll indicator space
        }
        
        for (size_t i = startIdx; i < endIdx; ++i) {
            size_t actualIndex = getActualItemIndex(i);
            const auto& item = items[actualIndex];
            
            std::string line;
            
            // Add selection indicator
            if (i == selectedItem) {
                line = "> ";
            } else {
                line = "  ";
            }
            
            if (verboseMode) {
                // Verbose mode: timestamp | lines | content
                auto time_t = std::chrono::system_clock::to_time_t(item.timestamp);
                auto tm = *std::localtime(&time_t);
                
                std::ostringstream timeStream;
                timeStream << std::put_time(&tm, "%H:%M:%S");
                
                // Count lines in content
                size_t lineCount = 1;
                for (char c : item.content) {
                    if (c == '\n') lineCount++;
                }
                
                line += timeStream.str() + " | " + std::to_string(lineCount) + " lines | ";
                
                // Truncate content if too long
                std::string content = item.content;
                int maxContentLength = calculateMaxContentLength(true);
                if (content.length() > maxContentLength) {
                    content = smartTrim(content, maxContentLength);
                }
                
                // Replace newlines with spaces for display
                for (char& c : content) {
                    if (c == '\n' || c == '\r') c = ' ';
                }
                
                line += content;
            } else {
                // Normal mode: content (line count if > 1)
                
                // Count lines in content
                size_t lineCount = 1;
                for (char c : item.content) {
                    if (c == '\n') lineCount++;
                }
                
                // Truncate content if too long
                std::string content = item.content;
                int maxContentLength = calculateMaxContentLength(false);
                if (content.length() > maxContentLength) {
                    content = smartTrim(content, maxContentLength);
                }
                
                // Replace newlines with spaces for display
                for (char& c : content) {
                    if (c == '\n' || c == '\r') c = ' ';
                }
                
                line += content;
                
                // Add line count if more than one line
                if (lineCount > 1) {
                    line += " (" + std::to_string(lineCount) + " lines)";
                }
            }
            
            // Highlight selected item with theme selection color
            if (i == selectedItem) {
                XSetForeground(display, gc, selectionColor);
                XFillRectangle(display, window, gc, 5, y - 12, getClipListWidth(), 15);
                XSetForeground(display, gc, textColor);
            } else {
                XSetForeground(display, gc, textColor);
            }
            
            XDrawString(display, window, gc, 10, y, line.c_str(), line.length());
            
            y += LINE_HEIGHT;
        }
        
        if (displayCount == 0 && !cmd_themeSelectMode) {
            std::string empty;
            if (filterMode) {
                empty = "No matching items...";
            } else if (commandMode) {
                empty = "Enter command...";
            } else {
                empty = "No clipboard items yet...";
            }
            XDrawString(display, window, gc, 10, y, empty.c_str(), empty.length());
        }
        
        // Draw dialogs if visible
        drawBookmarkDialog();
        drawAddToBookmarkDialog();
        drawViewBookmarksDialog();
        drawPinnedDialog();
        drawHelpDialog();
#endif

#ifdef _WIN32
        // Windows drawing
        HDC hdc = GetDC(hwnd);
        if (!hdc) {
            return;
        }

        // Extract theme colors
        BYTE bg_r = (backgroundColor >> 16) & 0xFF;
        BYTE bg_g = (backgroundColor >> 8) & 0xFF;
        BYTE bg_b = backgroundColor & 0xFF;

        BYTE text_r = (textColor >> 16) & 0xFF;
        BYTE text_g = (textColor >> 8) & 0xFF;
        BYTE text_b = textColor & 0xFF;

        BYTE sel_r = (selectionColor >> 16) & 0xFF;
        BYTE sel_g = (selectionColor >> 8) & 0xFF;
        BYTE sel_b = selectionColor & 0xFF;

        // Clear background
        RECT clientRect;
        GetClientRect(hwnd, &clientRect);
        HBRUSH bgBrush = CreateSolidBrush(RGB(bg_r, bg_g, bg_b));
        FillRect(hdc, &clientRect, bgBrush);
        DeleteObject(bgBrush);

        // Set text properties
        SetTextColor(hdc, RGB(text_r, text_g, text_b));
        SetBkMode(hdc, TRANSPARENT);

        // --- Scrolling and item list logic (similar to Linux) ---
        int startY = 20;
        size_t displayCount = getDisplayItemCount();
        const int SCROLL_INDICATOR_HEIGHT = 15;
        int availableHeight = windowHeight - startY - 10;

        if (displayCount > (size_t)(availableHeight / LINE_HEIGHT)) {
            availableHeight -= SCROLL_INDICATOR_HEIGHT;
        }

        int maxItems = availableHeight / LINE_HEIGHT;
        if (maxItems > 0) maxItems += 1;
        if (maxItems < 1) maxItems = 1;

        size_t startIdx = consoleScrollOffset;
        size_t endIdx = std::min(startIdx + (size_t)maxItems, displayCount);

        if (displayCount > (size_t)maxItems) {
            std::string scrollText = "[" + std::to_string(selectedItem + 1) + "/" + std::to_string(displayCount) + "]";
            RECT scrollRect;
            GetClientRect(hwnd, &scrollRect);
            scrollRect.left = scrollRect.right - 100;
            scrollRect.top = 5;
            DrawTextA(hdc, scrollText.c_str(), -1, &scrollRect, DT_RIGHT | DT_TOP | DT_SINGLELINE);
        }

        int y = startY;

        if (displayCount == 0) {
            TextOutA(hdc, 10, y, "No clipboard items yet...", 25);
        } else {
            for (size_t i = startIdx; i < endIdx; ++i) {
                size_t actualIndex = getActualItemIndex(i);
                const auto& item = items[actualIndex];

                std::string line;
                if (i == selectedItem) {
                    line = "> ";
                } else {
                    line = "  ";
                }

                size_t lineCount = 1 + std::count(item.content.begin(), item.content.end(), '\n');
                if (item.content.empty()) lineCount = 0;

                std::string content = item.content;
                int maxContentLength = calculateMaxContentLength(false);
                if (content.length() > (size_t)maxContentLength) {
                    content = smartTrim(content, maxContentLength);
                }

                for (char& c : content) {
                    if (c == '\n' || c == '\r') c = ' ';
                }

                line += content;

                if (lineCount > 1) {
                    line += " (" + std::to_string(lineCount) + " lines)";
                }

                if (i == selectedItem) {
                    RECT selRect = { 5, y, clientRect.right - 5, y + LINE_HEIGHT };
                    HBRUSH selBrush = CreateSolidBrush(RGB(sel_r, sel_g, sel_b));
                    FillRect(hdc, &selRect, selBrush);
                    DeleteObject(selBrush);
                }

                int textY = y + (LINE_HEIGHT - 15) / 2;
                TextOutA(hdc, 10, textY, line.c_str(), line.length());
                y += LINE_HEIGHT;
            }
        }

        ReleaseDC(hwnd, hdc);
#endif

#ifdef __APPLE__
        // macOS drawing
#endif
    }
    

    
    void requestClipboardContent() {
#ifdef __linux__
        // Request clipboard content as UTF8_STRING
        XConvertSelection(display, clipboardAtom, utf8Atom, clipboardAtom, window, CurrentTime);
#endif
    }

#ifdef __linux__
    void handleSelectionNotify(XEvent* event) {
        if (event->xselection.property == None) {
            // If UTF8_STRING is not available, try plain TEXT
            XConvertSelection(display, clipboardAtom, XA_STRING, clipboardAtom, window, CurrentTime);
            return;
        }

        Atom target = event->xselection.target;
        if (target != utf8Atom && target != XA_STRING) {
            // We are not interested in other formats
            return;
        }

        Atom type;
        int format;
        unsigned long nitems, bytes_after;
        unsigned char* data = nullptr;

        XGetWindowProperty(display, window, clipboardAtom, 0, LONG_MAX, False, AnyPropertyType,
                           &type, &format, &nitems, &bytes_after, &data);

        if (data) {
            std::string content(reinterpret_cast<char*>(data), nitems);
            XFree(data);
            processClipboardContent(content);
        }
    }
#endif

    void processClipboardContent(const std::string& content) {
        // Trim trailing newlines
        std::string trimmed_content = content;
        while (!trimmed_content.empty() && (trimmed_content.back() == '\n' || trimmed_content.back() == '\r')) {
            trimmed_content.pop_back();
        }

        if (trimmed_content.empty() || trimmed_content == lastClipboardContent) {
            return;
        }

        lastClipboardContent = trimmed_content;

        // The rest of the logic from checkClipboard
        size_t duplicateIndex = 0;
        
        // Check for duplicates and move to top if found
        bool isDuplicate = false;
        for (size_t i = 0; i < items.size(); i++) {
            if (items[i].content == trimmed_content) {
                isDuplicate = true;
                duplicateIndex = i;
                break;
            }
        }
      
        if (isDuplicate) {
            // Move existing clip to top
            std::string clipContent = items[duplicateIndex].content;
            items.erase(items.begin() + duplicateIndex);
            items.emplace(items.begin(), clipContent);

            // Reset selection to top when item is moved
            selectedItem = 0;

            // Update filtered items if in filter mode
            if (filterMode) {
                updateFilteredItems();
            }

            saveToFile();

            std::cout << "Existing clip moved to top" << std::endl;

            // Refresh display if window is visible
            if (visible) {
                drawConsole();
            }

            return;
        }

        items.emplace(items.begin(), trimmed_content);
        if (items.size() > maxClips) {
            items.pop_back();
        }
        
        // Reset selection to top when new item is added
        selectedItem = 0;
        
        // Update filtered items if in filter mode
        if (filterMode) {
            updateFilteredItems();
        }
        
        saveToFile();
        
        std::cout << "New clipboard item added" << std::endl;
        
        // Refresh display if window is visible
        if (visible) {
            drawConsole();
        }
    }
    
    void loadConfig() {
        // Cross-platform path separator
#ifdef _WIN32
        const char pathSep = '\\';
#else
        const char pathSep = '/';
#endif
        
        std::string configFile = configDir + pathSep + "config.json";
        std::ifstream file(configFile);
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                // Parse verbose
                if (line.find("\"verbose\"") != std::string::npos) {
                    verboseMode = line.find("true") != std::string::npos;
                }
                // Parse max_clips
                else if (line.find("\"max_clips\"") != std::string::npos) {
                    size_t colon = line.find(':');
                    if (colon != std::string::npos) {
                        std::string value = line.substr(colon + 1);
                        // Remove whitespace and commas
                        value.erase(0, value.find_first_not_of(" \t"));
                        value.erase(value.find_last_not_of(" \t,") + 1);
                        maxClips = std::stoull(value);
                    }
                }
                // Parse encrypted
                else if (line.find("\"encrypted\"") != std::string::npos) {
                    encrypted = line.find("true") != std::string::npos;
                }
                // Parse autostart
                else if (line.find("\"autostart\"") != std::string::npos) {
                    autoStart = line.find("true") != std::string::npos;
                }
                // Parse encryption_key
                else if (line.find("\"encryption_key\"") != std::string::npos) {
                    size_t start = line.find('"', line.find(':'));
                    size_t end = line.find('"', start + 1);
                    if (start != std::string::npos && end != std::string::npos) {
                        encryptionKey = line.substr(start + 1, end - start - 1);
                    }
                }
                // Parse theme
                else if (line.find("\"theme\"") != std::string::npos) {
                    size_t start = line.find('"', line.find(':'));
                    size_t end = line.find('"', start + 1);
                    if (start != std::string::npos && end != std::string::npos) {
                        theme = line.substr(start + 1, end - start - 1);
                    }
                }
            }
            file.close();
        } else {
            // Create default config with all settings
            createDefaultConfig();
        }
    }
    
    void saveConfig() {
        std::string configFile = configDir + "/config.json";
        std::ofstream outFile(configFile);
        outFile << "{\n";
        outFile << "    \"verbose\": " << (verboseMode ? "true" : "false") << ",\n";
        outFile << "    \"max_clips\": " << maxClips << ",\n";
        outFile << "    \"encrypted\": " << (encrypted ? "true" : "false") << ",\n";
        outFile << "    \"encryption_key\": \"" << encryptionKey << "\",\n";
        outFile << "    \"autostart\": " << (autoStart ? "true" : "false") << ",\n";
        outFile << "    \"theme\": \"" << theme << "\"\n";
        outFile << "}\n";
        outFile.close();
    }
    
    void createDefaultConfig() {
        std::string configFile = configDir + "/config.json";
        std::ofstream outFile(configFile);
        outFile << "{\n";
        outFile << "    \"verbose\": false,\n";
        outFile << "    \"max_clips\": 500,\n";
        outFile << "    \"encrypted\": true,\n";
        outFile << "    \"encryption_key\": \"mmry_default_key_2024\",\n";
        outFile << "    \"autostart\": false,\n";
        outFile << "    \"theme\": \"console\"\n";
        outFile << "}\n";
        outFile.close();
        
        std::cout << "Created default config at: " << configFile << std::endl;
    }
    
    int countLines(const std::string& content) {
        if (content.empty()) return 0;
        int lines = 1;
        for (char c : content) {
            if (c == '\n') lines++;
        }
        return lines;
    }
    
    void copyToClipboard(const std::string& content) {
#ifdef __linux__
        // Use xclip to copy to clipboard
        FILE* pipe = popen("xclip -selection clipboard", "w");
        if (pipe) {
            fwrite(content.c_str(), 1, content.length(), pipe);
            pclose(pipe);
        }
#endif

#ifdef _WIN32
        // Windows clipboard
        if (OpenClipboard(nullptr)) {
            EmptyClipboard();
            HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, content.length() + 1);
            if (hMem) {
                memcpy(GlobalLock(hMem), content.c_str(), content.length() + 1);
                GlobalUnlock(hMem);
                SetClipboardData(CF_TEXT, hMem);
            }
            CloseClipboard();
        }
#endif

#ifdef __APPLE__
        // macOS clipboard using pbcopy
        FILE* pipe = popen("pbcopy", "w");
        if (pipe) {
            fwrite(content.c_str(), 1, content.length(), pipe);
            pclose(pipe);
        }
#endif
    }
    
    void saveToFile() {
        std::ofstream file(dataFile);
        if (file.is_open()) {
            for (const auto& item : items) {
                // Store timestamp and content (encrypted if enabled)
                auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                    item.timestamp.time_since_epoch()).count();
                std::string contentToSave = encrypt(item.content);
                file << timestamp << "|" << contentToSave << "\n";
            }
            file.close();
        }
    }
    
    void loadFromFile() {
        std::ifstream file(dataFile);
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                size_t pos = line.find('|');
                if (pos != std::string::npos && pos > 0) {
                    std::string timestampStr = line.substr(0, pos);
                    std::string content = line.substr(pos + 1);
                    
                    try {
                        std::string decryptedContent;
                        
                        // Try to decrypt first
                        try {
                            decryptedContent = decrypt(content);
                            // Check if decryption produced reasonable results (no control characters)
                            bool hasControlChars = false;
                            for (char c : decryptedContent) {
                                if (c < 32 && c != '\n' && c != '\r' && c != '\t') {
                                    hasControlChars = true;
                                    break;
                                }
                            }
                            
                            // If decryption produced garbage, assume the content was never encrypted
                            if (hasControlChars || decryptedContent.empty()) {
                                decryptedContent = content;
                            }
                        } catch (...) {
                            // If decryption fails, assume content was never encrypted
                            decryptedContent = content;
                        }
                        
                        ClipboardItem item(decryptedContent);
                        auto timestamp = std::chrono::seconds(std::stoll(timestampStr));
                        item.timestamp = std::chrono::system_clock::time_point(timestamp);
                        
                        items.push_back(item);
                    } catch (const std::exception& e) {
                        // Skip invalid entries
                        continue;
                    }
                }
            }
            file.close();
        } else {
            // Create empty clips.txt file if it doesn't exist
            std::ofstream outFile(dataFile);
            if (outFile.is_open()) {
                outFile.close();
                std::cout << "Created empty clips file: " << dataFile << std::endl;
            } else {
                std::cerr << "Failed to create clips file: " << dataFile << std::endl;
            }
        }
    }
};


// Global pointer for signal handling
ClipboardManager* g_manager = nullptr;

#include <signal.h>
void signal_handler(int signal) {
    std::cout << "\nReceived signal " << signal << ", cleaning up..." << std::endl;
    if (g_manager) {
        // Just set running to false, don't join in signal handler
        g_manager->setRunning(false);
    }
    exit(0);
}


#ifdef _WIN32
LRESULT CALLBACK MMRYWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    // Get the ClipboardManager instance
    ClipboardManager* manager = nullptr;
    
    if (msg == WM_NCCREATE) {
        // Store instance pointer
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
        manager = (ClipboardManager*)pCreate->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)manager);
    } else {
        manager = (ClipboardManager*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }
    
    switch (msg) {
        case WM_KEYDOWN:
        case WM_CHAR:
            if (manager) {
                MSG winMsg = {hwnd, msg, wParam, lParam};
                manager->handleKeyPressCommon(&winMsg);
            }
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}
#endif


int main() {
#ifdef __linux__
    // Install signal handlers for graceful shutdown
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    SingleInstance guard("Mmry");
    if (guard.isAnotherInstanceRunning()) {
        std::cerr << "Another instance is already running. Exiting." << std::endl;
        return 1;
    }
    
    // Install temporary error handler
    XErrorHandler oldHandler = XSetErrorHandler(ignore_x11_errors);
    (void)oldHandler; // Suppress unused variable warning

    ClipboardManager manager;
    g_manager = &manager;
    manager.run();
    g_manager = nullptr;
#endif

#ifdef _WIN32
    // Check for another instance using a named mutex
    HANDLE hMutex = CreateMutexA(NULL, TRUE, "Global\\MmryClipboardManager");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        std::cerr << "Another instance is already running. Exiting." << std::endl;
        if (hMutex) {
            CloseHandle(hMutex);
        }
        MessageBoxA(NULL, "MMRY is already running.", "MMRY", MB_OK | MB_ICONINFORMATION);
        return 1;
    }

    // Install signal handlers for graceful shutdown
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    ClipboardManager manager;
    g_manager = &manager;
    manager.run();
    g_manager = nullptr;

    // Clean up mutex
    if (hMutex) {
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
    }
#endif

    return 0;
}
