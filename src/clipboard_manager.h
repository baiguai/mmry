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

    char getCharFromMsg(MSG* msg);
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

    void drawConsole();

    
#ifdef __linux__
    void requestClipboardContent();
    void handleSelectionNotify(XEvent* event);
#endif

    void processClipboardContent(const std::string& content);
    void copyToClipboard(const std::string& content);
    
    void saveToFile();
    void loadFromFile();

};

#endif // CLIPBOARD_MANAGER_H
