#include <iostream>
#include <unistd.h>
#include <thread>
#include <atomic>
#include <string>
#include <vector>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <sys/stat.h>


#ifdef __linux__
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#endif

struct ClipboardItem {
    std::string content;
    std::chrono::system_clock::time_point timestamp;
    
    ClipboardItem(const std::string& content) 
        : content(content), timestamp(std::chrono::system_clock::now()) {}
};

class ClipboardManager {
private:
    std::atomic<bool> hotkeyGrabbed{false};
    std::thread hotkeyMonitorThread;

public:
    void handleKeyPress(XEvent* event) {
#ifdef __linux__
        KeySym keysym;
        char buffer[10];
        XKeyEvent* keyEvent = (XKeyEvent*)event;
        
        XLookupString(keyEvent, buffer, sizeof(buffer), &keysym, nullptr);

        if (keysym == XK_Q && (keyEvent->state & ShiftMask)) {
            // Shift+Q quits application even from dialog
            std::cout << "Quitting MMRY..." << std::endl;
            stop();
        }
        
        // Check if this is from the root window (global hotkey)
        if (event->xany.window == root) {
            // More robust hotkey detection
            if (keysym == XK_c || keysym == XK_C) {
                unsigned int state = keyEvent->state;
                
                // Mask out NumLock (Mod2Mask) and CapsLock (LockMask) for comparison
                state &= ~(Mod2Mask | LockMask);
                
                // Check if Ctrl+Alt are pressed (ignore other modifiers)
                if ((state & ControlMask) && (state & Mod1Mask)) {
                    std::cout << "Hotkey triggered: Ctrl+Alt+C (state: 0x" 
                             << std::hex << keyEvent->state << std::dec << ")" << std::endl;
                    showWindow();
                    return;
                }
            }
        }
        
        if (keysym == XK_Escape) {
            if (bookmarkDialogVisible) {
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
            } else {
                // Normal escape behavior - hide window
                hideWindow();
            }

            return;
        }

        if (helpDialogVisible) {
            if (keysym == XK_Escape || keysym == XK_question) {
                // Escape or '?' closes help dialog
                helpDialogVisible = false;
                drawConsole();
            }

            return;
        }

        // ---------------------------------------------------------------------


        // Adding bookmark groups
        //
        if (bookmarkDialogVisible && !addToBookmarkDialogVisible) {
            if (keysym == XK_Return) {
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

            if (keysym == XK_Down) {
                // j/Down arrow navigates through bookmark groups only when input is empty
                std::vector<std::string> filteredGroups;
                for (const auto& group : bookmarkGroups) {
                    if (bookmarkDialogInput.empty() || group.find(bookmarkDialogInput) != std::string::npos) {
                        filteredGroups.push_back(group);
                    }
                }
                
                if (!filteredGroups.empty()) {
                    selectedBookmarkGroup = (selectedBookmarkGroup + 1) % filteredGroups.size();
                    updateBookmarkMgmtScrollOffset();
                    drawConsole();
                }
                return;
            }

            if (keysym == XK_Up) {
                // k/Up arrow navigates backwards through bookmark groups only when input is empty
                std::vector<std::string> filteredGroups;
                for (const auto& group : bookmarkGroups) {
                    if (bookmarkDialogInput.empty() || group.find(bookmarkDialogInput) != std::string::npos) {
                        filteredGroups.push_back(group);
                    }
                }
                
                if (!filteredGroups.empty()) {
                    selectedBookmarkGroup = (selectedBookmarkGroup == 0) ? filteredGroups.size() - 1 : selectedBookmarkGroup - 1;
                    updateBookmarkMgmtScrollOffset();
                    drawConsole();
                }
                return;
            }
            
            if (keysym == XK_BackSpace) {
                // Backspace in input field
                if (!bookmarkDialogInput.empty()) {
                    bookmarkDialogInput.pop_back();
                    drawConsole();
                }
                return;
            }

            // Text input for bookmark dialog - exclude vim navigation keys
            char buffer[10];
            int count = XLookupString(keyEvent, buffer, sizeof(buffer), nullptr, nullptr);
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
            if (keysym == XK_Escape || keysym == XK_grave) {
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
                if (keysym == XK_j || keysym == XK_Down) {
                    // Move down in groups
                    if (selectedViewBookmarkGroup < bookmarkGroups.size() - 1) {
                        selectedViewBookmarkGroup++;
                        updateScrollOffset();
                        drawConsole();
                    }
                    return;
                }

                if (keysym == XK_k || keysym == XK_Up) {
                    // Move up in groups
                    if (selectedViewBookmarkGroup > 0) {
                        selectedViewBookmarkGroup--;
                        updateScrollOffset();
                        drawConsole();
                    }
                    return;
                }

                if (keysym == XK_g) {
                    // Go to top
                    selectedViewBookmarkGroup = 0;
                    viewBookmarksScrollOffset = 0;
                    drawConsole();
                    return;
                }

                if (keysym == XK_G && (keyEvent->state & ShiftMask)) {
                    // Go to bottom
                    selectedViewBookmarkGroup = bookmarkGroups.size() - 1;
                    updateScrollOffset();
                    drawConsole();
                    return;
                }

                if (keysym == XK_D && (keyEvent->state & ShiftMask)) {
                    // Shift+D deletes selected group and its clips
                    if (selectedViewBookmarkGroup < bookmarkGroups.size()) {
                        std::string groupToDelete = bookmarkGroups[selectedViewBookmarkGroup];
                        
                        // Remove group from list
                        bookmarkGroups.erase(bookmarkGroups.begin() + selectedViewBookmarkGroup);
                        saveBookmarkGroups();
                        
                        // Delete bookmark file
                        std::string bookmarkFile = configDir + "/bookmarks_" + groupToDelete + ".txt";
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

                if (keysym == XK_Return) {
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
                if (keysym == XK_j || keysym == XK_Down) {
                    // Move down in bookmark items
                    selectedViewBookmarkItem++;
                    updateScrollOffset();
                    drawConsole();
                    return;
                }

                if (keysym == XK_k || keysym == XK_Up) {
                    // Move up in bookmark items
                    if (selectedViewBookmarkItem > 0) {
                        selectedViewBookmarkItem--;
                        updateScrollOffset();
                        drawConsole();
                    }
                    return;
                }

                if (keysym == XK_g) {
                    // Go to top
                    selectedViewBookmarkItem = 0;
                    viewBookmarksScrollOffset = 0;
                    drawConsole();
                    return;
                }

                if (keysym == XK_G && (keyEvent->state & ShiftMask)) {
                    // Go to bottom
                    if (selectedViewBookmarkGroup < bookmarkGroups.size()) {
                        std::string selectedGroup = bookmarkGroups[selectedViewBookmarkGroup];
                        std::string bookmarkFile = configDir + "/bookmarks_" + selectedGroup + ".txt";
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

                if (keysym == XK_D && (keyEvent->state & ShiftMask)) {
                    // Shift+D deletes selected clip
                    if (selectedViewBookmarkGroup < bookmarkGroups.size()) {
                        std::string selectedGroup = bookmarkGroups[selectedViewBookmarkGroup];
                        std::string bookmarkFile = configDir + "/bookmarks_" + selectedGroup + ".txt";
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

                if (keysym == XK_Return) {
                    // Copy selected bookmark item to clipboard
                    if (selectedViewBookmarkGroup < bookmarkGroups.size()) {
                        std::string selectedGroup = bookmarkGroups[selectedViewBookmarkGroup];
                        std::string bookmarkFile = configDir + "/bookmarks_" + selectedGroup + ".txt";
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
            }
            return;
        }


        // Adding the current clip to a bookmark group
        //
        if (addToBookmarkDialogVisible) {
            // Add to bookmark dialog is visible - handle dialog-specific keys
            if (keysym == XK_Escape) {
                // Escape closes dialog but not window
                addToBookmarkDialogVisible = false;
                drawConsole();
                return;
            }

            if (keysym == XK_Return) {
                // std::cout << "TEST: hit return" << std::endl;
                // Enter adds clip to selected bookmark group
                if (!bookmarkGroups.empty() && selectedAddBookmarkGroup < bookmarkGroups.size()) {
                    std::string selectedGroup = bookmarkGroups[selectedAddBookmarkGroup];
                    
                    // Check if current clip is already in this group
                    if (!items.empty() && selectedItem < getDisplayItemCount()) {
                        size_t actualIndex = getActualItemIndex(selectedItem);
                        std::string clipContent = items[actualIndex].content;
                        
                        // Read existing bookmarks in this group
                        std::string bookmarkFile = configDir + "/bookmarks_" + selectedGroup + ".txt";
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

            if (keysym == XK_j || keysym == XK_Down) {
                // Move down in bookmark groups
                if (!bookmarkGroups.empty() && selectedAddBookmarkGroup < bookmarkGroups.size() - 1) {
                    selectedAddBookmarkGroup++;
                    updateAddBookmarkScrollOffset();
                    drawConsole();
                }
                return;
            }

            if (keysym == XK_k || keysym == XK_Up) {
                // Move up in bookmark groups
                if (selectedAddBookmarkGroup > 0) {
                    selectedAddBookmarkGroup--;
                    updateAddBookmarkScrollOffset();
                    drawConsole();
                }
                return;
            }

            if (keysym == XK_g) {
                // Go to top
                selectedAddBookmarkGroup = 0;
                addBookmarkScrollOffset = 0;
                drawConsole();
                return;
            }

            if (keysym == XK_G && (keyEvent->state & ShiftMask)) {
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
            if (keysym == XK_BackSpace) {
                // Remove last character from filter
                if (!filterText.empty()) {
                    filterText.pop_back();
                    updateFilteredItems();
                    selectedItem = 0;
                    drawConsole();
                }
                return;
            }

            if (keysym == XK_Return) {
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

            if (keysym == XK_Down) {
                // Move down
                size_t displayCount = getDisplayItemCount();
                if (selectedItem < displayCount - 1) {
                    selectedItem++;
                    updateConsoleScrollOffset();
                    drawConsole();
                }
                return;
            }

            if (keysym == XK_Up) {
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
            int count = XLookupString(keyEvent, buffer, sizeof(buffer), nullptr, nullptr);
            filterText += std::string(buffer, count);
            updateFilteredItems();
            selectedItem = 0;
            drawConsole();

            return;
        }



        // General keys - main clips list
        //
        if (keysym == XK_j || keysym == XK_Down) {
            // Move down
            size_t displayCount = getDisplayItemCount();
            if (selectedItem < displayCount - 1) {
                selectedItem++;
                updateConsoleScrollOffset();
                drawConsole();
            }
            return;
        }

        if (keysym == XK_k || keysym == XK_Up) {
            // Move up
            if (selectedItem > 0) {
                selectedItem--;
                updateConsoleScrollOffset();
                drawConsole();
            }
            return;
        }

        if (keysym == XK_g) {
            // Go to top
            selectedItem = 0;
            updateConsoleScrollOffset();
            drawConsole();
            return;
        }

        if (keysym == XK_G) {
            // Go to bottom
            size_t displayCount = getDisplayItemCount();
            if (displayCount > 0) {
                selectedItem = displayCount - 1;
                updateConsoleScrollOffset();
                drawConsole();
            }
            return;
        }

        if (keysym == XK_D && (keyEvent->state & ShiftMask)) {
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

        if (keysym == XK_slash) {
            // Enter filter mode
            filterMode = true;
            filterText = "";
            updateFilteredItems();
            selectedItem = 0;
            drawConsole();
            return;
        }

        if (keysym == XK_Return) {
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
                hideWindow();
            }
            return;
        }

        if (keysym == XK_M && (keyEvent->state & ShiftMask)) {
            // Shift+M to show bookmark dialog
            bookmarkDialogVisible = true;
            bookmarkDialogInput = "";
            selectedBookmarkGroup = 0;
            bookmarkMgmtScrollOffset = 0; // Reset scroll when opening
            drawConsole();
            return;
        }

        if (keysym == XK_m) {
            // Lowercase m to show add-to-bookmark dialog
            if (!bookmarkGroups.empty()) {
                addToBookmarkDialogVisible = true;
                selectedAddBookmarkGroup = 0;
                addBookmarkScrollOffset = 0; // Reset scroll when opening
                drawConsole();
            }
            return;
        }

        if (keysym == XK_question) {
            // '?' to show help dialog
            helpDialogVisible = true;
            drawConsole();
            return;
        }

        if (keysym == XK_grave) {
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
#endif
    }
    
    void run() {
        running = true;
        
        // Initialize X11
#ifdef __linux__
        display = XOpenDisplay(nullptr);
        if (!display) {
            std::cerr << "Cannot open display" << std::endl;
            return;
        }
        screen = DefaultScreen(display);
        root = RootWindow(display, screen);
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
        
        // Start clipboard monitoring after everything is set up
        startClipboardMonitoring();
        
        std::cout << "MMRY Clipboard Manager started" << std::endl;
        std::cout << "Config directory: " << configDir << std::endl;
        std::cout << "Press Ctrl+Alt+C to show window, Escape to hide" << std::endl;
        std::cout << "Press Shift+Q in window to quit application" << std::endl;
        std::cout << "Press Ctrl+C in terminal to exit" << std::endl;
        
#ifdef __linux__
        XEvent event;
        while (running) {
            XNextEvent(display, &event);
            
            // Check if event is from root window (hotkey) or application window
            if (event.xany.window == root) {
                // Hotkey event from root window
                if (event.type == KeyPress) {
                    KeySym keysym = XLookupKeysym(&event.xkey, 0);
                    // Check for Ctrl+Alt+C more precisely
                    if (keysym == XK_c && 
                        (event.xkey.state & ControlMask) && 
                        (event.xkey.state & Mod1Mask)) {
                        // Ctrl+Alt+C pressed - show window
                        std::cout << "Hotkey triggered: Ctrl+Alt+C" << std::endl;
                        showWindow();
                    }
                }
            } else {
                // Event from application window
                switch (event.type) {
                    case Expose:
                        drawConsole();
                        break;
                    case KeyPress:
                        handleKeyPress(&event);
                        break;
                }
            }
        }
#endif

#ifdef _WIN32
        // Windows event loop
        MSG msg;
        while (running && GetMessage(&msg, nullptr, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
#endif

#ifdef __APPLE__
        // macOS event loop
        // This would require NSApplication setup
#endif
    }
    
    void stop() {
        running = false;
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
#endif

#ifdef _WIN32
    HWND hwnd;
    HFONT font;
#endif

    std::atomic<bool> running;
    std::atomic<bool> visible;
    
    // Window properties
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;
    const int WINDOW_X = 100;
    const int WINDOW_Y = 100;
    
    // Clipboard data
    std::vector<ClipboardItem> items;
    std::string lastClipboardContent;
    bool verboseMode;
    std::string configDir;
    std::string dataFile;
    size_t maxClips = 500;
    bool encrypted = false;
    std::string encryptionKey = "";
    std::string theme = "console";
    
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
    
    // Bookmark dialog
    bool bookmarkDialogVisible = false;
    std::string bookmarkDialogInput;
    std::vector<std::string> bookmarkGroups;
    size_t selectedBookmarkGroup = 0;
    size_t bookmarkMgmtScrollOffset = 0; // For scrolling long lists
    
    // Add to bookmark dialog state
    bool addToBookmarkDialogVisible = false;
    size_t selectedAddBookmarkGroup = 0;
    size_t addBookmarkScrollOffset = 0; // For scrolling long lists
    
    // Help dialog state
    bool helpDialogVisible = false;
    
    // View bookmarks dialog state
    bool viewBookmarksDialogVisible = false;
    bool viewBookmarksShowingGroups = true; // true = groups, false = clips
    size_t selectedViewBookmarkGroup = 0;
    size_t selectedViewBookmarkItem = 0;
    size_t viewBookmarksScrollOffset = 0; // For scrolling long lists
    
    // Helper methods
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
        const int VISIBLE_ITEMS = (WINDOW_HEIGHT - 60) / 15; // Approximate lines that fit
        
        if (selectedItem < consoleScrollOffset) {
            consoleScrollOffset = selectedItem;
        } else if (selectedItem >= consoleScrollOffset + VISIBLE_ITEMS) {
            consoleScrollOffset = selectedItem - VISIBLE_ITEMS + 1;
        }
    }
    
    void updateBookmarkMgmtScrollOffset() {
        const int VISIBLE_ITEMS = 8; // Number of groups visible in bookmark management dialog
        
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
    
    void updateAddBookmarkScrollOffset() {
        const int VISIBLE_ITEMS = 8; // Number of groups visible in add bookmark dialog
        
        if (selectedAddBookmarkGroup < addBookmarkScrollOffset) {
            addBookmarkScrollOffset = selectedAddBookmarkGroup;
        } else if (selectedAddBookmarkGroup >= addBookmarkScrollOffset + VISIBLE_ITEMS) {
            addBookmarkScrollOffset = selectedAddBookmarkGroup - VISIBLE_ITEMS + 1;
        }
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
        std::string themeFile = configDir + "/themes/" + theme + ".json";
        std::ifstream file(themeFile);
        
        // Set default colors (console theme)
        backgroundColor = 0x000000; // Black
        textColor = 0xFFFFFF;      // White
        selectionColor = 0x404040;  // Dim gray
        borderColor = 0xFFFFFF;    // White
        
        if (!file.is_open()) {
            // Try to create default theme file
            createDefaultThemeFile();
            return;
        }
        
        std::string line;
        std::string currentSection;
        while (std::getline(file, line)) {
            // Remove whitespace
            line.erase(0, line.find_first_not_of(" \t"));
            line.erase(line.find_last_not_of(" \t") + 1);
            
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
        file.close();
    }
    
    void createDefaultThemeFile() {
        // Create themes directory if it doesn't exist
        std::string themesDir = configDir + "/themes";
        struct stat st = {0};
        if (stat(themesDir.c_str(), &st) == -1) {
            mkdir(themesDir.c_str(), 0755);
        }
        
        // Create default console theme file
        std::string themeFile = themesDir + "/console.json";
        std::ofstream outFile(themeFile);
        if (outFile.is_open()) {
            outFile << "{\n";
            outFile << "    \"name\": \"Console\",\n";
            outFile << "    \"description\": \"Default console theme with black background and white text\",\n";
            outFile << "    \"colors\": {\n";
            outFile << "        \"background\": \"#000000\",\n";
            outFile << "        \"text\": \"#FFFFFF\",\n";
            outFile << "        \"selection\": \"#404040\",\n";
            outFile << "        \"border\": \"#FFFFFF\"\n";
            outFile << "    }\n";
            outFile << "}\n";
            outFile.close();
        }
    }
    
    void loadBookmarkGroups() {
        std::string bookmarkFile = configDir + "/bookmarks.txt";
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
        std::string bookmarkFile = configDir + "/bookmarks.txt";
        std::ofstream file(bookmarkFile);
        
        if (file.is_open()) {
            for (const auto& group : bookmarkGroups) {
                file << group << "|0\n"; // Group name | clip count
            }
            file.close();
        }
    }
    
    void addClipToBookmarkGroup(const std::string& groupName, const std::string& content) {
        std::string bookmarkFile = configDir + "/bookmarks_" + groupName + ".txt";
        std::ofstream file(bookmarkFile, std::ios::app);
        
        if (file.is_open()) {
            auto timestamp = std::chrono::system_clock::now().time_since_epoch().count();
            std::string contentToSave = encrypt(content);
            file << timestamp << "|" << contentToSave << "\n";
            file.close();
        }
    }
    
    void setupConfigDir() {
        const char* home = getenv("HOME");
        if (!home) home = getenv("USERPROFILE");
        if (!home) home = ".";
        
        configDir = std::string(home) + "/.config/mmry";
        
        // Create config directory if it doesn't exist
        struct stat st = {0};
        if (stat(configDir.c_str(), &st) == -1) {
            mkdir(configDir.c_str(), 0755);
            std::cout << "Created config directory: " << configDir << std::endl;
        }
        
        // Create themes directory if it doesn't exist
        std::string themesDir = configDir + "/themes";
        if (stat(themesDir.c_str(), &st) == -1) {
            mkdir(themesDir.c_str(), 0755);
            std::cout << "Created themes directory: " << themesDir << std::endl;
        }
        
        dataFile = configDir + "/clips.txt";
        
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
        std::string bookmarksFile = configDir + "/bookmarks.txt";
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
    }
    
    void createWindow() {
#ifdef __linux__
        // Create window with theme colors
        window = XCreateSimpleWindow(display, root, 
                                   WINDOW_X, WINDOW_Y, 
                                   WINDOW_WIDTH, WINDOW_HEIGHT,
                                   2, borderColor, backgroundColor);
        
        // Set window properties
        XStoreName(display, window, "MMRY");
        XSelectInput(display, window, ExposureMask | KeyPressMask);
        
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
        XWMHints hints;
        hints.flags = InputHint | StateHint;
        hints.input = True;
        hints.initial_state = NormalState;
        XSetWMHints(display, window, &hints);
        
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
        
        // Start a monitoring thread to periodically verify the hotkey is still grabbed
        startHotkeyMonitoring();
#endif
    }

    void startHotkeyMonitoring() {
#ifdef __linux__
        hotkeyMonitorThread = std::thread([this]() {
            while (running) {
                std::this_thread::sleep_for(std::chrono::seconds(5));
                
                if (!hotkeyGrabbed) {
                    continue;
                }
                
                // Periodically try to re-grab the hotkey in case it was lost
                // This is a safety mechanism for when other applications interfere
                KeyCode keycode = XKeysymToKeycode(display, XK_c);
                
                // Check if we still have the grab by attempting to grab again
                // If we already have it, this should fail gracefully
                XGrabKey(display, keycode, ControlMask | Mod1Mask, root, True,
                        GrabModeAsync, GrabModeAsync);
                XSync(display, False);
            }
        });
        hotkeyMonitorThread.detach();
#endif
    }
    
    void showWindow() {
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
            ShowWindow(hwnd, SW_HIDE);
#endif
            visible = false;
            std::cout << "Window hidden" << std::endl;
        }
    }
    
    void drawBookmarkDialog() {
#ifdef __linux__
        if (!bookmarkDialogVisible) return;
        
        // Dialog dimensions
        const int DIALOG_WIDTH = 400;
        const int DIALOG_HEIGHT = 300;
        const int DIALOG_X = (WINDOW_WIDTH - DIALOG_WIDTH) / 2;
        const int DIALOG_Y = (WINDOW_HEIGHT - DIALOG_HEIGHT) / 2;
        
        // Draw dialog background
        XSetForeground(display, gc, backgroundColor);
        XFillRectangle(display, window, gc, DIALOG_X, DIALOG_Y, DIALOG_WIDTH, DIALOG_HEIGHT);
        XSetForeground(display, gc, borderColor);
        XDrawRectangle(display, window, gc, DIALOG_X, DIALOG_Y, DIALOG_WIDTH, DIALOG_HEIGHT);
        
        // Draw title
        std::string title = "Bookmark Groups";
        int titleWidth = XTextWidth(font, title.c_str(), title.length());
        XDrawString(display, window, gc, DIALOG_X + (DIALOG_WIDTH - titleWidth) / 2, DIALOG_Y + 25, title.c_str(), title.length());
        
        // Draw input field
        XSetForeground(display, gc, textColor);
        XDrawString(display, window, gc, DIALOG_X + 20, DIALOG_Y + 60, "New group name:", 16);
        
        // Draw input box
        XSetForeground(display, gc, selectionColor);
        XFillRectangle(display, window, gc, DIALOG_X + 20, DIALOG_Y + 70, DIALOG_WIDTH - 40, 25);
        XSetForeground(display, gc, textColor);
        XDrawRectangle(display, window, gc, DIALOG_X + 20, DIALOG_Y + 70, DIALOG_WIDTH - 40, 25);
        
        // Draw input text
        std::string displayInput = bookmarkDialogInput + "_";
        XDrawString(display, window, gc, DIALOG_X + 25, DIALOG_Y + 87, displayInput.c_str(), displayInput.length());
        
        // Draw existing groups
        XSetForeground(display, gc, textColor);
        XDrawString(display, window, gc, DIALOG_X + 20, DIALOG_Y + 120, "Existing groups:", 15);
        
        // Filter and display groups
        std::vector<std::string> filteredGroups;
        for (const auto& group : bookmarkGroups) {
            if (bookmarkDialogInput.empty() || group.find(bookmarkDialogInput) != std::string::npos) {
                filteredGroups.push_back(group);
            }
        }
        
        int y = DIALOG_Y + 140;
        const int VISIBLE_ITEMS = 8;
        
        size_t startIdx = bookmarkMgmtScrollOffset;
        size_t endIdx = std::min(startIdx + VISIBLE_ITEMS, filteredGroups.size());
        
        for (size_t i = startIdx; i < endIdx; ++i) {
            std::string displayText = "  " + filteredGroups[i];
            if (i == selectedBookmarkGroup) {
                displayText = "> " + filteredGroups[i];
                // Highlight selected
                XSetForeground(display, gc, selectionColor);
                XFillRectangle(display, window, gc, DIALOG_X + 15, y - 12, DIALOG_WIDTH - 30, 15);
                XSetForeground(display, gc, textColor);
            }
            XDrawString(display, window, gc, DIALOG_X + 20, y, displayText.c_str(), displayText.length());
            y += 18;
        }
        
        // Show scroll indicator if needed
        if (filteredGroups.size() > VISIBLE_ITEMS) {
            std::string scrollText = "[" + std::to_string(selectedBookmarkGroup + 1) + "/" + std::to_string(filteredGroups.size()) + "]";
            XDrawString(display, window, gc, DIALOG_X + DIALOG_WIDTH - 60, DIALOG_Y + 40, scrollText.c_str(), scrollText.length());
        }
        

#endif
    }
    
    void drawAddToBookmarkDialog() {
#ifdef __linux__
        if (!addToBookmarkDialogVisible) return;
        
        // Dialog dimensions
        const int DIALOG_WIDTH = 400;
        const int DIALOG_HEIGHT = 300;
        const int DIALOG_X = (WINDOW_WIDTH - DIALOG_WIDTH) / 2;
        const int DIALOG_Y = (WINDOW_HEIGHT - DIALOG_HEIGHT) / 2;
        
        // Draw dialog background
        XSetForeground(display, gc, backgroundColor);
        XFillRectangle(display, window, gc, DIALOG_X, DIALOG_Y, DIALOG_WIDTH, DIALOG_HEIGHT);
        XSetForeground(display, gc, borderColor);
        XDrawRectangle(display, window, gc, DIALOG_X, DIALOG_Y, DIALOG_WIDTH, DIALOG_HEIGHT);
        
        // Draw title
        std::string title = "Add to Bookmark Group";
        int titleWidth = XTextWidth(font, title.c_str(), title.length());
        XDrawString(display, window, gc, DIALOG_X + (DIALOG_WIDTH - titleWidth) / 2, DIALOG_Y + 25, title.c_str(), title.length());
        
        // Draw bookmark groups
        XSetForeground(display, gc, textColor);
        XDrawString(display, window, gc, DIALOG_X + 20, DIALOG_Y + 60, "Select bookmark group:", 20);
        
        int y = DIALOG_Y + 80;
        const int VISIBLE_ITEMS = 8;
        
        size_t startIdx = addBookmarkScrollOffset;
        size_t endIdx = std::min(startIdx + VISIBLE_ITEMS, bookmarkGroups.size());
        
        for (size_t i = startIdx; i < endIdx; ++i) {
            std::string displayText = "  " + bookmarkGroups[i];
            if (i == selectedAddBookmarkGroup) {
                displayText = "> " + bookmarkGroups[i];
                // Highlight selected
                XSetForeground(display, gc, selectionColor);
                XFillRectangle(display, window, gc, DIALOG_X + 15, y - 12, DIALOG_WIDTH - 30, 15);
                XSetForeground(display, gc, textColor);
            }
            XDrawString(display, window, gc, DIALOG_X + 20, y, displayText.c_str(), displayText.length());
            y += 18;
        }
        
        // Show scroll indicator if needed
        if (bookmarkGroups.size() > VISIBLE_ITEMS) {
            std::string scrollText = "[" + std::to_string(selectedAddBookmarkGroup + 1) + "/" + std::to_string(bookmarkGroups.size()) + "]";
            XDrawString(display, window, gc, DIALOG_X + DIALOG_WIDTH - 60, DIALOG_Y + 40, scrollText.c_str(), scrollText.length());
        }
#endif
    }
    
    void drawHelpDialog() {
#ifdef __linux__
        if (!helpDialogVisible) return;
        
        // Dialog dimensions
        const int DIALOG_WIDTH = 600;
        const int DIALOG_HEIGHT = 500;
        const int DIALOG_X = (WINDOW_WIDTH - DIALOG_WIDTH) / 2;
        const int DIALOG_Y = (WINDOW_HEIGHT - DIALOG_HEIGHT) / 2;
        
        // Draw dialog background
        XSetForeground(display, gc, backgroundColor);
        XFillRectangle(display, window, gc, DIALOG_X, DIALOG_Y, DIALOG_WIDTH, DIALOG_HEIGHT);
        XSetForeground(display, gc, borderColor);
        XDrawRectangle(display, window, gc, DIALOG_X, DIALOG_Y, DIALOG_WIDTH, DIALOG_HEIGHT);
        
        // Draw title
        std::string title = "MMRY Keyboard Shortcuts";
        int titleWidth = XTextWidth(font, title.c_str(), title.length());
        XDrawString(display, window, gc, DIALOG_X + (DIALOG_WIDTH - titleWidth) / 2, DIALOG_Y + 25, title.c_str(), title.length());
        
        // Draw help content
        XSetForeground(display, gc, textColor);
        int y = DIALOG_Y + 50;
        int lineHeight = 15;
        
        // Main Window shortcuts
        XDrawString(display, window, gc, DIALOG_X + 20, y, "Main Window:", 11);
        y += lineHeight;
        XDrawString(display, window, gc, DIALOG_X + 30, y, "j/k          - Navigate items", 29);
        y += lineHeight;
        XDrawString(display, window, gc, DIALOG_X + 30, y, "g/G          - Top/bottom", 25);
        y += lineHeight;
        XDrawString(display, window, gc, DIALOG_X + 30, y, "Enter        - Copy item", 24);
        y += lineHeight;
        XDrawString(display, window, gc, DIALOG_X + 30, y, "/            - Filter mode", 26);
        y += lineHeight;
        XDrawString(display, window, gc, DIALOG_X + 30, y, "Shift+M      - Manage bookmark groups", 37);
        y += lineHeight;
        XDrawString(display, window, gc, DIALOG_X + 30, y, "m            - Add to bookmark", 30);
        y += lineHeight;
        XDrawString(display, window, gc, DIALOG_X + 30, y, "`            - View bookmarks", 29);
        y += lineHeight;
        XDrawString(display, window, gc, DIALOG_X + 30, y, "?            - This help", 24);
        y += lineHeight;
        XDrawString(display, window, gc, DIALOG_X + 30, y, "Shift+D      - Delete item", 26);
        y += lineHeight;
        XDrawString(display, window, gc, DIALOG_X + 30, y, "Shift+Q      - Quit", 19);
        y += lineHeight;
        XDrawString(display, window, gc, DIALOG_X + 30, y, "Escape       - Hide window", 26);
        y += lineHeight + 5;
        
        // Filter Mode shortcuts
        XDrawString(display, window, gc, DIALOG_X + 20, y, "Filter Mode:", 12);
        y += lineHeight;
        XDrawString(display, window, gc, DIALOG_X + 30, y, "Type text    - Filter items", 27);
        y += lineHeight;
        XDrawString(display, window, gc, DIALOG_X + 30, y, "Backspace    - Delete char", 26);
        y += lineHeight;
        XDrawString(display, window, gc, DIALOG_X + 30, y, "Enter        - Copy item", 24);
        y += lineHeight;
        XDrawString(display, window, gc, DIALOG_X + 30, y, "Escape       - Exit filter", 26);
        y += lineHeight + 5;
        
        // Bookmark Dialog shortcuts
        XDrawString(display, window, gc, DIALOG_X + 20, y, "Bookmark Dialogs:", 17);
        y += lineHeight;
        XDrawString(display, window, gc, DIALOG_X + 30, y, "All dialogs: up/down or j/k - Navigate", 38);
        y += lineHeight;
        XDrawString(display, window, gc, DIALOG_X + 30, y, "Mgmt: Enter - Create/select, Shift+D - Delete", 45);
        y += lineHeight;
        XDrawString(display, window, gc, DIALOG_X + 30, y, "Add: Enter - Add to group", 25);
        y += lineHeight;
        XDrawString(display, window, gc, DIALOG_X + 30, y, "View: g/G - Top/bottom, Enter - Select", 38);
        y += lineHeight;

        y += lineHeight + 10;
        
        // Global hotkey
        XDrawString(display, window, gc, DIALOG_X + 20, y, "Global Hotkey:", 14);
        y += lineHeight;
        XDrawString(display, window, gc, DIALOG_X + 30, y, "Ctrl+Alt+C   - Show/hide window", 31);
        
#endif
    }
    
    void drawViewBookmarksDialog() {
#ifdef __linux__
        if (!viewBookmarksDialogVisible) return;
        
        // Dialog dimensions
        const int DIALOG_WIDTH = 600;
        const int DIALOG_HEIGHT = 500;
        const int DIALOG_X = (WINDOW_WIDTH - DIALOG_WIDTH) / 2;
        const int DIALOG_Y = (WINDOW_HEIGHT - DIALOG_HEIGHT) / 2;
        
        // Draw dialog background
        XSetForeground(display, gc, backgroundColor);
        XFillRectangle(display, window, gc, DIALOG_X, DIALOG_Y, DIALOG_WIDTH, DIALOG_HEIGHT);
        XSetForeground(display, gc, borderColor);
        XDrawRectangle(display, window, gc, DIALOG_X, DIALOG_Y, DIALOG_WIDTH, DIALOG_HEIGHT);
        
        // Draw title
        std::string title = viewBookmarksShowingGroups ? "Select Bookmark Group" : "View Bookmarks: " + bookmarkGroups[selectedViewBookmarkGroup];
        int titleWidth = XTextWidth(font, title.c_str(), title.length());
        XDrawString(display, window, gc, DIALOG_X + (DIALOG_WIDTH - titleWidth) / 2, DIALOG_Y + 25, title.c_str(), title.length());
        
        if (viewBookmarksShowingGroups) {
            // Show bookmark groups list with scrolling
            XSetForeground(display, gc, textColor);
            int y = DIALOG_Y + 60;
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
                    XFillRectangle(display, window, gc, DIALOG_X + 15, y - 12, DIALOG_WIDTH - 30, 15);
                    XSetForeground(display, gc, textColor);
                } else {
                    displayText = "  " + displayText;
                }
                
                XDrawString(display, window, gc, DIALOG_X + 20, y, displayText.c_str(), displayText.length());
                y += 18;
            }
            
            // Show scroll indicator if needed
            if (bookmarkGroups.size() > VISIBLE_ITEMS) {
                std::string scrollText = "[" + std::to_string(selectedViewBookmarkGroup + 1) + "/" + std::to_string(bookmarkGroups.size()) + "]";
                XDrawString(display, window, gc, DIALOG_X + DIALOG_WIDTH - 60, DIALOG_Y + 40, scrollText.c_str(), scrollText.length());
            }
            

            
        } else {
            // Show bookmark items for selected group
            if (selectedViewBookmarkGroup < bookmarkGroups.size()) {
                std::string selectedGroup = bookmarkGroups[selectedViewBookmarkGroup];
                std::string bookmarkFile = configDir + "/bookmarks_" + selectedGroup + ".txt";
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
                    int itemY = DIALOG_Y + 60;
                    const int VISIBLE_ITEMS = 20;
                    
                    size_t startIdx = viewBookmarksScrollOffset;
                    size_t endIdx = std::min(startIdx + VISIBLE_ITEMS, bookmarkItems.size());
                    
                    for (size_t i = startIdx; i < endIdx; ++i) {
                        std::string displayText = bookmarkItems[i];
                        
                        // Truncate if too long
                        if (displayText.length() > 70) {
                            displayText = displayText.substr(0, 67) + "...";
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
                            XFillRectangle(display, window, gc, DIALOG_X + 15, itemY - 12, DIALOG_WIDTH - 30, 15);
                            XSetForeground(display, gc, textColor);
                        } else {
                            displayText = "  " + displayText;
                        }
                        
                        XDrawString(display, window, gc, DIALOG_X + 20, itemY, displayText.c_str(), displayText.length());
                        itemY += 18;
                    }
                    
                    // Show scroll indicator if needed
                    if (bookmarkItems.size() > VISIBLE_ITEMS) {
                        std::string scrollText = "[" + std::to_string(selectedViewBookmarkItem + 1) + "/" + std::to_string(bookmarkItems.size()) + "]";
                        XDrawString(display, window, gc, DIALOG_X + DIALOG_WIDTH - 60, DIALOG_Y + 40, scrollText.c_str(), scrollText.length());
                    }
                    
                    if (bookmarkItems.empty()) {
                        XDrawString(display, window, gc, DIALOG_X + 20, itemY, "No bookmarks in this group", 26);
                    }
                }
            }
            

        }
        
#endif
    }
    
    void drawConsole() {
        if (!visible) return;
        
#ifdef __linux__
        // Clear window with theme background
        XSetWindowBackground(display, window, backgroundColor);
        XClearWindow(display, window);
        
        // Draw filter textbox if in filter mode
        int startY = 20;
        if (filterMode) {
            // Draw filter input
            std::string filterDisplay = "/" + filterText;
            XDrawString(display, window, gc, 10, startY, filterDisplay.c_str(), filterDisplay.length());
            startY += 20;
        }
        
        // Draw clipboard items
        int y = startY;
        int maxItems = (WINDOW_HEIGHT - startY - 20) / 15; // Approximate lines that fit
        
        size_t displayCount = getDisplayItemCount();
        size_t startIdx = consoleScrollOffset;
        size_t endIdx = std::min(startIdx + maxItems, displayCount);
        
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
                if (content.length() > 50) {
                    content = content.substr(0, 47) + "...";
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
                if (content.length() > 80) {
                    content = content.substr(0, 77) + "...";
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
                XFillRectangle(display, window, gc, 5, y - 12, WINDOW_WIDTH - 10, 15);
                XSetForeground(display, gc, textColor);
            } else {
                XSetForeground(display, gc, textColor);
            }
            
            XDrawString(display, window, gc, 10, y, line.c_str(), line.length());
            
            y += 15;
        }
        
        // Show scroll indicator if needed
        if (displayCount > maxItems) {
            std::string scrollText = "[" + std::to_string(selectedItem + 1) + "/" + std::to_string(displayCount) + "]";
            XDrawString(display, window, gc, WINDOW_WIDTH - 80, 15, scrollText.c_str(), scrollText.length());
        }
        
        if (displayCount == 0) {
            std::string empty = filterMode ? "No matching items..." : "No clipboard items yet...";
            XDrawString(display, window, gc, 10, y, empty.c_str(), empty.length());
        }
        
        // Draw dialogs if visible
        drawBookmarkDialog();
        drawAddToBookmarkDialog();
        drawViewBookmarksDialog();
        drawHelpDialog();
#endif

#ifdef _WIN32
        // Windows drawing
#endif

#ifdef __APPLE__
        // macOS drawing
#endif
    }
    
    void startClipboardMonitoring() {
        // Start a separate thread for clipboard monitoring
        std::thread([this]() {
            while (running) {
                checkClipboard();
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
        }).detach();
    }
    
    void checkClipboard() {
        std::string content;
        
#ifdef __linux__
        // Linux: use xclip command
        FILE* pipe = popen("xclip -selection clipboard -o 2>/dev/null", "r");
        if (!pipe) return;
        
        char buffer[4096];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            content += buffer;
        }
        pclose(pipe);
#endif

#ifdef _WIN32
        // Windows: use Windows API
        if (OpenClipboard(nullptr)) {
            HANDLE hData = GetClipboardData(CF_TEXT);
            if (hData) {
                char* text = static_cast<char*>(GlobalLock(hData));
                if (text) {
                    content = text;
                    GlobalUnlock(hData);
                }
            }
            CloseClipboard();
        }
#endif

#ifdef __APPLE__
        // macOS: use pbpaste command
        FILE* pipe = popen("pbpaste 2>/dev/null", "r");
        if (!pipe) return;
        
        char buffer[4096];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            content += buffer;
        }
        pclose(pipe);
#endif
        
        // Trim trailing newlines
        while (!content.empty() && (content.back() == '\n' || content.back() == '\r')) {
            content.pop_back();
        }
        
        if (!content.empty() && content != lastClipboardContent) {
            lastClipboardContent = content;
            
            // Check for duplicates
            bool isDuplicate = false;
            for (const auto& item : items) {
                if (item.content == content) {
                    isDuplicate = true;
                    break;
                }
            }
            
            if (!isDuplicate) {
                items.emplace(items.begin(), content);
                if (items.size() > maxClips) { // Keep only last maxClips items
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
        }
    }
    
    void loadConfig() {
        std::string configFile = configDir + "/config.json";
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
    
    void createDefaultConfig() {
        std::string configFile = configDir + "/config.json";
        std::ofstream outFile(configFile);
        outFile << "{\n";
        outFile << "    \"verbose\": false,\n";
        outFile << "    \"max_clips\": 500,\n";
        outFile << "    \"encrypted\": true,\n";
        outFile << "    \"encryption_key\": \"mmry_default_key_2024\",\n";
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
            }
        }
    }
};

int main() {
    ClipboardManager manager;
    manager.run();
    return 0;
}
