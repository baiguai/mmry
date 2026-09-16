#ifndef CLIPBOARD_MANAGER_H
#define CLIPBOARD_MANAGER_H

#include "main.h"
#include "key_translation.h"
#include "help.h"
#include "ui.h"
#include "config.h"
#include "utils.h"

// Global X11 state shared by main.cpp and key_handling.cpp
#ifdef __linux__
inline Display* display;
inline Window window;
inline Window root;
inline int screen;
inline GC gc;
inline XFontStruct* font;
inline Atom clipboardAtom;
inline Atom utf8Atom;
inline Atom textAtom;
inline int xfixes_event_base;
inline int xfixes_error_base;
#endif

class ClipboardManager
{

public:
    ClipboardManager()
    {
        logfile.open("mmry_debug.log");
        writeLog("________ NEW MMRY SESSION ________");
        writeLog("");
        writeLog("");
    }

    ~ClipboardManager()
    {
        writeLog("");
        writeLog("");
        writeLog("");
        writeLog("");
        logfile.close();
        std::cout << "ClipboardManager destructor called - cleaning up resources\n";
        stop();
    }

private:
    std::atomic<bool> hotkeyGrabbed{false};
    mutable std::ofstream logfile;
    ConfigManager config;

    // Helper method for logging
    void writeLog(const std::string& message) const
    {
        if (config.m_debugging)
        {
            auto now = std::chrono::system_clock::now();
            auto in_time_t = std::chrono::system_clock::to_time_t(now);
            logfile << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X") << " | " << message << std::endl;
        }
    }

    void moveCursorWordLeft();
    void moveCursorWordRight();

public:

//// KEY HANDLING //////////////////////////////////////////////////////////////
///
///

#ifdef __linux__
        void handleKeyPress(XEvent* event);
#endif
        void handleKeyPressCommon(void* eventPtr);

    //// Key Press Methods /////////////////////////////////////////////////////
        bool key_global_escape();

        // Edit Keys
        bool key_edit_escape();
        bool key_edit_save();
        bool key_edit_backspace();
        bool key_edit_delete();
        void key_edit_add_char(char c);
        bool key_edit_add_newline();
        bool key_edit_scroll_up();
        bool key_edit_scroll_down();

        // Help Keys
        bool key_help_hide();
        bool key_help_scroll_down();
        bool key_help_scroll_up();
        bool key_help_scroll_top();

        // Add Groups
        bool key_addgroup_add();
        bool key_addgroup_back();

        // Bookmarks
        bool key_marks_show();
        bool key_marks_groups_down();
        bool key_marks_groups_up();
        bool key_marks_groups_top();
        bool key_marks_groups_bottom();
        bool key_marks_groups_delete();
        bool key_marks_groups_clips();
        bool key_marks_clips_down();
        bool key_marks_clips_up();
        bool key_marks_clips_top();
        bool key_marks_clips_bottom();
        bool key_marks_clips_delete();
        bool key_marks_clips_copy();
        bool key_marks_clips_groups();

        // Pinned Clips
        bool key_pin_down();
        bool key_pin_up();
        bool key_pin_top();
        bool key_pin_bottom();
        bool key_pin_delete();
        bool key_pin_copy();

        // Add Bookmarks to Groups
        bool key_addmarks_add();
        bool key_addmarks_down();
        bool key_addmarks_up();
        bool key_addmarks_top();
        bool key_addmarks_bottom();

        // Filter Mode
        bool key_filter_delete();
        bool key_filter_copy();
        bool key_filter_down();
        bool key_filter_up();

        // Command
        bool key_command_execute();
        bool key_command_down();
        bool key_command_up();
        bool key_command_detect();

        // Theme Command
        bool key_theme_cancel();
        bool key_theme_apply();
        bool key_theme_down();
        bool key_theme_up();
        bool key_theme_top();
        bool key_theme_bottom();

        // Config Command
        bool key_config_cancel();
        bool key_config_select();
        bool key_config_down();
        bool key_config_up();
        bool key_config_top();
        bool key_config_bottom();

        // Main Clips List
        bool key_main_down();
        bool key_main_up();
        bool key_main_top();
        bool key_main_bottom();

        bool key_main_delete();
        bool key_main_filter_start();
        bool key_main_command_start();
        bool key_main_copy();
        bool key_main_addgroup_start();
        bool key_main_addclip_start();
        bool key_main_help_start();
        bool key_main_accessmarks_start();
        bool key_main_pin_clip();
        bool key_main_pins_start();


        bool key_main_edit_start();
        bool key_edit_cursor_left();
        bool key_edit_cursor_right();
        bool key_edit_cursor_up();
        bool key_edit_cursor_down();
        bool key_edit_home();
        bool key_edit_end();

    //// End Key Press Methods /////////////////////////////////////////////////


///
///
//// END KEY HANDLING //////////////////////////////////////////////////////////



    // Core Methods
    void updateWindowDimensions(int newWidth, int newHeight);
    void updateClipListWidth();
    void run();
    void setRunning(bool state);
    void stop();

    
#ifdef _WIN32
    HWND hwnd = nullptr;
    HFONT font = nullptr;

    char getCharFromMsg(MSG* msg)
    {
        // Get the scan code from lParam
        UINT scanCode = (msg->lParam >> 16) & 0xFF;

        // Get current keyboard state
        BYTE keyboardState[256];
        GetKeyboardState(keyboardState);

        // Convert virtual key to character
        char charBuffer[2]; // Needs space for null terminator
        int result = ToAscii(msg->wParam, scanCode, keyboardState, (LPWORD)charBuffer, 0);

        if (result == 1)
        {
            return charBuffer[0];
        }
        return 0; // Return null character if conversion fails
    }
#endif
    
    void updateFilteredBookmarkClips();
    size_t getBookmarkItemCount();
    
    // Helper methods

    size_t getDisplayItemCount();
    size_t getActualItemIndex(size_t displayIndex);
    void updateFilteredItems();
                    
    void updateScrollOffset();
    void updateEditDialogScrollOffset();
    void updateConsoleScrollOffset();
    void updateBookmarkMgmtScrollOffset();
    void updatePinnedScrollOffset();
    void updateAddBookmarkScrollOffset();
    void updateThemeSelectScrollOffset();
    void updateConfigSelectScrollOffset();
    void updateHelpDialogScrollOffset(int adjustment);

    
    int getClipListWidth() const
    {
        return clipListWidth;
    }
    
    int getWindowWidth() const
    {
        return windowWidth;
    }
    
    int getWindowHeight() const
    {
        return windowHeight;
    }
    
    
    
    // Dialog positioning and sizing structure

    
    void executeCommand(const std::string& command)
    {
        // Parse command and arguments
        std::istringstream iss(command);
        std::string cmd;
        std::string args;
        
        if (iss >> cmd)
        {
            std::getline(iss, args);
            // Trim leading whitespace from args
            if (!args.empty() && args[0] == ' ')
            {
                args = args.substr(1);
            }
        }
        
        if (cmd == "theme")
        {
            if (!args.empty())
            {
                // Direct theme switch: "theme dracula"
                config.switchTheme(args);
            }
            else
            {
                // Enter theme selection mode: "theme"
                commandMode = false;
                cmd_themeSelectMode = true;
                availableThemes = config.discoverThemes();
                // Store original theme and apply first theme for preview
                if (!availableThemes.empty())
                {
                    config.originalTheme = config.theme;
                    selectedTheme = 0;
                    config.switchTheme(availableThemes[0]);
                }
                drawConsole();
            }
            return;
        }
        
        if (cmd == "config")
        {
            if (!args.empty())
            {
                std::cout << "DEBUG: Processing config command with args: '" << args << "'\n";
                // Parse "config key value" format
                std::istringstream configIss(args);
                std::string configKey, configValue;
                
                if (configIss >> configKey)
                {
                    std::string remaining;
                    std::getline(configIss, remaining);
                    // Trim leading whitespace from config value
                    if (!remaining.empty() && remaining[0] == ' ')
                    {
                        remaining = remaining.substr(1);
                    }
                    configValue = remaining;
                    
                    std::cout << "DEBUG: Parsed configKey='" << configKey << "', configValue='" << configValue << "'\n";
                    
                        // Validate and update config based on type
                    if (config.updateConfigValue(configKey, configValue))
                    {
                        std::cout << "DEBUG: updateConfigValue returned true, calling saveConfig()\n";
                        config.saveConfig();
                        std::cout << "Updated " << configKey << " = " << configValue << "\n";
                    }
                    else
                    {
                        std::cout << "DEBUG: updateConfigValue returned false\n";
                        std::cout << "Invalid value for " << configKey << ". Expected type: " << config.getConfigType(configKey) << "\n";
                    }
                }
                else
                {
                    std::cout << "DEBUG: Failed to parse config key from args\n";
                }
            }
            else
            {
                // Enter config selection mode: "config"
                commandMode = false;
                cmd_configSelectMode = true;
                availableConfigs = config.discoverConfigs();
                drawConsole();
            }
            return;
        }
        
        // Handle other commands (for future implementation)
        std::cout << "Command executed: " << command << "\n";
        
        // TODO: Add specific command implementations here
        // Examples:
        // - "delete" - delete selected item
        // - "pin" - pin selected item  
        // - "clear" - clear all items
        // - "export" - export clipboard history
    }
    
    void loadBookmarkGroups()
    {
        // Cross-platform path separator
#ifdef _WIN32
        const char pathSep = '\\';
#else
        const char pathSep = '/';
#endif
        
        std::string bookmarkFile = config.bookmarksDir + pathSep + "bookmarks.txt";
        std::ifstream file(bookmarkFile);
        bookmarkGroups.clear();
        
        if (file.is_open())
        {
            std::string line;
            while (std::getline(file, line))
            {
                if (!line.empty() && line.find('|') != std::string::npos)
                {
                    size_t pos = line.find('|');
                    std::string groupName = line.substr(0, pos);
                    if (!groupName.empty())
                    {
                        bookmarkGroups.push_back(groupName);
                    }
                }
            }
            file.close();
        }
        
        // Always ensure we have at least one group
        if (bookmarkGroups.empty())
        {
            bookmarkGroups.push_back("default");
        }
    }
    
    void saveBookmarkGroups()
    {
        std::string bookmarkFile = config.bookmarksDir + "/bookmarks.txt";
        std::ofstream file(bookmarkFile);
        
        if (file.is_open())
        {
            for (const auto& group : bookmarkGroups)
            {
                file << group << "|0\n"; // Group name | clip count
            }
            file.close();
        }
    }
    
    void addClipToBookmarkGroup(const std::string& groupName, const std::string& content)
    {
        std::string bookmarkFile = config.bookmarksDir + "/bookmarks_" + groupName + ".txt";
        std::ofstream file(bookmarkFile, std::ios::app);
        
        if (file.is_open())
        {
            auto timestamp = std::chrono::system_clock::now().time_since_epoch().count();
            std::string contentToSave = encrypt(content, config);
            file << timestamp << "|" << contentToSave << "\n";
            file.close();
        }
    }

    void addClipToPinned(const std::string& content)
    {
        std::ofstream file(config.pinnedFile, std::ios::app);
        
        if (file.is_open())
        {
            auto timestamp = std::chrono::system_clock::now().time_since_epoch().count();
            std::string contentToSave = encrypt(content, config);
            file << timestamp << "|" << contentToSave << "\n";
            file.close();
        }
    }

    void createWindow()
    {
#ifdef __linux__
        // Create window with theme colors
        window = XCreateSimpleWindow(display, root, 
                                   config.windowX, config.windowY, 
                                   windowWidth, windowHeight,
                                   2, config.borderColor, config.backgroundColor);
        
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
        XSetForeground(display, gc, config.textColor);
        
        // Load font (try to find a monospace font)
        font = XLoadQueryFont(display, "-*-fixed-medium-r-*-*-13-*-*-*-*-*-*-*");
        if (!font)
        {
            font = XLoadQueryFont(display, "fixed");
        }
        if (font)
        {
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
    
    void setupHotkeys()
    {
#ifdef __linux__
        // First, ensure X11 is fully synchronized
        XSync(display, False);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        // Install X11 error handler to catch grab failures
        auto oldHandler = XSetErrorHandler([](Display* /*d*/, XErrorEvent* e) -> int {
            if (e->error_code == BadAccess)
            {
                std::cerr << "X11 Error: BadAccess when trying to grab key\n";
                return 0;
            }
            return 0;
        });
        
        // Try multiple times with exponential backoff
        const int MAX_RETRIES = 5;
        bool success = false;
        
        for (int retry = 0; retry < MAX_RETRIES && !success; retry++)
        {
            if (retry > 0)
            {
                std::cerr << "Retry " << retry << " of " << MAX_RETRIES << "...\n";
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
            unsigned int modifiers[] =
            {
                ControlMask | Mod1Mask,                    // Ctrl+Alt
                ControlMask | Mod1Mask | Mod2Mask,         // Ctrl+Alt+NumLock
                ControlMask | Mod1Mask | LockMask,         // Ctrl+Alt+CapsLock
                ControlMask | Mod1Mask | Mod2Mask | LockMask // Ctrl+Alt+NumLock+CapsLock
            };
            
            bool grabFailed = false;
            for (unsigned int mod : modifiers)
            {
                int result = XGrabKey(display, keycode, mod, root, True, 
                                     GrabModeAsync, GrabModeAsync);
                
                // Force synchronization to detect errors immediately
                XSync(display, False);
                
                if (result == BadAccess)
                {
                    grabFailed = true;
                    break;
                }
            }
            
            if (!grabFailed)
            {
                success = true;
                hotkeyGrabbed = true;
                std::cout << "Successfully grabbed Ctrl+Alt+C hotkey\n";
            }
        }
        
        // Restore old error handler
        XSetErrorHandler(oldHandler);
        
        if (!success)
        {
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

    
    void showWindow()
    {
        std::cout << "Visible: " << visible << "\n";

        if (!visible)
        {
#ifdef __linux__
            XMapWindow(display, window);
#endif
#ifdef _WIN32
            ShowWindow(hwnd, SW_SHOW);
#endif
            visible = true;
            std::cout << "Window shown\n";
        }
    }
    
    void hideWindow()
    {
        if (visible)
        {
#ifdef __linux__
            updateWindowPosition();
            XUnmapWindow(display, window);
#endif
#ifdef _WIN32
            if (hwnd)
            {
                std::cout << "Calling ShowWindow(SW_HIDE)\n";
                updateWindowPosition();
                ShowWindow(hwnd, SW_HIDE);
            }
            else
            {
                std::cout << "hwnd is null!\n";
            }
#endif
            visible = false;
            std::cout << "Window hidden\n";
        }
    }

    void updateWindowPosition()
    {
#ifdef __linux__
        if (!display)
        {
            return;
        }
        Window child;
        int x { 0 };
        int y { 0 };
        if (XTranslateCoordinates(display, window, root, 0, 0, &x, &y, &child))
        {
            config.windowX = x;
            config.windowY = y;
        }
#endif
#ifdef _WIN32
        if (hwnd)
        {
            RECT rect;
            if (GetWindowRect(hwnd, &rect))
            {
                config.windowX = rect.left;
                config.windowY = rect.top;
            }
        }
#endif
    }

    void restoreWindowPosition()
    {
#ifdef __linux__
        if (!display)
        {
            return;
        }
        XMoveWindow(display, window, config.windowX, config.windowY);
        XSync(display, False);
#endif
#ifdef _WIN32
        if (hwnd)
        {
            SetWindowPos(hwnd, NULL, config.windowX, config.windowY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        }
#endif
    }
    
    



//// UI METHODS ////////////////////////////////////////////////////////////////
///
///

public:
    // Linux UI Methods
    // !@!
#ifdef __linux__
        void drawConsole()
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
        void drawConsole()
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

///
///
//// END UI METHODS ////////////////////////////////////////////////////////////



    
#ifdef __linux__
    void requestClipboardContent()
    {
        // Request clipboard content as UTF8_STRING
        XConvertSelection(display, clipboardAtom, utf8Atom, clipboardAtom, window, CurrentTime);
    }
#endif

#ifdef __linux__
    void handleSelectionNotify(XEvent* event)
    {
        if (event->xselection.property == None)
        {
            // If UTF8_STRING is not available, try plain TEXT
            XConvertSelection(display, clipboardAtom, XA_STRING, clipboardAtom, window, CurrentTime);
            return;
        }

        Atom target = event->xselection.target;
        if (target != utf8Atom && target != XA_STRING)
        {
            // We are not interested in other formats
            return;
        }

        Atom type;
        int format;
        unsigned long nitems, bytes_after;
        unsigned char* data = nullptr;

        XGetWindowProperty(display, window, clipboardAtom, 0, LONG_MAX, False, AnyPropertyType,
                           &type, &format, &nitems, &bytes_after, &data);

        if (data)
        {
            std::string content(reinterpret_cast<char*>(data), nitems);
            XFree(data);
            processClipboardContent(content);
        }
    }
#endif

    void processClipboardContent(const std::string& content)
    {
        // Trim trailing newlines
        std::string trimmed_content = content;
        while (!trimmed_content.empty() && (trimmed_content.back() == '\n' || trimmed_content.back() == '\r'))
        {
            trimmed_content.pop_back();
        }

        if (trimmed_content.empty() || trimmed_content == lastClipboardContent)
        {
            return;
        }

        lastClipboardContent = trimmed_content;

        // The rest of the logic from checkClipboard
        size_t duplicateIndex = 0;
        
        // Check for duplicates and move to top if found
        bool isDuplicate = false;
        for (size_t i = 0; i < items.size(); i++)
        {
            if (items[i].content == trimmed_content)
            {
                isDuplicate = true;
                duplicateIndex = i;
                break;
            }
        }
      
        if (isDuplicate)
        {
            // Move existing clip to top
            std::string clipContent = items[duplicateIndex].content;
            items.erase(items.begin() + duplicateIndex);
            items.emplace(items.begin(), clipContent);

            // Reset selection to top when item is moved
            selectedItem = 0;

            // Update filtered items if in filter mode
            if (filterMode)
            {
                updateFilteredItems();
            }

            saveToFile();

            std::cout << "Existing clip moved to top\n";

            // Refresh display if window is visible
            if (visible)
            {
                drawConsole();
            }

            return;
        }

        items.emplace(items.begin(), trimmed_content);
        while (items.size() > config.maxClips)
        {
            items.pop_back();
        }
        
        // Reset selection to top when new item is added
        selectedItem = 0;
        
        // Update filtered items if in filter mode
        if (filterMode)
        {
            updateFilteredItems();
        }
        
        saveToFile();
        
        std::cout << "New clipboard item added\n";
        
        // Refresh display if window is visible
        if (visible)
        {
            drawConsole();
        }
    }
    
    void copyToClipboard(const std::string& content)
    {
#ifdef __linux__
        // Use xclip to copy to clipboard
        FILE* pipe = popen("xclip -selection clipboard", "w");
        if (pipe)
        {
            fwrite(content.c_str(), 1, content.length(), pipe);
            pclose(pipe);
        }
#endif

#ifdef _WIN32
        // Windows clipboard
        if (OpenClipboard(nullptr))
        {
            EmptyClipboard();
            HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, content.length() + 1);
            if (hMem)
            {
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
        if (pipe)
        {
            fwrite(content.c_str(), 1, content.length(), pipe);
            pclose(pipe);
        }
#endif
    }
    
    void saveToFile()
    {
        std::ofstream file(config.dataFile);
        if (file.is_open())
        {
            for (const auto& item : items)
            {
                // Store timestamp and content (encrypted if enabled)
                auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                    item.timestamp.time_since_epoch()).count();
                std::string contentToSave = encrypt(item.content, config);
                file << timestamp << "|" << contentToSave << "\n";
            }
            file.close();
        }
    }
    
    void loadFromFile()
    {
        std::ifstream file(config.dataFile);
        if (file.is_open())
        {
            std::string line;
            while (std::getline(file, line))
            {
                size_t pos = line.find('|');
                if (pos != std::string::npos && pos > 0)
                {
                    std::string timestampStr = line.substr(0, pos);
                    std::string content = line.substr(pos + 1);
                    
                    try
                    {
                        std::string decryptedContent;
                        
                        // Try to decrypt first
                        try
                        {
                            decryptedContent = decrypt(content, config);
                            // Check if decryption produced reasonable results (no control characters)
                            bool hasControlChars = false;
                            for (char c : decryptedContent)
                            {
                                if (c < 32 && c != '\n' && c != '\r' && c != '\t')
                                {
                                    hasControlChars = true;
                                    break;
                                }
                            }
                            
                            // If decryption produced garbage, assume the content was never encrypted
                            if (hasControlChars || decryptedContent.empty())
                            {
                                decryptedContent = content;
                            }
                        }
                        catch (...)
                        {
                            // If decryption fails, assume content was never encrypted
                            decryptedContent = content;
                        }
                        
                        ClipboardItem item(decryptedContent);
                        auto timestamp = std::chrono::seconds(std::stoll(timestampStr));
                        item.timestamp = std::chrono::system_clock::time_point(timestamp);
                        
                        items.push_back(item);
                    }
                    catch (const std::exception& e)
                    {
                        // Skip invalid entries
                        continue;
                    }
                }
            }
            file.close();
        }
        else
        {
            // Create empty clips.txt file if it doesn't exist
            std::ofstream outFile(config.dataFile);
            if (outFile.is_open())
            {
                outFile.close();
                std::cout << "Created empty clips file: " << config.dataFile << "\n";
            }
            else
            {
                std::cerr << "Failed to create clips file: " << config.dataFile << "\n";
            }
        }
    }
};

#endif // CLIPBOARD_MANAGER_H
