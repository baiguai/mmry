#include "main.h"
#include "key_translation.h"
#include "help.h"
#include "ui.h"
#include "config.h"
#include "utils.h"

/*

    ============================================================================
    MMRY
    Clipboard Manager
    ============================================================================


    -- SOURCE CODE -------------------------------------------------------------

    https://github.com/baiguai/mmry


    -- PROJECT STRUCTURE -------------------------------------------------------

    key_translations
        This file handles translating key codes from different platforms into
        one common set of key values.

    help
        This file handles the drawing of the help dialog as well as defining
        all of the help entries themselves.

    ui
        For the UI the code is broken out into each platform's logic.
        This file handles the drawing of all the various dialogs, including
        the main window.

    config
        This file handles all of the configuration logic, including handling
        themes and ensuring the required configuration files are present.

    utils
        This file contains various helper methods that are used throughout
        the project.

*/



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
    void handleKeyPress(XEvent* event)
    {
        handleKeyPressCommon(event);
    }
#endif

    void handleKeyPressCommon(void* eventPtr)
    {
        std::string key_value = translateKey(eventPtr);

#ifdef __linux__
        XEvent* event = (XEvent*)eventPtr;
        XKeyEvent* keyEvent = (XKeyEvent*)event;
        KeySym keysym;
        {
            char buf[10];
            XLookupString(keyEvent, buf, sizeof(buf), &keysym, nullptr);
        }
#endif
#ifdef _WIN32
        MSG* msg = (MSG*)eventPtr;
#endif
        if (key_value == "Q")
        {
            // Shift+Q quits application even from dialog
            std::cout << "Quitting MMRY...\n";
            running = false; // Let main loop exit naturally to avoid deadlock
        }

        //---- General Escape --------------------------------------------------
        if (key_value == "ESCAPE")
        {
            if (key_global_escape()) return;
        }
        //----------------------------------------------------------------------


        //---- Help Dialog -----------------------------------------------------
        if (helpDialogVisible)
        {
            if (key_value == "ESCAPE")
            {
                if (helpFilterMode)
                {
                    helpFilterMode = false;
                    helpFilterText.clear();
                    drawConsole();
                    return;
                }
                if (key_help_hide()) return;
            }

            if (helpFilterMode)
            {
                if (key_value == "BACKSPACE")
                {
                    if (!helpFilterText.empty())
                    {
                        helpFilterText.pop_back();
                        helpDialogScrollOffset = 0;
                        drawConsole();
                    }
                    return;
                }
                if (key_value == "DOWN")
                {
                    helpDialogScrollOffset++;
                    drawConsole();
                    return;
                }
                if (key_value == "UP")
                {
                    if (helpDialogScrollOffset > 0) helpDialogScrollOffset--;
                    drawConsole();
                    return;
                }
                if (key_value == "LEFT" || key_value == "RIGHT" || key_value == "HOME" || key_value == "END")
                {
                    return;
                }
#ifdef _WIN32
                char typedChar = getCharFromMsg(msg);
                if (typedChar != 0)
                {
                    helpFilterText += typedChar;
                    drawConsole();
                }
#else
                char buffer[10];
                int count = XLookupString(keyEvent, buffer, sizeof(buffer), nullptr, nullptr);
                if (count > 0)
                {
                    helpFilterText += std::string(buffer, count);
                    drawConsole();
                }
#endif
                return;
            }

            if (key_value == "/")
            {
                helpFilterMode = true;
                helpFilterText.clear();
                drawConsole();
                return;
            }

            if (key_value == "?")
            {
                if (key_help_hide()) return;
            }

            if (key_value == "j" || key_value == "DOWN")
            {
                if (key_help_scroll_down()) return;
            }

            if (key_value == "k" || key_value == "UP")
            {
                if (key_help_scroll_up()) return;
            }

            if (key_value == "g")
            {
                if (key_help_scroll_top()) return;
            }

            return;
        }

        //---- Edit Dialog -----------------------------------------------------
        if (editDialogVisible)
        {
            if (key_value == "ESCAPE")
            {
                if (key_edit_escape()) return;
            }

            // Handle CTRL+ENTER for saving
#ifdef __linux__
            if (keysym == XK_Return && (keyEvent->state & ControlMask))
            {
                if (key_edit_save()) return;
            }
#endif
#ifdef _WIN32
            if (msg->wParam == VK_RETURN && (GetKeyState(VK_CONTROL) & 0x8000))
            {
                if (key_edit_save()) return;
            }
#endif
            // Handle CTRL+LEFT for moving one word left
#ifdef __linux__
            if (keysym == XK_Left && (keyEvent->state & ControlMask))
            {
                moveCursorWordLeft();
                drawConsole();
                return;
            }
#endif
#ifdef _WIN32
            if (msg->wParam == VK_LEFT && (GetKeyState(VK_CONTROL) & 0x8000))
            {
                moveCursorWordLeft();
                drawConsole();
                return;
            }
#endif

            // Handle CTRL+RIGHT for moving one word right
#ifdef __linux__
            if (keysym == XK_Right && (keyEvent->state & ControlMask))
            {
                moveCursorWordRight();
                drawConsole();
                return;
            }
#endif
#ifdef _WIN32
            if (msg->wParam == VK_RIGHT && (GetKeyState(VK_CONTROL) & 0x8000))
            {
                moveCursorWordRight();
                drawConsole();
                return;
            }
#endif
            
            if (key_value == "RETURN")
            {
                if (key_edit_add_newline()) return;
            }

            if (key_value == "BACKSPACE")
            {
                if (key_edit_backspace()) return;
            }

            if (key_value == "DELETE")
            {
                if (key_edit_delete()) return;
            }

            if (key_value == "UP")
            {
                if (key_edit_cursor_up()) return;
            }

            if (key_value == "DOWN")
            {
                if (key_edit_cursor_down()) return;
            }

            if (key_value == "LEFT")
            {
                if (key_edit_cursor_left()) return;
            }

            if (key_value == "RIGHT")
            {
                if (key_edit_cursor_right()) return;
            }

            if (key_value == "HOME")
            {
                if (key_edit_home()) return;
            }

            if (key_value == "END")
            {
                if (key_edit_end()) return;
            }

            // Text input
#ifdef _WIN32
            char typedChar = getCharFromMsg(msg);
            if (typedChar != 0 && typedChar != '\r' && typedChar != '\n') // Ignore enter key here as it is handled by CTRL+ENTER
            {
                key_edit_add_char(typedChar);
                return;
            }
#else
            char buffer[10];
            int count = XLookupString(keyEvent, buffer, sizeof(buffer), nullptr, nullptr);
            if (count > 0 && buffer[0] != '\r' && buffer[0] != '\n') // Ignore enter key here as it is handled by CTRL+ENTER
            {
                key_edit_add_char(buffer[0]);
                return;
            }
#endif
            return;
        }

        // ---------------------------------------------------------------------


        // Adding bookmark groups
        //
        if (bookmarkDialogVisible && !addToBookmarkDialogVisible)
        {
            if (key_value == "RETURN")
            {
                if (key_addgroup_add()) return;
            }

            if (key_value == "BACKSPACE")
            {
                if (key_addgroup_back()) return;
            }

            // Text input for bookmark dialog - exclude vim navigation keys
            // Plain Text
#ifdef _WIN32
            // Handle character input from WM_KEYDOWN
            char typedChar = getCharFromMsg(msg); 
            if (typedChar != 0)
            {
                bookmarkDialogInput += typedChar;  
                drawConsole();
            }
#else
            char buffer[10];
            int count = XLookupString(keyEvent, buffer, sizeof(buffer), nullptr, nullptr);
            if (count > 0)
            {
                bookmarkDialogInput += std::string(buffer, count);
                drawConsole();
            }
#endif
            // End Plain Text

            return;
        }


        // Accessing bookmarked clips
        //
        if (viewBookmarksDialogVisible)
        {
            if (key_value == "`")
            {
                if (key_marks_show()) return;
            }


            // Groups view
            //
            if (viewBookmarksShowingGroups)
            {
                if (filterBookmarksMode)
                {
                    if (key_value == "RETURN")
                    {
                        if (key_marks_groups_clips()) return;
                    }
                    if (key_value == "ESCAPE")
                    {
                        filterBookmarksMode = false;
                        filterBookmarksText.clear();
                        drawConsole();
                        return;
                    }
                    // ADDED: UP/DOWN arrow key handling for filtered bookmark groups
                    if (key_value == "j" || key_value == "DOWN")
                    {
                        if (key_marks_groups_down()) return;
                    }

                    if (key_value == "k" || key_value == "UP")
                    {
                        if (key_marks_groups_up()) return;
                    }
                    // End ADDED
                    if (key_value == "BACKSPACE")
                    {
                        if (!filterBookmarksText.empty())
                        {
                            filterBookmarksText.pop_back();
                            drawConsole();
                        }
                        return;
                    }
                    // Text input for filter
#ifdef _WIN32
                    char typedChar = getCharFromMsg(msg);
                    if (typedChar != 0)
                    {
                        filterBookmarksText += typedChar;
                        drawConsole();
                    }
#else
                    char buffer[10];
                    int count = XLookupString(keyEvent, buffer, sizeof(buffer), nullptr, nullptr);
                    if (count > 0)
                    {
                        filterBookmarksText += std::string(buffer, count);
                        drawConsole();
                    }
#endif
                    return;
                }

                if (key_value == "/")
                {
                    filterBookmarksMode = true;
                    filterBookmarksText.clear();
                    drawConsole();
                    return;
                }
                if (key_value == "j" || key_value == "DOWN")
                {
                    if (key_marks_groups_down()) return;
                }

                if (key_value == "k" || key_value == "UP")
                {
                    if (key_marks_groups_up()) return;
                }

                if (key_value == "g")
                {
                    if (key_marks_groups_top()) return;
                }

                if (key_value == "G")
                {
                    if (key_marks_groups_bottom()) return;
                }

                if (key_value == "D")
                {
                    if (key_marks_groups_delete()) return;
                }

                if (key_value == "RETURN")
                {
                    if (key_marks_groups_clips()) return;
                }
            }

            // Clips are being shown
            //
            else
            {
                if (filterBookmarkClipsMode)
                {
                    if (key_value == "RETURN")
                    {
                        if (key_marks_clips_copy()) return;
                    }
                    if (key_value == "ESCAPE")
                    {
                        filterBookmarkClipsMode = false;
                        filterBookmarkClipsText.clear();
                        drawConsole();
                        return;
                    }
                    if (key_value == "BACKSPACE")
                    {
                        if (!filterBookmarkClipsText.empty()) {
                            filterBookmarkClipsText.pop_back();
                            updateFilteredBookmarkClips();
                            drawConsole();
                        }
                        return;
                    }
                    // ADDED: UP/DOWN arrow key handling for filtered bookmark clips
                    if (key_value == "j" || key_value == "DOWN")
                    {
                        if (key_marks_clips_down()) return;
                    }

                    if (key_value == "k" || key_value == "UP")
                    {
                        if (key_marks_clips_up()) return;
                    }
                    // End ADDED
                    // Text input for filter
#ifdef _WIN32
                    char typedChar = getCharFromMsg(msg);
                    if (typedChar != 0 && typedChar != '\r' && typedChar != '\n')
                    {
                        // Don't add the triggering '/' as the first character
                        if (filterBookmarkClipsText.empty() && typedChar == '/')
                        {
                            // do nothing
                        }
                        else
                        {
                            filterBookmarkClipsText += typedChar;
                            updateFilteredBookmarkClips();
                            drawConsole();
                        }
                    }
#else
                    char buffer[10];
                    int count = XLookupString(keyEvent, buffer, sizeof(buffer), nullptr, nullptr);
                    if (count > 0 && buffer[0] != '\r' && buffer[0] != '\n')
                    {
                        // Don't add the triggering '/' as the first character
                        if (filterBookmarkClipsText.empty() && buffer[0] == '/')
                        {
                            // do nothing
                        }
                        else
                        {
                            filterBookmarkClipsText += std::string(buffer, count);
                            updateFilteredBookmarkClips();
                            drawConsole();
                        }
                    }
#endif
                    return;
                }

                if (key_value == "/")
                {
                    filterBookmarkClipsMode = true;
                    filterBookmarkClipsText.clear();
                    updateFilteredBookmarkClips(); // Initial filter
                    drawConsole();
                    return;
                }
                if (key_value == "j" || key_value == "DOWN")
                {
                    if (key_marks_clips_down()) return;
                }

                if (key_value == "k" || key_value == "UP")
                {
                    if (key_marks_clips_up()) return;
                }

                if (key_value == "g")
                {
                    if (key_marks_clips_top()) return;
                }

                if (key_value == "G")
                {
                    if (key_marks_clips_bottom()) return;
                }

                if (key_value == "D")
                {
                    if (key_marks_clips_delete()) return;
                }

                if (key_value == "RETURN")
                {
                    if (key_marks_clips_copy()) return;
                }

                if (key_value == "h")
                {
                    if (key_marks_clips_groups()) return;
                }
            }
            return;
        }


        // Accessing pinned clips
        //
        if (pinnedDialogVisible)
        {
            if (key_value == "j" || key_value == "DOWN")
            {
                if (key_pin_down()) return;
            }

            if (key_value == "k" || key_value == "UP")
            {
                if (key_pin_up()) return;
            }

            if (key_value == "g")
            {
                if (key_pin_top()) return;
            }

            if (key_value == "G")
            {
                if (key_pin_bottom()) return;
            }

            if (key_value == "D")
            {
                if (key_pin_delete()) return;
            }

            if (key_value == "RETURN")
            {
                if (key_pin_copy()) return;
            }

            return;
        }


        // Adding the current clip to a bookmark group
        //
        if (addToBookmarkDialogVisible)
        {
            // Add to bookmark dialog is visible - handle dialog-specific keys
            if (filterAddBookmarksMode)
            {
                if (key_value == "RETURN")
                {
                    if (key_addmarks_add()) return;
                }
                if (key_value == "ESCAPE")
                {
                    filterAddBookmarksMode = false;
                    filterAddBookmarksText.clear();
                    drawConsole();
                    return;
                }
                if (key_value == "BACKSPACE")
                {
                    if (!filterAddBookmarksText.empty())
                    {
                        filterAddBookmarksText.pop_back();
                        drawConsole();
                    }
                    return;
                }
                // Text input for filter
#ifdef _WIN32
                char typedChar = getCharFromMsg(msg);
                if (typedChar != 0)
                {
                    filterAddBookmarksText += typedChar;
                    drawConsole();
                }
#else
                char buffer[10];
                int count = XLookupString(keyEvent, buffer, sizeof(buffer), nullptr, nullptr);
                if (count > 0)
                {
                    filterAddBookmarksText += std::string(buffer, count);
                    drawConsole();
                }
#endif
                return;
            }

            if (key_value == "/")
            {
                filterAddBookmarksMode = true;
                filterAddBookmarksText.clear();
                drawConsole();
                return;
            }

            if (key_value == "RETURN")
            {
                if (key_addmarks_add()) return;
            }

            if (key_value == "j" || key_value == "DOWN")
            {
                if (key_addmarks_down()) return;
            }

            if (key_value == "k" || key_value == "UP")
            {
                if (key_addmarks_up()) return;
            }

            if (key_value == "g")
            {
                if (key_addmarks_top()) return;
            }

            if (key_value == "G")
            {
                if (key_addmarks_bottom()) return;
            }

            return;
        }


        // Filter mode
        //
        if (filterMode)
        {
            if (key_value == "BACKSPACE")
            {
                // Remove last character from filter
                if (!filterText.empty())
                {
                    filterText.pop_back();
                    updateFilteredItems();
                    selectedItem = 0;
                    drawConsole();
                }
                return;
            }

            if (key_value == "DELETE")
            {
                if (key_filter_delete()) return;
            }

            if (key_value == "RETURN")
            {
                if (key_filter_copy()) return;
            }

            if (key_value == "DOWN")
            {
                if (key_filter_down()) return;
            }

            if (key_value == "UP")
            {
                if (key_filter_up()) return;
            }

            
            // Free Text
#ifdef _WIN32
                // Handle character input from WM_KEYDOWN
                char typedChar = getCharFromMsg(msg); 
                if (typedChar != 0)
                {
                    bookmarkDialogInput += typedChar;  
                    // Don't add the triggering '/' as the first character
                    if (filterText.empty() && msg->wParam == '/')
                    {
                        // do nothing
                    }
                    else
                    {
                        filterText += typedChar;
                        updateFilteredItems();
                        selectedItem = 0;
                        drawConsole();
                    }
                }
#else
                // Original Linux part
                char buffer[10];
                int count = XLookupString(keyEvent, buffer, sizeof(buffer), nullptr, nullptr);
                filterText += std::string(buffer, count);
                updateFilteredItems();
                selectedItem = 0;
                drawConsole();
#endif
            // End Free Text

            return;
        }

        // Command mode
        //
        if (commandMode)
        {
            if (key_value == "BACKSPACE")
            {
                if (!commandText.empty()) {
                    commandText.pop_back();
                    drawConsole();
                }
                return;
            }

            if (key_value == "RETURN")
            {
                if (key_command_execute()) return;
            }

            if (key_value == "DOWN")
            {
                if (key_command_down()) return;
            }

            if (key_value == "UP")
            {
                if (key_command_up()) return;
            }

            if (key_value == "SPACE")
            {
                if (key_command_detect()) return;
            }

            // Free Text
#ifdef _WIN32
                // Handle character input from WM_KEYDOWN
                char typedChar = getCharFromMsg(msg); 
                if (typedChar != 0)
                {
                    bookmarkDialogInput += typedChar;  
                    // Don't add the triggering ':' as the first character
                    if (commandText.empty() && msg->wParam == ':')
                    {
                        // do nothing
                    }
                    else
                    {
                        commandText += typedChar;
                        drawConsole();
                    }
                }
#else
                // Original Linux part
                char buffer[10];
                int count = XLookupString(keyEvent, buffer, sizeof(buffer), nullptr, nullptr);
                commandText += std::string(buffer, count);
                drawConsole();
#endif
            // End Free Text

            return;
        }

        // Theme selection mode
        //
        if (cmd_themeSelectMode)
        {
            if (key_value == "ESCAPE")
            {
                if (key_theme_cancel()) return;
            }

            if (key_value == "RETURN")
            {
                if (key_theme_apply()) return;
            }

            if (key_value == "j" || key_value == "DOWN")
            {
                if (key_theme_down()) return;
            }

            if (key_value == "k" || key_value == "UP")
            {
                if (key_theme_up()) return;
            }

            if (key_value == "g")
            {
                if (key_theme_top()) return;
            }

            if (key_value == "G")
            {
                if (key_theme_bottom()) return;
            }

            return;
        }

        // Config selection mode
        //
        if (cmd_configSelectMode)
        {
            if (key_value == "ESCAPE")
            {
                if (key_config_cancel()) return;
            }

            if (key_value == "RETURN")
            {
                if (key_config_select()) return;
            }

            if (key_value == "j" || key_value == "DOWN")
            {
                if (key_config_down()) return;
            }

            if (key_value == "k" || key_value == "UP")
            {
                if (key_config_up()) return;
            }

            if (key_value == "g")
            {
                if (key_config_top()) return;
            }

            if (key_value == "G")
            {
                if (key_config_bottom()) return;
            }

            return;
        }


        // General keys - main clips list
        //
        if (key_value == "j" || key_value == "DOWN")
        {
            if (key_main_down()) return;
        }

        if (key_value == "k" || key_value == "UP")
        {
            if (key_main_up()) return;
        }

        if (key_value == "g")
        {
            if (key_main_top()) return;
        }

        if (key_value == "G")
        {
            if (key_main_bottom()) return;
        }

        if (key_value == "D")
        {
            if (key_main_delete()) return;
        }

        if (key_value == "/")
        {
            if (key_main_filter_start()) return;
        }

        if (key_value == ":")
        {
            if (key_main_command_start()) return;
        }

        if (key_value == "RETURN")
        {
            if (key_main_copy()) return;
        }

        if (key_value == "M")
        {
            if (key_main_addgroup_start()) return;
        }

        if (key_value == "m")
        {
            if (key_main_addclip_start()) return;
        }

        if (key_value == "?")
        {
            if (key_main_help_start()) return;
        }

        if (key_value == "`")
        {
            if (key_main_accessmarks_start()) return;
        }

        if (key_value == "p")
        {
            if (key_main_pin_clip()) return;
        }

        if (key_value == "i")
        {
            if (key_main_edit_start()) return;
        }

        // Pinned clips dialog
        if (key_value == "'")
        {
            if (key_main_pins_start()) return;
        }
    }


    //// Key Press Methods /////////////////////////////////////////////////////
        bool key_global_escape()
        {
            if (editDialogVisible)
            {
                if (key_edit_escape()) return true;
            }
            if (filterBookmarksMode)
            {
                filterBookmarksMode = false;
                filterBookmarksText.clear();
                drawConsole();
                return true;
            }
            if (filterAddBookmarksMode)
            {
                filterAddBookmarksMode = false;
                filterAddBookmarksText.clear();
                drawConsole();
                return true;
            }
            if (filterBookmarkClipsMode)
            {
                filterBookmarkClipsMode = false;
                filterBookmarkClipsText.clear();
                drawConsole();
                return true;
            }
            if (pinnedDialogVisible)
            {
                pinnedDialogVisible = false;
                drawConsole();
            }
            else if (bookmarkDialogVisible)
            {
                // Escape hides dialog but not window
                bookmarkDialogVisible = false;
                drawConsole();
            }
            else if (addToBookmarkDialogVisible)
            {
                // Escape hides dialog but not window
                addToBookmarkDialogVisible = false;
                drawConsole();
            }
            else if (helpDialogVisible)
            {
                // Escape hides help dialog but not window
                if (helpFilterMode)
                {
                    helpFilterMode = false;
                    helpFilterText.clear();
                    helpDialogScrollOffset = 0;
                    drawConsole();
                    return true;
                }
                helpDialogVisible = false;
                helpDialogScrollOffset = 0;
                drawConsole();
            }
            else if (viewBookmarksDialogVisible)
            {
                // Escape hides view bookmarks dialog but not window
                viewBookmarksDialogVisible = false;
                drawConsole();
            }
            else if (filterMode)
            {
                // Escape exits filter mode but doesn't hide window
                filterMode = false;
                filterText = "";
                filteredItems.clear();
                selectedItem = 0;
                drawConsole();
            }
            else if (commandMode)
            {
                // Escape exits command mode but doesn't hide window
                commandMode = false;
                commandText = "";
                selectedItem = 0;
                drawConsole();
            }
            else if (cmd_themeSelectMode)
            {
                // Restore original theme and exit theme selection mode but doesn't hide window
                if (!config.originalTheme.empty())
                {
                    config.switchTheme(config.originalTheme);
                }
                cmd_themeSelectMode = false;
                availableThemes.clear();
                config.originalTheme.clear();
                selectedItem = 0;
                drawConsole();
            }
            else if (cmd_configSelectMode)
            {
                // Exit config selection mode and return to command mode
                cmd_configSelectMode = false;
                commandMode = true;
                commandText = "";
                availableConfigs.clear();
                selectedItem = 0;
                drawConsole();
            }
            else
            {
                // Normal escape behavior - hide window
                hideWindow();
            }

            return true;
        }

        bool key_edit_escape()
        {
            editDialogVisible = false;
            drawConsole();
            return true;
        }

        bool key_edit_save()
        {
            if (!editDialogInput.empty())
            {
                // Create a new ClipboardItem from the edited content
                ClipboardItem newItem(editDialogInput);

                // Insert the new item at the beginning of the items vector
                items.insert(items.begin(), newItem);

                // Save to file with updated content
                saveToFile();

                std::cout << "Clip edited and saved as new item.\n";
            }
            editDialogVisible = false;
            drawConsole();
            return true;
        }

        bool key_edit_backspace()
        {
            std::string& text = editDialogInput;
            size_t& line_num = editDialogCursorLine;
            size_t& char_pos = editDialogCursorPos;

            if (char_pos > 0)
            {
                // Find the position of the character to delete
                size_t deletion_pos = 0;
                std::istringstream iss(text);
                std::string current_line;
                for (size_t i = 0; i < line_num; ++i)
                {
                    std::getline(iss, current_line);
                    deletion_pos += current_line.length() + 1; // +1 for newline
                }
                deletion_pos += char_pos -1;
                text.erase(deletion_pos, 1);
                char_pos--;

            }
            else if (line_num > 0)
            {
                // Find the end of the previous line
                size_t prev_line_end = 0;
                std::istringstream iss(text);
                std::string current_line;
                for (size_t i = 0; i < line_num -1; ++i)
                {
                    std::getline(iss, current_line);
                    prev_line_end += current_line.length() + 1;
                }
                std::getline(iss, current_line);
                size_t prev_line_len = current_line.length();

                // Find the start of the current line
                size_t current_line_start = prev_line_end + prev_line_len +1;
                
                // Erase the newline character
                text.erase(current_line_start -1, 1);

                line_num--;
                char_pos = prev_line_len;
            }

            updateEditDialogScrollOffset();
            drawConsole();
            return true;
        }

        bool key_edit_delete()
        {
            std::string& text = editDialogInput;
            size_t& line_num = editDialogCursorLine;
            size_t& char_pos = editDialogCursorPos;

            std::istringstream iss(text);
            std::string current_line;
            size_t current_line_index = 0;
            size_t deletion_pos = 0;

            while (current_line_index <= line_num && std::getline(iss, current_line))
            {
                if (current_line_index < line_num)
                {
                    deletion_pos += current_line.length() + 1; // +1 for newline
                }
                current_line_index++;
            }

            deletion_pos += char_pos;
            if (deletion_pos < text.length())
            {
                text.erase(deletion_pos, 1);
                updateEditDialogScrollOffset();
                drawConsole();
            }
            return true;
        }

        void key_edit_add_char(char c)
        {
            std::string& text = editDialogInput;
            size_t& line_num = editDialogCursorLine;
            size_t& char_pos = editDialogCursorPos;

            std::istringstream iss(text);
            std::string current_line;
            size_t current_line_index = 0;
            size_t insertion_pos = 0;

            while (current_line_index <= line_num && std::getline(iss, current_line))
            {
                if (current_line_index < line_num)
                {
                    insertion_pos += current_line.length() + 1; // +1 for newline
                }
                current_line_index++;
            }

            insertion_pos += char_pos;
            text.insert(insertion_pos, 1, c);
            char_pos++;
            updateEditDialogScrollOffset();
            drawConsole();
        }

        bool key_edit_add_newline()
        {
            key_edit_add_char('\n');
            editDialogCursorLine++;
            editDialogCursorPos = 0; // Ensure cursor is at the beginning of the new line
            updateEditDialogScrollOffset();
            return true;
        }

        bool key_edit_scroll_up()
        {
            if (editDialogScrollOffset > 0)
            {
                editDialogScrollOffset--;
                drawConsole();
            }
            return true;
        }

        bool key_edit_scroll_down()
        {
            // Need to calculate max scroll offset based on content and dialog height
            DialogDimensions dims = calculateDialogDimensions(windowWidth, windowHeight, 600, 400);
            const int lineHeight = 15;
            const int charWidth = 8;
            int maxCharsPerLine = (dims.width - 50) / charWidth;
            if (maxCharsPerLine < 1) maxCharsPerLine = 1;

            int totalVisualLines = 0;
            if (editDialogInput.empty())
            {
                totalVisualLines = 1;
            }
            else
            {
                std::istringstream iss(editDialogInput);
                std::string logicalLine;
                while (std::getline(iss, logicalLine))
                {
                    if (logicalLine.empty())
                    {
                        totalVisualLines++;
                    }
                    else
                    {
                        totalVisualLines += (logicalLine.length() + maxCharsPerLine - 1) / maxCharsPerLine;
                    }
                }
                if (editDialogInput.back() == '\n')
                {
                    totalVisualLines++;
                }
            }

            int maxVisibleLines = (dims.height - 70) / lineHeight;
            
            if (editDialogScrollOffset < totalVisualLines - maxVisibleLines)
            {
                editDialogScrollOffset++;
                drawConsole();
            }
            return true;
        }

        // Help
        bool key_help_hide()
        {
            helpDialogVisible = false;
            drawConsole();
            return true;
        }

        bool key_help_scroll_down()
        {
            updateHelpDialogScrollOffset(-1);
            drawConsole();
            return true;
        }

        bool key_help_scroll_up()
        {
            if (helpDialogScrollOffset > 0)
            {
                updateHelpDialogScrollOffset(1);
                drawConsole();
            }
            return true;
        }

        bool key_help_scroll_top()
        {
            helpDialogScrollOffset = 0;
            drawConsole();
            return true;
        }

        // Add Groups
        bool key_addgroup_add()
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

        bool key_addgroup_back()
        {
            if (!bookmarkDialogInput.empty())
            {
                bookmarkDialogInput.pop_back();
                drawConsole();
            }
            return true;
        }

        // Bookmarks
        bool key_marks_show()
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

        bool key_marks_groups_down()
        {
            if (selectedViewBookmarkGroup < bookmarkGroups.size() - 1)
            {
                selectedViewBookmarkGroup++;
                updateScrollOffset();
                drawConsole();
            }
            return true;
        }

        bool key_marks_groups_up()
        {
            if (selectedViewBookmarkGroup > 0)
            {
                selectedViewBookmarkGroup--;
                updateScrollOffset();
                drawConsole();
            }
            return true;
        }

        bool key_marks_groups_top()
        {
            selectedViewBookmarkGroup = 0;
            viewBookmarksScrollOffset = 0;
            drawConsole();
            return true;
        }

        bool key_marks_groups_bottom()
        {
            selectedViewBookmarkGroup = bookmarkGroups.size() - 1;
            updateScrollOffset();
            drawConsole();
            return true;
        }

        bool key_marks_groups_delete()
        {
            if (selectedViewBookmarkGroup < bookmarkGroups.size())
            {
                std::string groupToDelete = bookmarkGroups[selectedViewBookmarkGroup];
                
                // Remove group from list
                bookmarkGroups.erase(bookmarkGroups.begin() + selectedViewBookmarkGroup);
                saveBookmarkGroups();
                
                // Delete bookmark file
                std::string bookmarkFile = config.bookmarksDir + "/bookmarks_" + groupToDelete + ".txt";
                unlink(bookmarkFile.c_str());
                
                std::cout << "Deleted bookmark group and all clips: " << groupToDelete << "\n";
                
                // Adjust selection
                if (selectedViewBookmarkGroup > 0 && selectedViewBookmarkGroup >= bookmarkGroups.size())
                {
                    selectedViewBookmarkGroup = bookmarkGroups.size() - 1;
                }
                
                // Close dialog if no groups left
                if (bookmarkGroups.empty())
                {
                    viewBookmarksDialogVisible = false;
                }
                
                drawConsole();
            }
            return true;
        }

        bool key_marks_groups_clips()
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


        bool key_marks_clips_down()
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

        bool key_marks_clips_up()
        {
            if (selectedViewBookmarkItem > 0)
            {
                selectedViewBookmarkItem--;
                updateScrollOffset();
                drawConsole();
            }
            return true;
        }

        bool key_marks_clips_top()
        {
            selectedViewBookmarkItem = 0;
            viewBookmarksScrollOffset = 0;
            drawConsole();
            return true;
        }

        bool key_marks_clips_bottom()
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

        bool key_marks_clips_delete()
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

        bool key_marks_clips_copy()
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

        bool key_marks_clips_groups()
        {
            viewBookmarksShowingGroups = true;
            drawConsole();
            return true;
        }

        // Pinned Clips
        bool key_pin_down()
        {
            auto sortedItems = getSortedPinnedItems(config.pinnedFile);
            if (selectedViewPinnedItem < sortedItems.size() - 1)
            {
                selectedViewPinnedItem++;
                updatePinnedScrollOffset();
                drawConsole();
            }
            return true;
        }

        bool key_pin_up()
        {
            if (selectedViewPinnedItem > 0)
            {
                selectedViewPinnedItem--;
                updatePinnedScrollOffset();
                drawConsole();
            }
            return true;
        }

        bool key_pin_top()
        {
            selectedViewPinnedItem = 0;
            viewPinnedScrollOffset = 0;
            drawConsole();
            return true;
        }

        bool key_pin_bottom()
        {
            auto sortedItems = getSortedPinnedItems(config.pinnedFile);
            if (!sortedItems.empty())
            {
                selectedViewPinnedItem = sortedItems.size() - 1;
                updatePinnedScrollOffset();
                drawConsole();
            }
            return true;
        }

        bool key_pin_delete()
        {
            auto sortedItems = getSortedPinnedItems(config.pinnedFile);
            
            // Remove the selected item if valid
            if (selectedViewPinnedItem < sortedItems.size())
            {
                sortedItems.erase(sortedItems.begin() + selectedViewPinnedItem);

                // Write back remaining lines
                std::ofstream outFile(config.pinnedFile);
                if (outFile.is_open())
                {
                    for (const auto& item : sortedItems)
                    {
                        outFile << item << "\n";
                    }
                    outFile.close();
                    
                    std::cout << "Deleted pinned clip\n";
                    
                    // Adjust selection
                    if (selectedViewPinnedItem > 0 && selectedViewPinnedItem >= sortedItems.size())
                    {
                        selectedViewPinnedItem = sortedItems.size() - 1;
                    }
                    
                    // Close dialog if no pinned clips left
                    if (sortedItems.empty())
                    {
                        pinnedDialogVisible = false;
                    }
                    
                    drawConsole();
                }
            }
            return true;
        }

        bool key_pin_copy()
        {
            auto sortedItems = getSortedPinnedItems(config.pinnedFile);

            if (selectedViewPinnedItem < sortedItems.size())
            {
                std::string selectedLine = sortedItems[selectedViewPinnedItem];
                size_t pos = selectedLine.find('|');
                if (pos != std::string::npos)
                {
                    std::string contentToCopy;
                    std::string contentToSave = selectedLine.substr(pos + 1);
                    try
                    {
                        contentToCopy = decrypt(contentToSave, config);
                    }
                    catch (...)
                    {
                        contentToCopy = contentToSave;
                    }

                    copyToClipboard(contentToCopy);

                    int lineCount = countLines(contentToCopy);
                    if (lineCount > 1)
                    {
                        std::cout << "Copied " << lineCount << " lines from pinned clips\n";
                    }
                    else
                    {
                        std::cout << "Copied from pinned clips: " << contentToCopy.substr(0, 50) << "...\n";
                    }

                    // Remove the old line from sorted items
                    sortedItems.erase(sortedItems.begin() + selectedViewPinnedItem);

                    // Add the new line at the beginning
                    auto newTimestamp = std::chrono::system_clock::now().time_since_epoch().count();
                    std::string newLine = std::to_string(newTimestamp) + "|" + contentToSave;
                    sortedItems.insert(sortedItems.begin(), newLine);
                    
                    // Write the updated lines back to the file
                    std::ofstream outFile(config.pinnedFile);
                    for (const auto& l : sortedItems)
                    {
                        outFile << l << std::endl;
                    }
                    outFile.close();
                }

                pinnedDialogVisible = false;
                hideWindow();
            }
            return true;
        }

        // Add Bookmarks
        bool key_addmarks_add() {
            std::vector<std::string> displayedGroups;
            if (filterAddBookmarksMode) {
                for (const auto& group : bookmarkGroups) {
                    if (group.find(filterAddBookmarksText) != std::string::npos) {
                        displayedGroups.push_back(group);
                    }
                }
            } else {
                displayedGroups = bookmarkGroups;
            }

            if (!displayedGroups.empty() && selectedAddBookmarkGroup < displayedGroups.size()) {
                std::string selectedGroup = displayedGroups[selectedAddBookmarkGroup];
                
                // Check if current clip is already in this group
                if (!items.empty() && selectedItem < getDisplayItemCount()) {
                    size_t actualIndex = getActualItemIndex(selectedItem);
                    std::string clipContent = items[actualIndex].content;
                    
                    // Read existing bookmarks in this group
                    std::string bookmarkFile = config.bookmarksDir + "/bookmarks_" + selectedGroup + ".txt";
                    std::ifstream file(bookmarkFile);
                    std::string line;
                    bool alreadyExists = false;
                    
                    while (std::getline(file, line)) {
                        std::string decrypted = decrypt(line, config);
                        if (decrypted == clipContent) {
                            alreadyExists = true;
                            break;
                        }
                    }
                    file.close();
                    
                    if (!alreadyExists) {
                        addClipToBookmarkGroup(selectedGroup, clipContent);
                        std::cout << "Added clip to bookmark group: " << selectedGroup << "\n";
                    } else {
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

        bool key_addmarks_down() {
            if (!bookmarkGroups.empty() && selectedAddBookmarkGroup < bookmarkGroups.size() - 1) {
                selectedAddBookmarkGroup++;
                updateAddBookmarkScrollOffset();
                drawConsole();
            }
            return true;
        }

        bool key_addmarks_up() {
            if (selectedAddBookmarkGroup > 0) {
                selectedAddBookmarkGroup--;
                updateAddBookmarkScrollOffset();
                drawConsole();
            }
            return true;
        }

        bool key_addmarks_top() {
            selectedAddBookmarkGroup = 0;
            addBookmarkScrollOffset = 0;
            drawConsole();
            return true;
        }

        bool key_addmarks_bottom() {
            if (!bookmarkGroups.empty()) {
                selectedAddBookmarkGroup = bookmarkGroups.size() - 1;
                updateAddBookmarkScrollOffset();
                drawConsole();
            }
            return true;
        }

        // Filter Mode
        bool key_filter_delete() {
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
            return true;
        }

        bool key_filter_copy() {
            if (!items.empty() && selectedItem < getDisplayItemCount()) {
                size_t actualIndex = getActualItemIndex(selectedItem);
                copyToClipboard(items[actualIndex].content);
                int lines = countLines(items[actualIndex].content);
                if (lines > 1) {
                    std::cout << "Copied " << lines << " lines to clipboard" << "\n";
                } else {
                    std::cout << "Copied to clipboard: " << items[actualIndex].content.substr(0, 50) << "...\n";
                }
                filterMode = false;
                filterText = "";
                filteredItems.clear();
                hideWindow();
            }
            return true;
        }

        bool key_filter_down() {
            size_t displayCount = getDisplayItemCount();
            if (selectedItem < displayCount - 1) {
                selectedItem++;
                updateConsoleScrollOffset();
                drawConsole();
            }
            return true;
        }

        bool key_filter_up() {
            if (selectedItem > 0) {
                selectedItem--;
                updateConsoleScrollOffset();
                drawConsole();
            }
            return true;
        }

        // Command
        bool key_command_execute() {
            if (!commandText.empty()) {
                executeCommand(commandText);
            }
            commandMode = false;
            commandText = "";
            drawConsole();
            return true;
        }

        bool key_command_down() {
            size_t displayCount = getDisplayItemCount();
            if (selectedItem < displayCount - 1) {
                selectedItem++;
                updateConsoleScrollOffset();
                drawConsole();
            }
            return true;
        }

        bool key_command_up() {
            if (selectedItem > 0) {
                selectedItem--;
                updateConsoleScrollOffset();
                drawConsole();
            }
            return true;
        }

        bool key_command_detect() {
            if (commandText == "theme") {
                // Enter theme selection mode
                commandMode = false;
                cmd_themeSelectMode = true;
                availableThemes = config.discoverThemes();
                // Store original theme and apply first theme for preview
                if (!availableThemes.empty()) {
                    config.originalTheme = config.theme;
                    selectedTheme = 0;
                    config.switchTheme(availableThemes[0]);
                }
                drawConsole();
                return true;
            }
            if (commandText == "config") {
                // Enter config selection mode
                commandMode = false;
                cmd_configSelectMode = true;
                availableConfigs = config.discoverConfigs();
                drawConsole();
                return true;
            }

            // Add space to command text for other commands
            commandText += " ";
            drawConsole();

            return true;
        }

        // Theme Command
        bool key_theme_cancel() {
            if (!config.originalTheme.empty()) {
                config.switchTheme(config.originalTheme);
            }
            cmd_themeSelectMode = false;
            availableThemes.clear();
            config.originalTheme.clear();
            drawConsole();

            return true;
        }

        bool key_theme_apply() {
            if (selectedTheme < availableThemes.size()) {
                config.switchTheme(availableThemes[selectedTheme]);
                // Save to config
                config.saveConfig();
            }
            cmd_themeSelectMode = false;
            availableThemes.clear();
            config.originalTheme.clear();
            drawConsole();

            return true;
        }

        bool key_theme_down() {
            if (selectedTheme < availableThemes.size() - 1) {
                selectedTheme++;
                updateThemeSelectScrollOffset();
                // Apply live preview
                if (selectedTheme < availableThemes.size()) {
                    config.switchTheme(availableThemes[selectedTheme]);
                }
                drawConsole();
            }
            return true;
        }

        bool key_theme_up() {
            if (selectedTheme > 0) {
                selectedTheme--;
                updateThemeSelectScrollOffset();
                // Apply live preview
                if (selectedTheme < availableThemes.size()) {
                    config.switchTheme(availableThemes[selectedTheme]);
                }
                drawConsole();
            }
            return true;
        }

        bool key_theme_top() {
            selectedTheme = 0;
            themeSelectScrollOffset = 0;
            drawConsole();

            return true;
        }

        bool key_theme_bottom() {
            if (!availableThemes.empty()) {
                selectedTheme = availableThemes.size() - 1;
                updateThemeSelectScrollOffset();
                drawConsole();
            }
            return true;
        }

        // Config Command
        bool key_config_cancel() {
            cmd_configSelectMode = false;
            commandMode = true;
            commandText = "";
            availableConfigs.clear();
            selectedItem = 0;
            drawConsole();
            return true;
        }

        bool key_config_select() {
            if (selectedConfig < availableConfigs.size()) {
                std::string configKey = availableConfigs[selectedConfig];
                std::string currentValue = config.getConfigValue(configKey);
                cmd_configSelectMode = false;
                commandText = "config " + configKey + " " + currentValue;
                commandMode = true;
                availableConfigs.clear();
                drawConsole();
            }
            return true;
        }

        bool key_config_down() {
            if (selectedConfig < availableConfigs.size() - 1) {
                selectedConfig++;
                updateConfigSelectScrollOffset();
                drawConsole();
            }
            return true;
        }

        bool key_config_up() {
            if (selectedConfig > 0) {
                selectedConfig--;
                updateConfigSelectScrollOffset();
                drawConsole();
            }
            return true;
        }

        bool key_config_top() {
            selectedConfig = 0;
            configSelectScrollOffset = 0;
            drawConsole();
            return true;
        }

        bool key_config_bottom() {
            if (!availableConfigs.empty()) {
                selectedConfig = availableConfigs.size() - 1;
                updateConfigSelectScrollOffset();
                drawConsole();
            }
            return true;
        }

        // Main Clips List
        bool key_main_down() {
            size_t displayCount = getDisplayItemCount();
            if (selectedItem < displayCount - 1) {
                selectedItem++;
                updateConsoleScrollOffset();
                drawConsole();
            }
            return true;
        }

        bool key_main_up() {
            if (selectedItem > 0) {
                selectedItem--;
                updateConsoleScrollOffset();
                drawConsole();
            }
            return true;
        }

        bool key_main_top() {
            selectedItem = 0;
            updateConsoleScrollOffset();
            drawConsole();

            return true;
        }

        bool key_main_bottom() {
            size_t displayCount = getDisplayItemCount();
            if (displayCount > 0) {
                selectedItem = displayCount - 1;
                updateConsoleScrollOffset();
                drawConsole();
            }
            return true;
        }

        bool key_main_delete() {
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
            return true;
        }

        bool key_main_filter_start() {
            filterMode = true;
            filterText = "";
            updateFilteredItems();
            selectedItem = 0;
            drawConsole();

            return true;
        }

        bool key_main_command_start() {
            commandMode = true;
            commandText = "";
            selectedItem = 0;
            drawConsole();

            return true;
        }

        bool key_main_copy() {
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

                    std::cout << "Clip moved to top after copying\n";
                }

                int lines = countLines(clipContent);

                if (lines > 1) {
                    std::cout << "Copied " << lines << " lines to clipboard\n";
                } else {
                    std::cout << "Copied to clipboard: " << clipContent.substr(0, 50) << "...\n";
                }
                hideWindow();
            }
            return true;
        }

        bool key_main_addgroup_start() {
            bookmarkDialogVisible = true;
            bookmarkDialogInput = "";
            selectedBookmarkGroup = 0;
            bookmarkMgmtScrollOffset = 0; // Reset scroll when opening
            drawConsole();

            return true;
        }

        bool key_main_addclip_start() {
            if (!bookmarkGroups.empty()) {
                addToBookmarkDialogVisible = true;
                selectedAddBookmarkGroup = 0;
                addBookmarkScrollOffset = 0; // Reset scroll when opening
                drawConsole();
            }
            return true;
        }

        bool key_main_help_start() {
            helpDialogVisible = true;
            helpDialogScrollOffset = 0;
            helpFilterMode = true;
            helpFilterText.clear();
            drawConsole();

            return true;
        }

        bool key_main_accessmarks_start() {
            if (!bookmarkGroups.empty()) {
                viewBookmarksDialogVisible = true;
                viewBookmarksShowingGroups = true; // Start with group selection
                selectedViewBookmarkGroup = 0;
                selectedViewBookmarkItem = 0;
                viewBookmarksScrollOffset = 0; // Reset scroll when opening
                drawConsole();
            }
            return true;
        }

        bool key_main_pin_clip() {
            if (!items.empty() && selectedItem < getDisplayItemCount()) {
                size_t actualIndex = getActualItemIndex(selectedItem);
                std::string clipContent = items[actualIndex].content;
                
                // Read existing bookmarks in this group
                std::ifstream file(config.pinnedFile);
                std::string line;
                bool alreadyExists = false;
                
                while (std::getline(file, line)) {
                    std::string decrypted = decrypt(line, config);
                    if (decrypted == clipContent) {
                        alreadyExists = true;
                        break;
                    }
                }
                file.close();
                
                if (!alreadyExists) {
                    addClipToPinned(clipContent);
                    std::cout << "Added clip to pinned\n";
                } else {
                    std::cout << "Clip is already pinned\n";
                }
            }
            return true;
        }

        bool key_main_pins_start() {
            pinnedDialogVisible = true;
            selectedViewPinnedItem = 0;
            viewPinnedScrollOffset = 0; // Reset scroll when opening
            drawConsole();

            return true;
        }

        bool key_main_edit_start() {
            if (!items.empty() && selectedItem < getDisplayItemCount()) {
                size_t actualIndex = getActualItemIndex(selectedItem);
                editDialogInput = items[actualIndex].content;
                editDialogVisible = true;
                editDialogScrollOffset = 0;

                // Initialize cursor position
                editDialogCursorLine = 0;
                editDialogCursorPos = 0;
                std::string lastLine;
                for (char c : editDialogInput) {
                    if (c == '\n') {
                        editDialogCursorLine++;
                        lastLine.clear();
                    } else {
                        lastLine += c;
                    }
                }
                editDialogCursorPos = lastLine.length();

                updateEditDialogScrollOffset();
                drawConsole();
            }
            return true;
        }

        bool key_edit_cursor_left() {
            if (editDialogCursorPos > 0) {
                editDialogCursorPos--;
                drawConsole();
            }
            return true;
        }

        bool key_edit_cursor_right() {
            std::string currentLine = "";
            std::istringstream iss(editDialogInput);
            for (size_t i = 0; i <= editDialogCursorLine; ++i) {
                std::getline(iss, currentLine);
            }

            if (editDialogCursorPos < currentLine.length()) {
                editDialogCursorPos++;
                drawConsole();
            }
            return true;
        }

        bool key_edit_cursor_up() {
            if (editDialogCursorLine > 0) {
                editDialogCursorLine--;
                std::string currentLine = "";
                std::istringstream iss(editDialogInput);
                for (size_t i = 0; i <= editDialogCursorLine; ++i) {
                    std::getline(iss, currentLine);
                }
                if (editDialogCursorPos > currentLine.length()) {
                    editDialogCursorPos = currentLine.length();
                }
                updateEditDialogScrollOffset();
                drawConsole();
            }
            return true;
        }

        bool key_edit_cursor_down() {
            size_t totalLines = 1;
            for (char c : editDialogInput) {
                if (c == '\n') totalLines++;
            }
            if (editDialogCursorLine < totalLines - 1) {
                editDialogCursorLine++;
                std::string currentLine = "";
                std::istringstream iss(editDialogInput);
                for (size_t i = 0; i <= editDialogCursorLine; ++i) {
                    std::getline(iss, currentLine);
                }
                if (editDialogCursorPos > currentLine.length()) {
                    editDialogCursorPos = currentLine.length();
                }
                updateEditDialogScrollOffset();
                drawConsole();
            }
            return true;
        }

        bool key_edit_home() {
            editDialogCursorPos = 0;
            updateEditDialogScrollOffset();
            drawConsole();
            return true;
        }

        bool key_edit_end() {
            std::string currentLine = "";
            std::istringstream iss(editDialogInput);
            for (size_t i = 0; i <= editDialogCursorLine; ++i) {
                std::getline(iss, currentLine);
            }
            editDialogCursorPos = currentLine.length();
            updateEditDialogScrollOffset();
            drawConsole();
            return true;
        }
    //// End Key Press Methods /////////////////////////////////////////////////


///
///
//// END KEY HANDLING //////////////////////////////////////////////////////////





    // Dynamic window and layout management functions
    void updateWindowDimensions(int newWidth, int newHeight) {

#ifdef __linux__
        // Enforce minimum window size constraints
        if (newWidth < MIN_WINDOW_WIDTH) {
            newWidth = MIN_WINDOW_WIDTH;
        }
        if (newHeight < MIN_WINDOW_HEIGHT) {
            newHeight = MIN_WINDOW_HEIGHT;
        }
#endif
        
        
        // if (debugging) logfile << "updateWindowDimensions (internal): newWidth=" << newWidth << ", newHeight=" << newHeight << " -> windowWidth=" << windowWidth << ", windowHeight=" << windowHeight << std::endl;
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
        config.setupConfigDir();
        config.loadConfig();
        config.loadTheme();
#ifdef __linux__
        if (gc) {
            XSetForeground(display, gc, config.textColor);
        }
#endif
        loadFromFile();
        loadBookmarkGroups();


        #ifdef _WIN32
            std::cout << "Running on Windows.\n";
            // Add Windows-specific auto-start code here
        #elif __APPLE__
            std::cout << "Running on macOS.\n";
            // Add macOS-specific auto-start code here
        #elif __linux__
            std::string dir = std::string(getenv("HOME")) + "/.config/autostart";
            std::string filePath = dir + "/mmry.desktop";

            if (config.autoStart) {
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
                    std::cout << "Full path: " << result << "\n";
                } else {
                    std::cerr << "Error getting path" << "\n";
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
                    std::cout << "Autostart already disabled\n";
                } else {
                    std::cout << "Autostart disabled\n";
                }
            }
        #else
            std::cout << "Unknown operating system.\n";
        #endif

        
        std::cout << "MMRY Clipboard Manager started\n";
        std::cout << "Config directory: " << config.configDir << "\n";
        std::cout << "Press Ctrl+Alt+C to show window, Escape to hide\n";
        std::cout << "Press Shift+Q in window to quit application\n";
        std::cout << "Press Ctrl+C in terminal to exit\n";
        
#ifdef __linux__
        // --- Reliable X11 global hotkey setup for Ctrl+Alt+C ----------
        auto grab_global_hotkey = [&](Display* dpy, Window rootWin, KeySym keysym) {
            if (!dpy) {
                std::cout << "!dpy - returning\n";
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
                    std::cout << "Hotkey triggered: Ctrl+Alt+C\n";
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
        std::cout << "Windows: registering global hotkey Ctrl+Alt+C...\n" << std::endl;

        // Register Ctrl+Alt+C (ID: 1)
        if (!RegisterHotKey(NULL, 1, MOD_CONTROL | MOD_ALT, 'C')) {
            std::cerr << "Failed to register global hotkey." << std::endl;
        }

        // --- Win32 window class (blank window for now) ---
        WNDCLASS wc = {};
        wc.lpfnWndProc   = MMRYWndProc;
        wc.hInstance     = GetModuleHandle(NULL);
        wc.lpszClassName = "MMRY_Window_Class";
        wc.hbrBackground = NULL; // We'll handle background painting ourselves

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
                        800, 450,
                        NULL, NULL, GetModuleHandle(NULL), 
                        this); // Pass 'this' as lpParam
                    
                    if (hwnd) {
                        AddClipboardFormatListener(hwnd);

                        // Create and select a font
                        font = CreateFontA(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, 
                                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, 
                                           CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                                           FIXED_PITCH | FF_MODERN, "Courier New");

                        RECT clientRect;
                        GetClientRect(hwnd, &clientRect);
                        int actualClientWidth = clientRect.right - clientRect.left;
                        int actualClientHeight = clientRect.bottom - clientRect.top;
                        writeLog("run(): Before updateWindowDimensions. actualClientWidth=" + std::to_string(actualClientWidth) + ", actualClientHeight=" + std::to_string(actualClientHeight));
                        updateWindowDimensions(actualClientWidth, actualClientHeight);
                    }
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
    
#ifdef _WIN32
    HWND hwnd = nullptr;
    HFONT font = nullptr;

    char getCharFromMsg(MSG* msg) {
        // Get the scan code from lParam
        UINT scanCode = (msg->lParam >> 16) & 0xFF;

        // Get current keyboard state
        BYTE keyboardState[256];
        GetKeyboardState(keyboardState);

        // Convert virtual key to character
        char charBuffer[2]; // Needs space for null terminator
        int result = ToAscii(msg->wParam, scanCode, keyboardState, (LPWORD)charBuffer, 0);

        if (result == 1) {
            return charBuffer[0];
        }
        return 0; // Return null character if conversion fails
    }
#endif
    
    void updateFilteredBookmarkClips();
    size_t getBookmarkItemCount();
    
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
        selectedItem = 0;
        consoleScrollOffset = 0;
        filteredItems.clear();

        if (filterText.empty()) {
            for (size_t i = 0; i < items.size(); ++i) {
                filteredItems.push_back(i);
            }
        } else if (filterText[0] == '!') {
            // Explicit regex search (after '!' prefix)
            std::string regex_pattern = filterText.substr(1);
            if (!regex_pattern.empty()) {
                try {
                    std::regex rgx(regex_pattern, std::regex_constants::icase | std::regex_constants::multiline);

                    for (size_t i = 0; i < items.size(); ++i) {
                        if (!items[i].lowercase_content.empty() && 
                            std::regex_search(items[i].lowercase_content, rgx)) {
                            filteredItems.push_back(i);
                        }
                    }
                } catch (const std::regex_error& e) {
                    writeLog(std::string(__FUNCTION__) + std::string(e.what()));
                }
            }
        } else {
            // Fast path: simple substring search (most common case)
            if (filterText.find('*') == std::string::npos && 
                filterText.find('?') == std::string::npos &&
                filterText.find('.') == std::string::npos &&
                filterText.find('+') == std::string::npos) {
                
                std::string lower_filter = filterText;
                std::transform(lower_filter.begin(), lower_filter.end(), lower_filter.begin(),
                               [](unsigned char c){ return std::tolower(c); });

                for (size_t i = 0; i < items.size(); ++i) {
                    if (items[i].lowercase_content.find(lower_filter) != std::string::npos) {
                        filteredItems.push_back(i);
                    }
                }
            } else {
                // Slow path: regex search for wildcard patterns
                try {
                    std::string regex_str = wildcardToRegex(filterText);
                    std::regex rgx(regex_str, std::regex_constants::icase);
                    
                    for (size_t i = 0; i < items.size(); ++i) {
                        if (std::regex_search(items[i].content, rgx)) {
                            filteredItems.push_back(i);
                        }
                    }
                } catch (const std::regex_error& e) {
                    // Handle invalid regex patterns gracefully
                    writeLog("Regex error: " + std::string(e.what()));
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
        DialogDimensions dims = calculateDialogDimensions(windowWidth, windowHeight, 600, 500);
        const int ITEM_LINE_HEIGHT = 25; // Now uses LINE_HEIGHT
        
        // This 'y' is the starting point of the list within the dialog
        // This needs to match the y = dims.y + 60 in drawViewBookmarksDialog
        const int LIST_START_OFFSET_Y = 60; 

        // Calculate how many items can actually fit in the scrollable area
        // dims.contentHeight is the total content area. Subtract the space taken by the header.
        int availableHeightForScrollableItems = dims.contentHeight - LIST_START_OFFSET_Y;
        
        int dynamicVisibleItems = std::max(1, availableHeightForScrollableItems / ITEM_LINE_HEIGHT);

        if (viewBookmarksShowingGroups) {
            // Scrolling for groups
            if (selectedViewBookmarkGroup < viewBookmarksScrollOffset) {
                viewBookmarksScrollOffset = selectedViewBookmarkGroup;
            } else if (selectedViewBookmarkGroup >= viewBookmarksScrollOffset + dynamicVisibleItems) {
                viewBookmarksScrollOffset = selectedViewBookmarkGroup - dynamicVisibleItems + 1;
            }
        } else {
            // Scrolling for clips
            size_t currentItemCount = filterBookmarkClipsMode ? filteredBookmarkClips.size() : getBookmarkItemCount();
            
            if (selectedViewBookmarkItem < viewBookmarksScrollOffset) {
                viewBookmarksScrollOffset = selectedViewBookmarkItem;
            } else if (selectedViewBookmarkItem >= viewBookmarksScrollOffset + dynamicVisibleItems) {
                viewBookmarksScrollOffset = selectedViewBookmarkItem - dynamicVisibleItems + 1;
            }

            // Ensure scroll offset does not exceed available items
            if (currentItemCount == 0) {
                viewBookmarksScrollOffset = 0;
            } else if (viewBookmarksScrollOffset + dynamicVisibleItems > currentItemCount) {
                viewBookmarksScrollOffset = std::max(0, (int)currentItemCount - dynamicVisibleItems);
            }
        }
    }
    
    void updateEditDialogScrollOffset() {
        DialogDimensions dims = calculateDialogDimensions(windowWidth, windowHeight, 600, 400);
        const int lineHeight = 15;
        const int charWidth = 8;
        int maxCharsPerLine = (dims.width - 50) / charWidth;
        if (maxCharsPerLine < 1) maxCharsPerLine = 1;

        // Calculate the visual line number of the cursor
        int cursorVisualLine = 0;
        std::istringstream iss(editDialogInput);
        std::string logicalLine;
        for (int i = 0; i < (int)editDialogCursorLine; ++i) {
            if (!std::getline(iss, logicalLine)) break;
            if (logicalLine.empty()) {
                cursorVisualLine++;
            } else {
                cursorVisualLine += (logicalLine.length() + maxCharsPerLine - 1) / maxCharsPerLine;
            }
        }
        // Add visual lines from the current logical line, up to the cursor
        cursorVisualLine += editDialogCursorPos / maxCharsPerLine;

        // Calculate max visible lines
        int maxVisibleLines = (dims.height - 70 - 15) / lineHeight; // 70 for header/footer, 15 for some padding
        if (maxVisibleLines < 1) maxVisibleLines = 1;

        // Adjust scroll offset
        if (cursorVisualLine < editDialogScrollOffset) {
            editDialogScrollOffset = cursorVisualLine;
        } else if (cursorVisualLine >= editDialogScrollOffset + maxVisibleLines) {
            editDialogScrollOffset = cursorVisualLine - maxVisibleLines + 1;
        }

        // Clamp scroll offset
        if (editDialogScrollOffset < 0) {
            editDialogScrollOffset = 0;
        }
    }
    
    void updateConsoleScrollOffset() {
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
        if (static_cast<int>(displayCount) > maxVisibleItems) {
            availableHeight -= SCROLL_INDICATOR_HEIGHT;
            maxVisibleItems = availableHeight / LINE_HEIGHT;
        }

        if (maxVisibleItems > 0) maxVisibleItems += 1;
        
        // Ensure we show at least 1 item
        if (maxVisibleItems < 1) {
            maxVisibleItems = 1;
        }
        // logfile << "updateConsoleScrollOffset: displayCount=" << displayCount << ", maxVisibleItems (final)=" << maxVisibleItems << ", selectedItem=" << selectedItem << ", consoleScrollOffset=" << consoleScrollOffset << std::endl;
        
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
        if (selectedViewPinnedItem < viewPinnedScrollOffset) {
            viewPinnedScrollOffset = selectedViewPinnedItem;
        } else if (selectedViewPinnedItem >= viewPinnedScrollOffset + m_maxVisiblePinnedItems) {
            viewPinnedScrollOffset = selectedViewPinnedItem - m_maxVisiblePinnedItems + 1;
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
    
    void updateConfigSelectScrollOffset() {
        const int VISIBLE_ITEMS = 10; // Number of configs visible in config selection
        
        if (selectedConfig < configSelectScrollOffset) {
            configSelectScrollOffset = selectedConfig;
        } else if (selectedConfig >= configSelectScrollOffset + VISIBLE_ITEMS) {
            configSelectScrollOffset = selectedConfig - VISIBLE_ITEMS + 1;
        }
    }
    
    void updateHelpDialogScrollOffset(int adjustment) {
        const int STEP = 10;
       
        helpDialogScrollOffset = helpDialogScrollOffset + (adjustment * STEP);

        if (static_cast<int>(helpDialogScrollOffset) > -1)
        {
            helpDialogScrollOffset = 0;
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
    
    
    
    // Dialog positioning and sizing structure

    
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
                config.switchTheme(args);
            } else {
                // Enter theme selection mode: "theme"
                commandMode = false;
                cmd_themeSelectMode = true;
                availableThemes = config.discoverThemes();
                // Store original theme and apply first theme for preview
                if (!availableThemes.empty()) {
                    config.originalTheme = config.theme;
                    selectedTheme = 0;
                    config.switchTheme(availableThemes[0]);
                }
                drawConsole();
            }
            return;
        }
        
        if (cmd == "config") {
            if (!args.empty()) {
                std::cout << "DEBUG: Processing config command with args: '" << args << "'\n";
                // Parse "config key value" format
                std::istringstream configIss(args);
                std::string configKey, configValue;
                
                if (configIss >> configKey) {
                    std::string remaining;
                    std::getline(configIss, remaining);
                    // Trim leading whitespace from config value
                    if (!remaining.empty() && remaining[0] == ' ') {
                        remaining = remaining.substr(1);
                    }
                    configValue = remaining;
                    
                    std::cout << "DEBUG: Parsed configKey='" << configKey << "', configValue='" << configValue << "'\n";
                    
                        // Validate and update config based on type
                    if (config.updateConfigValue(configKey, configValue)) {
                        std::cout << "DEBUG: updateConfigValue returned true, calling saveConfig()\n";
                        config.saveConfig();
                        std::cout << "Updated " << configKey << " = " << configValue << "\n";
                    } else {
                        std::cout << "DEBUG: updateConfigValue returned false\n";
                        std::cout << "Invalid value for " << configKey << ". Expected type: " << config.getConfigType(configKey) << "\n";
                    }
                } else {
                    std::cout << "DEBUG: Failed to parse config key from args\n";
                }
            } else {
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
    
    void loadBookmarkGroups() {
        // Cross-platform path separator
#ifdef _WIN32
        const char pathSep = '\\';
#else
        const char pathSep = '/';
#endif
        
        std::string bookmarkFile = config.bookmarksDir + pathSep + "bookmarks.txt";
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
        std::string bookmarkFile = config.bookmarksDir + "/bookmarks.txt";
        std::ofstream file(bookmarkFile);
        
        if (file.is_open()) {
            for (const auto& group : bookmarkGroups) {
                file << group << "|0\n"; // Group name | clip count
            }
            file.close();
        }
    }
    
    void addClipToBookmarkGroup(const std::string& groupName, const std::string& content) {
        std::string bookmarkFile = config.bookmarksDir + "/bookmarks_" + groupName + ".txt";
        std::ofstream file(bookmarkFile, std::ios::app);
        
        if (file.is_open()) {
            auto timestamp = std::chrono::system_clock::now().time_since_epoch().count();
            std::string contentToSave = encrypt(content, config);
            file << timestamp << "|" << contentToSave << "\n";
            file.close();
        }
    }

    void addClipToPinned(const std::string& content) {
        std::ofstream file(config.pinnedFile, std::ios::app);
        
        if (file.is_open()) {
            auto timestamp = std::chrono::system_clock::now().time_since_epoch().count();
            std::string contentToSave = encrypt(content, config);
            file << timestamp << "|" << contentToSave << "\n";
            file.close();
        }
    }

    void createWindow() {
#ifdef __linux__
        // Create window with theme colors
        window = XCreateSimpleWindow(display, root, 
                                   WINDOW_X, WINDOW_Y, 
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
        auto oldHandler = XSetErrorHandler([](Display* /*d*/, XErrorEvent* e) -> int {
            if (e->error_code == BadAccess) {
                std::cerr << "X11 Error: BadAccess when trying to grab key\n";
                return 0;
            }
            return 0;
        });
        
        // Try multiple times with exponential backoff
        const int MAX_RETRIES = 5;
        bool success = false;
        
        for (int retry = 0; retry < MAX_RETRIES && !success; retry++) {
            if (retry > 0) {
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
                std::cout << "Successfully grabbed Ctrl+Alt+C hotkey\n";
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
        std::cout << "Visible: " << visible << "\n";

        if (!visible) {
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
    
    void hideWindow() {
        if (visible) {
#ifdef __linux__
            XUnmapWindow(display, window);
#endif
#ifdef _WIN32
            if (hwnd) {
                std::cout << "Calling ShowWindow(SW_HIDE)\n";
                ShowWindow(hwnd, SW_HIDE);
            } else {
                std::cout << "hwnd is null!\n";
            }
#endif
            visible = false;
            std::cout << "Window hidden\n";
        }
    }
    
    



//// UI METHODS ////////////////////////////////////////////////////////////////
///
///

public:
    // Linux UI Methods
    // !@!
#ifdef __linux__
        void drawConsole() {
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
            
            if (cmd_themeSelectMode) {
                data.themeItems = availableThemes;
                data.selectedTheme = selectedTheme;
                data.themeScrollOffset = themeSelectScrollOffset;
            }
            if (cmd_configSelectMode) {
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
            if (!cmd_themeSelectMode && !cmd_configSelectMode) {
                size_t displayCount = filterMode ? filteredItems.size() : items.size();
                data.totalClipCount = displayCount;
                data.selectedItem = selectedItem;
                data.clipScrollOffset = consoleScrollOffset;
                
                int availableHeight = windowHeight - data.startY - 10;
                const int SCROLL_INDICATOR_HEIGHT = 15;
                
                if (static_cast<int>(displayCount) > (availableHeight / LINE_HEIGHT)) {
                    availableHeight -= SCROLL_INDICATOR_HEIGHT;
                }
                
                int maxItems = availableHeight / LINE_HEIGHT;
                if (maxItems > 0) maxItems += 1;
                if (maxItems < 1) maxItems = 1;
                
                size_t endIdx = std::min(consoleScrollOffset + maxItems, displayCount);
                
                for (size_t i = consoleScrollOffset; i < endIdx; ++i) {
                    size_t actualIndex = filterMode ? filteredItems[i] : i;
                    const auto& item = items[actualIndex];
                    
                    std::string line;
                    if (i == selectedItem) {
                        line = "> ";
                    } else {
                        line = "  ";
                    }
                    
                    if (config.verboseMode) {
                        auto time_t = std::chrono::system_clock::to_time_t(item.timestamp);
                        auto tm = *std::localtime(&time_t);
                        
                        std::ostringstream timeStream;
                        timeStream << std::put_time(&tm, "%H:%M:%S");
                        
                        size_t lineCount = 1;
                        for (char c : item.content) {
                            if (c == '\n') lineCount++;
                        }
                        
                        line += timeStream.str() + " | " + std::to_string(lineCount) + " lines | ";
                        
                        std::string content = item.content;
                        int maxContentLength = calculateMaxContentLength(clipListWidth, true);
                        if (static_cast<int>(content.length()) > maxContentLength) {
                            content = smartTrim(content, maxContentLength);
                        }
                        
                        for (char& c : content) {
                            if (c == '\n' || c == '\r') c = ' ';
                        }
                        
                        line += content;
                    } else {
                        size_t lineCount = 1;
                        for (char c : item.content) {
                            if (c == '\n') lineCount++;
                        }
                        
                        std::string content = item.content;
                        int maxContentLength = calculateMaxContentLength(clipListWidth, false);
                        if (static_cast<int>(content.length()) > maxContentLength) {
                            content = smartTrim(content, maxContentLength);
                        }
                        
                        for (char& c : content) {
                            if (c == '\n' || c == '\r') c = ' ';
                        }
                        
                        line += content;
                        
                        if (lineCount > 1) {
                            line += " (" + std::to_string(lineCount) + " lines)";
                        }
                    }
                    
                    data.clipLines.push_back(line);
                }
            }
            
            ::drawConsole(display, window, gc, data);
            
            // Draw dialogs if visible
            if (bookmarkDialogVisible) {
                DialogDimensions dims = calculateDialogDimensions(windowWidth, windowHeight, 400, 300);
                std::vector<std::string> filteredGroups;
                for (const auto& group : bookmarkGroups) {
                    if (bookmarkDialogInput.empty() || group.find(bookmarkDialogInput) != std::string::npos) {
                        filteredGroups.push_back(group);
                    }
                }
                drawBookmarkDialog(display, window, gc, font, dims,
                                 bookmarkDialogInput, filteredGroups,
                                 selectedBookmarkGroup, bookmarkMgmtScrollOffset,
                                 config.backgroundColor, config.textColor, config.selectionColor, config.borderColor);
            }
            if (addToBookmarkDialogVisible) {
                DialogDimensions dims = calculateDialogDimensions(windowWidth, windowHeight, 400, 300);
                std::vector<std::string> displayedGroups;
                if (filterAddBookmarksMode) {
                    std::string lowerFilterText = stringToLower(filterAddBookmarksText);
                    for (const auto& group : bookmarkGroups) {
                        if (stringToLower(group).find(lowerFilterText) != std::string::npos) {
                            displayedGroups.push_back(group);
                        }
                    }
                } else {
                    displayedGroups = bookmarkGroups;
                }
                if (selectedAddBookmarkGroup >= displayedGroups.size() && !displayedGroups.empty()) {
                    selectedAddBookmarkGroup = displayedGroups.size() - 1;
                }
                drawAddToBookmarkDialog(display, window, gc, font, dims,
                                      displayedGroups,
                                      selectedAddBookmarkGroup, addBookmarkScrollOffset,
                                      filterAddBookmarksMode, filterAddBookmarksText,
                                      config.backgroundColor, config.textColor, config.selectionColor, config.borderColor);
            }
            if (viewBookmarksDialogVisible) {
                DialogDimensions dims = calculateDialogDimensions(windowWidth, windowHeight, 600, 500);
                std::string title;
                std::vector<std::string> items;
                size_t selItem = 0;
                size_t scrollOff = 0;
                bool filterActive = false;
                std::string filterTxt;
                int itemLH = 18;
                std::string emptyMsg;
                if (viewBookmarksShowingGroups) {
                    title = "Select Bookmark Group";
                    if (filterBookmarksMode) {
                        std::string lowerFilter = stringToLower(filterBookmarksText);
                        for (const auto& group : bookmarkGroups) {
                            if (stringToLower(group).find(lowerFilter) != std::string::npos) {
                                items.push_back(group);
                            }
                        }
                        filterActive = true;
                        filterTxt = filterBookmarksText;
                    } else {
                        items = bookmarkGroups;
                    }
                    if (selectedViewBookmarkGroup >= items.size() && !items.empty()) {
                        selectedViewBookmarkGroup = items.size() - 1;
                    }
                    selItem = selectedViewBookmarkGroup;
                    scrollOff = viewBookmarksScrollOffset;
                } else {
                    if (selectedViewBookmarkGroup < bookmarkGroups.size()) {
                        title = "View Bookmarks: " + bookmarkGroups[selectedViewBookmarkGroup];
                    } else {
                        title = "View Bookmarks";
                    }
                    if (filterBookmarkClipsMode) {
                        items = filteredBookmarkClips;
                        filterActive = true;
                        filterTxt = filterBookmarkClipsText;
                    } else {
                        if (selectedViewBookmarkGroup < bookmarkGroups.size()) {
                            std::string selectedGroup = bookmarkGroups[selectedViewBookmarkGroup];
                            std::string bookmarkFile = config.bookmarksDir + "/bookmarks_" + selectedGroup + ".txt";
                            std::ifstream file(bookmarkFile);
                            if (file.is_open()) {
                                std::string line;
                                while (std::getline(file, line)) {
                                    size_t pos = line.find('|');
                                    if (pos != std::string::npos && pos > 0) {
                                        std::string content = line.substr(pos + 1);
                                        try {
                                            items.push_back(decrypt(content, config));
                                        } catch (...) {
                                            items.push_back(content);
                                        }
                                    }
                                }
                                file.close();
                            }
                        }
                    }
                    int maxContentLength = calculateDialogContentLength(dims);
                    for (auto& item : items) {
                        if (static_cast<int>(item.length()) > maxContentLength) {
                            item = smartTrim(item, maxContentLength);
                        }
                        for (char& c : item) {
                            if (c == '\n' || c == '\r') c = ' ';
                        }
                    }
                    if (selectedViewBookmarkItem >= items.size() && !items.empty()) {
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
            if (pinnedDialogVisible) {
                auto sortedItems = getSortedPinnedItems(config.pinnedFile);
                std::vector<std::pair<long long, std::string>> displayItems;
                for (const auto& line : sortedItems) {
                    size_t pos = line.find('|');
                    if (pos != std::string::npos && pos > 0) {
                        std::string timestampStr = line.substr(0, pos);
                        std::string content = line.substr(pos + 1);
                        try {
                            std::string decryptedContent = decrypt(content, config);
                            long long timestamp = std::stoll(timestampStr);
                            displayItems.push_back({timestamp, decryptedContent});
                        } catch (...) {
                            try {
                                long long timestamp = std::stoll(timestampStr);
                                displayItems.push_back({timestamp, content});
                            } catch (...) {
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
                for (auto& entry : displayItems) {
                    if (static_cast<int>(entry.second.length()) > maxContentLength) {
                        entry.second = smartTrim(entry.second, maxContentLength);
                    }
                    for (char& c : entry.second) {
                        if (c == '\n' || c == '\r') c = ' ';
                    }
                }
                drawPinnedDialog(display, window, gc, font, displayItems, dims,
                                 selectedViewPinnedItem, viewPinnedScrollOffset, m_maxVisiblePinnedItems,
                                 config.backgroundColor, config.textColor, config.selectionColor, config.borderColor, LINE_HEIGHT);
            }
            if (helpDialogVisible) {
                DialogDimensions dims = calculateDialogDimensions(windowWidth, windowHeight, 600, 500);
                drawHelpDialog(display, window, gc, dims,
                               helpFilterMode, helpFilterText, helpDialogScrollOffset,
                               config.backgroundColor, config.textColor, config.borderColor);
            }
            if (editDialogVisible) {
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
        void drawConsole() {
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
            
            if (cmd_themeSelectMode) {
                data.themeItems = availableThemes;
                data.selectedTheme = selectedTheme;
                data.themeScrollOffset = themeSelectScrollOffset;
            }
            if (cmd_configSelectMode) {
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
            if (!cmd_themeSelectMode && !cmd_configSelectMode) {
                size_t displayCount = filterMode ? filteredItems.size() : items.size();
                data.totalClipCount = displayCount;
                data.selectedItem = selectedItem;
                data.clipScrollOffset = consoleScrollOffset;
                
                int availableHeight = windowHeight - data.startY - 10;
                const int SCROLL_INDICATOR_HEIGHT = 15;
                
                if (static_cast<int>(displayCount) > (availableHeight / LINE_HEIGHT)) {
                    availableHeight -= SCROLL_INDICATOR_HEIGHT;
                }
                
                int maxItems = availableHeight / LINE_HEIGHT;
                if (maxItems > 0) maxItems += 1;
                if (maxItems < 1) maxItems = 1;
                
                size_t endIdx = std::min(consoleScrollOffset + maxItems, displayCount);
                
                for (size_t i = consoleScrollOffset; i < endIdx; ++i) {
                    size_t actualIndex = filterMode ? filteredItems[i] : i;
                    const auto& item = items[actualIndex];
                    
                    std::string line;
                    if (i == selectedItem) {
                        line = "> ";
                    } else {
                        line = "  ";
                    }
                    
                    if (config.verboseMode) {
                        auto time_t = std::chrono::system_clock::to_time_t(item.timestamp);
                        auto tm = *std::localtime(&time_t);
                        
                        std::ostringstream timeStream;
                        timeStream << std::put_time(&tm, "%H:%M:%S");
                        
                        size_t lineCount = 1;
                        for (char c : item.content) {
                            if (c == '\n') lineCount++;
                        }
                        
                        line += timeStream.str() + " | " + std::to_string(lineCount) + " lines | ";
                        
                        std::string content = item.content;
                        int maxContentLength = calculateMaxContentLength(clipListWidth, true);
                        if (static_cast<int>(content.length()) > maxContentLength) {
                            content = smartTrim(content, maxContentLength);
                        }
                        
                        for (char& c : content) {
                            if (c == '\n' || c == '\r') c = ' ';
                        }
                        
                        line += content;
                    } else {
                        size_t lineCount = 1;
                        for (char c : item.content) {
                            if (c == '\n') lineCount++;
                        }
                        
                        std::string content = item.content;
                        int maxContentLength = calculateMaxContentLength(clipListWidth, false);
                        if (static_cast<int>(content.length()) > maxContentLength) {
                            content = smartTrim(content, maxContentLength);
                        }
                        
                        for (char& c : content) {
                            if (c == '\n' || c == '\r') c = ' ';
                        }
                        
                        line += content;
                        
                        if (lineCount > 1) {
                            line += " (" + std::to_string(lineCount) + " lines)";
                        }
                    }
                    
                    data.clipLines.push_back(line);
                }
            }
            
            ::drawConsole(hdc, data, WIN_SEL_RECT_HEIGHT, WIN_SEL_RECT_OFFSET_Y);
            
            // Draw dialogs if visible
            if (bookmarkDialogVisible) {
                DialogDimensions dims = calculateDialogDimensions(windowWidth, windowHeight, 400, 300);
                std::vector<std::string> filteredGroups;
                for (const auto& group : bookmarkGroups) {
                    if (bookmarkDialogInput.empty() || group.find(bookmarkDialogInput) != std::string::npos) {
                        filteredGroups.push_back(group);
                    }
                }
                drawBookmarkDialog(hdc, dims,
                                 bookmarkDialogInput, filteredGroups,
                                 selectedBookmarkGroup, bookmarkMgmtScrollOffset,
                                 config.backgroundColor, config.textColor, config.selectionColor, config.borderColor,
                                 WIN_SEL_RECT_HEIGHT, WIN_SEL_RECT_OFFSET_Y);
            }
            if (addToBookmarkDialogVisible) {
                DialogDimensions dims = calculateDialogDimensions(windowWidth, windowHeight, 400, 300);
                std::vector<std::string> displayedGroups;
                if (filterAddBookmarksMode) {
                    std::string lowerFilterText = stringToLower(filterAddBookmarksText);
                    for (const auto& group : bookmarkGroups) {
                        if (stringToLower(group).find(lowerFilterText) != std::string::npos) {
                            displayedGroups.push_back(group);
                        }
                    }
                } else {
                    displayedGroups = bookmarkGroups;
                }
                if (selectedAddBookmarkGroup >= displayedGroups.size() && !displayedGroups.empty()) {
                    selectedAddBookmarkGroup = displayedGroups.size() - 1;
                }
                drawAddToBookmarkDialog(hdc, dims,
                                      displayedGroups,
                                      selectedAddBookmarkGroup, addBookmarkScrollOffset,
                                      filterAddBookmarksMode, filterAddBookmarksText,
                                      config.backgroundColor, config.textColor, config.selectionColor, config.borderColor,
                                      WIN_SEL_RECT_HEIGHT, WIN_SEL_RECT_OFFSET_Y);
            }
            if (viewBookmarksDialogVisible) {
                DialogDimensions dims = calculateDialogDimensions(windowWidth, windowHeight, 600, 500);
                std::string title;
                std::vector<std::string> items;
                size_t selItem = 0;
                size_t scrollOff = 0;
                bool filterActive = false;
                std::string filterTxt;
                int itemLH = LINE_HEIGHT;
                std::string emptyMsg;
                if (viewBookmarksShowingGroups) {
                    title = "Select Bookmark Group";
                    if (filterBookmarksMode) {
                        std::string lowerFilter = stringToLower(filterBookmarksText);
                        for (const auto& group : bookmarkGroups) {
                            if (stringToLower(group).find(lowerFilter) != std::string::npos) {
                                items.push_back(group);
                            }
                        }
                        filterActive = true;
                        filterTxt = filterBookmarksText;
                    } else {
                        items = bookmarkGroups;
                    }
                    if (selectedViewBookmarkGroup >= items.size() && !items.empty()) {
                        selectedViewBookmarkGroup = items.size() - 1;
                    }
                    selItem = selectedViewBookmarkGroup;
                    scrollOff = viewBookmarksScrollOffset;
                } else {
                    if (selectedViewBookmarkGroup < bookmarkGroups.size()) {
                        title = "View Bookmarks: " + bookmarkGroups[selectedViewBookmarkGroup];
                    } else {
                        title = "View Bookmarks";
                    }
                    if (filterBookmarkClipsMode) {
                        items = filteredBookmarkClips;
                        filterActive = true;
                        filterTxt = filterBookmarkClipsText;
                    } else {
                        if (selectedViewBookmarkGroup < bookmarkGroups.size()) {
                            std::string selectedGroup = bookmarkGroups[selectedViewBookmarkGroup];
                            std::string bookmarkFile = config.bookmarksDir + "/bookmarks_" + selectedGroup + ".txt";
                            std::ifstream file(bookmarkFile);
                            if (file.is_open()) {
                                std::string line;
                                while (std::getline(file, line)) {
                                    size_t pos = line.find('|');
                                    if (pos != std::string::npos && pos > 0) {
                                        std::string content = line.substr(pos + 1);
                                        try {
                                            items.push_back(decrypt(content, config));
                                        } catch (...) {
                                            items.push_back(content);
                                        }
                                    }
                                }
                                file.close();
                            }
                        }
                    }
                    int maxContentLength = calculateDialogContentLength(dims);
                    for (auto& item : items) {
                        if (static_cast<int>(item.length()) > maxContentLength) {
                            item = smartTrim(item, maxContentLength);
                        }
                        for (char& c : item) {
                            if (c == '\n' || c == '\r') c = ' ';
                        }
                    }
                    if (selectedViewBookmarkItem >= items.size() && !items.empty()) {
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
            if (pinnedDialogVisible) {
                auto sortedItems = getSortedPinnedItems(config.pinnedFile);
                std::vector<std::pair<long long, std::string>> displayItems;
                for (const auto& line : sortedItems) {
                    size_t pos = line.find('|');
                    if (pos != std::string::npos && pos > 0) {
                        std::string timestampStr = line.substr(0, pos);
                        std::string content = line.substr(pos + 1);
                        try {
                            std::string decryptedContent = decrypt(content, config);
                            long long timestamp = std::stoll(timestampStr);
                            displayItems.push_back({timestamp, decryptedContent});
                        } catch (...) {
                            try {
                                long long timestamp = std::stoll(timestampStr);
                                displayItems.push_back({timestamp, content});
                            } catch (...) {
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
                for (auto& entry : displayItems) {
                    if (static_cast<int>(entry.second.length()) > maxContentLength) {
                        entry.second = smartTrim(entry.second, maxContentLength);
                    }
                    for (char& c : entry.second) {
                        if (c == '\n' || c == '\r') c = ' ';
                    }
                }
                drawPinnedDialog(hdc, displayItems, dims,
                                 selectedViewPinnedItem, viewPinnedScrollOffset, m_maxVisiblePinnedItems,
                                 config.backgroundColor, config.textColor, config.selectionColor, config.borderColor,
                                 LINE_HEIGHT, WIN_SEL_RECT_HEIGHT, WIN_SEL_RECT_OFFSET_Y);
            }
            if (helpDialogVisible) {
                DialogDimensions dims = calculateDialogDimensions(windowWidth, windowHeight, 600, 500);
                drawHelpDialog(hdc, dims,
                               helpFilterMode, helpFilterText, helpDialogScrollOffset,
                               config.backgroundColor, config.textColor, config.borderColor);
            }
            if (editDialogVisible) {
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
    void requestClipboardContent() {
        // Request clipboard content as UTF8_STRING
        XConvertSelection(display, clipboardAtom, utf8Atom, clipboardAtom, window, CurrentTime);
    }
#endif

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

            std::cout << "Existing clip moved to top\n";

            // Refresh display if window is visible
            if (visible) {
                drawConsole();
            }

            return;
        }

        items.emplace(items.begin(), trimmed_content);
        while (items.size() > config.maxClips) {
            items.pop_back();
        }
        
        // Reset selection to top when new item is added
        selectedItem = 0;
        
        // Update filtered items if in filter mode
        if (filterMode) {
            updateFilteredItems();
        }
        
        saveToFile();
        
        std::cout << "New clipboard item added\n";
        
        // Refresh display if window is visible
        if (visible) {
            drawConsole();
        }
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
        std::ofstream file(config.dataFile);
        if (file.is_open()) {
            for (const auto& item : items) {
                // Store timestamp and content (encrypted if enabled)
                auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                    item.timestamp.time_since_epoch()).count();
                std::string contentToSave = encrypt(item.content, config);
                file << timestamp << "|" << contentToSave << "\n";
            }
            file.close();
        }
    }
    
    void loadFromFile() {
        std::ifstream file(config.dataFile);
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
                            decryptedContent = decrypt(content, config);
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
            std::ofstream outFile(config.dataFile);
            if (outFile.is_open()) {
                outFile.close();
                std::cout << "Created empty clips file: " << config.dataFile << "\n";
            } else {
                std::cerr << "Failed to create clips file: " << config.dataFile << "\n";
            }
        }
    }
};

void ClipboardManager::moveCursorWordLeft() {
    std::string currentLine = "";
    std::istringstream iss(editDialogInput);
    for (size_t i = 0; i <= editDialogCursorLine; ++i) {
        std::getline(iss, currentLine);
    }

    if (editDialogCursorPos > 0) {
        size_t pos = editDialogCursorPos;
        while (pos > 0 && isspace(currentLine[pos - 1])) {
            pos--;
        }
        while (pos > 0 && !isspace(currentLine[pos - 1])) {
            pos--;
        }
        editDialogCursorPos = pos;
    }
}

void ClipboardManager::moveCursorWordRight() {
    std::string currentLine = "";
    std::istringstream iss(editDialogInput);
    for (size_t i = 0; i <= editDialogCursorLine; ++i) {
        std::getline(iss, currentLine);
    }

    if (editDialogCursorPos < currentLine.length()) {
        size_t pos = editDialogCursorPos;
        while (pos < currentLine.length() && !isspace(currentLine[pos])) {
            pos++;
        }
        while (pos < currentLine.length() && isspace(currentLine[pos])) {
            pos++;
        }
        editDialogCursorPos = pos;
    }
}

void ClipboardManager::updateFilteredBookmarkClips() {

    filteredBookmarkClips.clear();

    std::string selectedGroup = bookmarkGroups[selectedViewBookmarkGroup];

    std::string bookmarkFile = config.bookmarksDir + "/bookmarks_" + selectedGroup + ".txt";

    std::ifstream file(bookmarkFile);



    if (file.is_open()) {

        std::string line;

        while (std::getline(file, line)) {

            size_t pos = line.find('|');

            if (pos != std::string::npos && pos > 0) {

                std::string content = line.substr(pos + 1);

                try {

                    std::string decryptedContent = decrypt(content, config);

                    // Perform case-insensitive search

                    std::string lower_decrypted_content = decryptedContent;

                    std::transform(lower_decrypted_content.begin(), lower_decrypted_content.end(), lower_decrypted_content.begin(),

                                   [](unsigned char c){ return std::tolower(c); });



                    std::string lower_filter_text = filterBookmarkClipsText;

                    std::transform(lower_filter_text.begin(), lower_filter_text.end(), lower_filter_text.begin(),

                                   [](unsigned char c){ return std::tolower(c); });



                    if (lower_decrypted_content.find(lower_filter_text) != std::string::npos) {

                        filteredBookmarkClips.push_back(decryptedContent);

                    }

                } catch (...) {

                    // Fallback to non-decrypted content if decryption fails

                    std::string lower_content = content;

                    std::transform(lower_content.begin(), lower_content.end(), lower_content.begin(),

                                   [](unsigned char c){ return std::tolower(c); });



                    std::string lower_filter_text = filterBookmarkClipsText;

                    std::transform(lower_filter_text.begin(), lower_filter_text.end(), lower_filter_text.begin(),

                                   [](unsigned char c){ return std::tolower(c); });



                    if (lower_content.find(lower_filter_text) != std::string::npos) {

                        filteredBookmarkClips.push_back(content);

                    }

                }

            }

        }

        file.close();

    }

    // Reset selection if no items match

    if (filteredBookmarkClips.empty()) {

        selectedViewBookmarkItem = 0;

    } else if (selectedViewBookmarkItem >= filteredBookmarkClips.size()) {

        selectedViewBookmarkItem = filteredBookmarkClips.size() - 1;
    }
}



size_t ClipboardManager::getBookmarkItemCount() {
        if (selectedViewBookmarkGroup >= bookmarkGroups.size()) {
            return 0;
        }

        std::string selectedGroup = bookmarkGroups[selectedViewBookmarkGroup];
        std::string bookmarkFile = config.bookmarksDir + "/bookmarks_" + selectedGroup + ".txt";
        std::ifstream file(bookmarkFile);
        
            size_t itemCount = 0;
                if (file.is_open()) {
                    std::string line;

                    while (std::getline(file, line)) {
                        size_t pos = line.find('|');

                        if (pos != std::string::npos && pos > 0) {
                            itemCount++;
                        }
                    }

                    file.close();
                }

                return itemCount;
            }

        std::string stringToLower(const std::string& str) {
            std::string lower_str;

            lower_str.reserve(str.length());

            std::transform(str.begin(), str.end(), std::back_inserter(lower_str),
                           [](unsigned char c){ return std::tolower(c); });

            return lower_str;
        }

        // Global pointer for signal handling
        ClipboardManager* g_manager = nullptr;

        #include <signal.h>

        void signal_handler(int signal) {
            std::cout << "\nReceived signal " << signal << ", cleaning up...\n";

            if (g_manager) {
                // Just set running to false, don't join in signal handler
                g_manager->setRunning(false);
            }
            exit(0);
        }





#ifdef _WIN32
LRESULT CALLBACK MMRYWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Get the ClipboardManager instance
    ClipboardManager* manager = nullptr;

    if (msg == WM_NCCREATE)
    {
        // Store instance pointer
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
        manager = (ClipboardManager*)pCreate->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)manager);
    }
    else
    {
        manager = (ClipboardManager*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }
    
    switch (msg)
    {
        case WM_SIZE:
            if (manager)
            {
                int newWidth = LOWORD(lParam);
                int newHeight = HIWORD(lParam);
                manager->updateWindowDimensions(newWidth, newHeight);
                manager->drawConsole();
            }
            return 0;

        case WM_KEYDOWN:
            // Process WM_KEYDOWN only - this prevents double processing
            if (manager)
            {
                MSG winMsg = {hwnd, msg, wParam, lParam, 0, 0, 0};
                manager->handleKeyPressCommon(&winMsg);
            }
            return 0;

        case WM_CHAR:
            // Skip WM_CHAR to prevent double processing
            // All key handling is done via WM_KEYDOWN
            return 0;

        case WM_PAINT:
            if (manager)
            {
                PAINTSTRUCT ps;
                BeginPaint(hwnd, &ps);
                manager->drawConsole();
                EndPaint(hwnd, &ps);
            }
            return 0;

        case WM_ERASEBKGND:
            // Return 1 to indicate we handled background erasing
            // This prevents Windows from erasing to white
            return 1;

        case WM_CLIPBOARDUPDATE:
            if (manager)
            {
                if (OpenClipboard(hwnd))
                {
                    if (IsClipboardFormatAvailable(CF_TEXT))
                    {
                        HANDLE hData = GetClipboardData(CF_TEXT);
                        if (hData)
                        {
                            char* pszText = static_cast<char*>(GlobalLock(hData));
                            if (pszText)
                            {
                                manager->processClipboardContent(pszText);
                                GlobalUnlock(hData);
                            }
                        }
                    }
                    CloseClipboard();
                }
            }
            return 0;

        case WM_DESTROY:
            if (manager)
            {
                RemoveClipboardFormatListener(hwnd);
            }
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

#endif

int main()
{
#ifdef __linux__
    // Install signal handlers for graceful shutdown
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    SingleInstance guard("Mmry");
    if (guard.isAnotherInstanceRunning())
    {
        std::cerr << "Another instance is already running. Exiting.\n";
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
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        std::cerr << "Another instance is already running. Exiting.\n";
        if (hMutex)
        {
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
    if (hMutex)
    {
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
    }
#endif

    return 0;
}
