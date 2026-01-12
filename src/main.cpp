#include "main.h"
#include <regex>

class ClipboardManager {
public:
    ClipboardManager() {
        logfile.open("mmry_debug.log");
        writeLog("________ NEW MMRY SESSION ________");
        writeLog("");
        writeLog("");
    }

    ~ClipboardManager() {
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
    bool m_debugging = true; // Debugging flag, controlled via config
    mutable std::ofstream logfile;

    // Helper method for logging
    void writeLog(const std::string& message) const {
        if (m_debugging) {
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
    void handleKeyPress(XEvent* event) {
        handleKeyPressCommon(event);
    }
#endif

    void handleKeyPressCommon(void* eventPtr) {
        std::string key_value = "";

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
        BYTE keyboardState[256] = {0};
        GetKeyboardState(keyboardState);
#endif


        //---- Set the KeyValue ------------------------------------------------
        // Linux Keys
#ifdef __linux__
            if (keysym == XK_D && (keyEvent->state & ShiftMask)) key_value = "D";
            if (keysym == XK_G && (keyEvent->state & ShiftMask)) key_value = "G";
            if (keysym == XK_g) key_value = "g";
            if (keysym == XK_h) key_value = "h";
            if (keysym == XK_i) key_value = "i";
            if (keysym == XK_j) key_value = "j";
            if (keysym == XK_k) key_value = "k";
            if (keysym == XK_M && (keyEvent->state & ShiftMask)) key_value = "M";
            if (keysym == XK_m) key_value = "m";
            if (keysym == XK_p) key_value = "p";
            if (keysym == XK_Q && (keyEvent->state & ShiftMask)) key_value = "Q";
            if (keysym == XK_Up) key_value = "UP";
            if (keysym == XK_Down) key_value = "DOWN";
            if (keysym == XK_Left) key_value = "LEFT";
            if (keysym == XK_Right) key_value = "RIGHT";
            if (keysym == XK_Home) key_value = "HOME";
            if (keysym == XK_End) key_value = "END";
            if (keysym == XK_Escape) key_value = "ESCAPE";
            if (keysym == XK_Return) key_value = "RETURN";
            if (keysym == XK_BackSpace) key_value = "BACKSPACE";
            if (keysym == XK_Delete) key_value = "DELETE";
            if (keysym == XK_space) key_value = "SPACE";
            if (keysym == XK_grave) key_value = "`";
            if (keysym == XK_apostrophe) key_value = "'";
            if (keysym == XK_colon) key_value = ":";
            if (keysym == XK_slash) key_value = "/";
            if (keysym == XK_question) key_value = "?";
#endif
#ifdef _WIN32
            // Windows Keys - Handle WM_KEYDOWN only (WM_CHAR is skipped to prevent double processing)
            if (msg->wParam == 'D' && (GetKeyState(VK_SHIFT) & 0x8000)) key_value = "D";
            if (msg->wParam == 'G') {
                if (GetKeyState(VK_SHIFT) & 0x8000) key_value = "G";
                else key_value = "g";
            }
            if (msg->wParam == 'H') key_value = "h";
            if (msg->wParam == 'I') key_value = "i";
            if (msg->wParam == 'J') key_value = "j";
            if (msg->wParam == 'K') key_value = "k";
            if (msg->wParam == 'M') {
                if (GetKeyState(VK_SHIFT) & 0x8000) key_value = "M";
                else key_value = "m";
            }
            if (msg->wParam == 'P') key_value = "p";
            if (msg->wParam == 'Q' && (GetKeyState(VK_SHIFT) & 0x8000)) key_value = "Q";
            if (msg->wParam == VK_UP) key_value = "UP";
            if (msg->wParam == VK_DOWN) key_value = "DOWN";
            if (msg->wParam == VK_LEFT) key_value = "LEFT";
            if (msg->wParam == VK_RIGHT) key_value = "RIGHT";
            if (msg->wParam == VK_HOME) key_value = "HOME";
            if (msg->wParam == VK_END) key_value = "END";
            if (msg->wParam == VK_ESCAPE) key_value = "ESCAPE";
            if (msg->wParam == VK_RETURN) key_value = "RETURN";
            if (msg->wParam == VK_BACK) key_value = "BACKSPACE";
            if (msg->wParam == VK_DELETE) key_value = "DELETE";
            if (msg->wParam == VK_SPACE) key_value = "SPACE";
            if (msg->wParam == VK_OEM_3) key_value = "`";
            if (msg->wParam == VK_OEM_7) key_value = "'";
            if (msg->wParam == VK_OEM_1 && (GetKeyState(VK_SHIFT) & 0x8000)) key_value = ":";
            if (msg->wParam == VK_OEM_2) key_value = "/";
            if ((msg->wParam == VK_OEM_2 && (GetKeyState(VK_SHIFT) & 0x8000))) key_value = "?";
#endif
        //---- End Set the KeyValue --------------------------------------------




        if (key_value == "Q") {
            // Shift+Q quits application even from dialog
            std::cout << "Quitting MMRY...\n";
            running = false; // Let main loop exit naturally to avoid deadlock
        }

        //---- General Escape --------------------------------------------------
        if (key_value == "ESCAPE") {
            if (key_global_escape()) return;
        }
        //----------------------------------------------------------------------


        //---- Help Dialog -----------------------------------------------------
        if (helpDialogVisible) {
            if (key_value == "?") {
                if (key_help_hide()) return;
            }

            if (key_value == "j" || key_value == "DOWN") {
                if (key_help_scroll_down()) return;
            }

            if (key_value == "k" || key_value == "UP") {
                if (key_help_scroll_up()) return;
            }

            if (key_value == "g") {
                if (key_help_scroll_top()) return;
            }

            return;
        }

        //---- Edit Dialog -----------------------------------------------------
        if (editDialogVisible) {
            if (key_value == "ESCAPE") {
                if (key_edit_escape()) return;
            }

            // Handle CTRL+ENTER for saving
#ifdef __linux__
            if (keysym == XK_Return && (keyEvent->state & ControlMask)) {
                if (key_edit_save()) return;
            }
#endif
#ifdef _WIN32
            if (msg->wParam == VK_RETURN && (GetKeyState(VK_CONTROL) & 0x8000)) {
                if (key_edit_save()) return;
            }
#endif
            // Handle CTRL+LEFT for moving one word left
#ifdef __linux__
            if (keysym == XK_Left && (keyEvent->state & ControlMask)) {
                moveCursorWordLeft();
                drawConsole();
                return;
            }
#endif
#ifdef _WIN32
            if (msg->wParam == VK_LEFT && (GetKeyState(VK_CONTROL) & 0x8000)) {
                moveCursorWordLeft();
                drawConsole();
                return;
            }
#endif

            // Handle CTRL+RIGHT for moving one word right
#ifdef __linux__
            if (keysym == XK_Right && (keyEvent->state & ControlMask)) {
                moveCursorWordRight();
                drawConsole();
                return;
            }
#endif
#ifdef _WIN32
            if (msg->wParam == VK_RIGHT && (GetKeyState(VK_CONTROL) & 0x8000)) {
                moveCursorWordRight();
                drawConsole();
                return;
            }
#endif
            
            if (key_value == "RETURN") {
                if (key_edit_add_newline()) return;
            }

            if (key_value == "BACKSPACE") {
                if (key_edit_backspace()) return;
            }

            if (key_value == "DELETE") {
                if (key_edit_delete()) return;
            }

            if (key_value == "UP") {
                if (key_edit_cursor_up()) return;
            }

            if (key_value == "DOWN") {
                if (key_edit_cursor_down()) return;
            }

            if (key_value == "LEFT") {
                if (key_edit_cursor_left()) return;
            }

            if (key_value == "RIGHT") {
                if (key_edit_cursor_right()) return;
            }

            if (key_value == "HOME") {
                if (key_edit_home()) return;
            }

            if (key_value == "END") {
                if (key_edit_end()) return;
            }

            // Text input
#ifdef _WIN32
            char typedChar = getCharFromMsg(msg);
            if (typedChar != 0 && typedChar != '\r' && typedChar != '\n') { // Ignore enter key here as it is handled by CTRL+ENTER
                key_edit_add_char(typedChar);
                return;
            }
#else
            char buffer[10];
            int count = XLookupString(keyEvent, buffer, sizeof(buffer), nullptr, nullptr);
            if (count > 0 && buffer[0] != '\r' && buffer[0] != '\n') { // Ignore enter key here as it is handled by CTRL+ENTER
                key_edit_add_char(buffer[0]);
                return;
            }
#endif
            return;
        }

        // ---------------------------------------------------------------------


        // Adding bookmark groups
        //
        if (bookmarkDialogVisible && !addToBookmarkDialogVisible) {
            if (key_value == "RETURN") {
                if (key_addgroup_add()) return;
            }

            if (key_value == "BACKSPACE") {
                if (key_addgroup_back()) return;
            }

            // Text input for bookmark dialog - exclude vim navigation keys
            // Plain Text
#ifdef _WIN32
                // Handle character input from WM_KEYDOWN
                char typedChar = getCharFromMsg(msg); 
                if (typedChar != 0) {
                    bookmarkDialogInput += typedChar;  
                    drawConsole();
                }
#else
                char buffer[10];
                int count = XLookupString(keyEvent, buffer, sizeof(buffer), nullptr, nullptr);
                if (count > 0) {
                    bookmarkDialogInput += std::string(buffer, count);
                    drawConsole();
                }
#endif
            // End Plain Text

            return;
        }


        // Accessing bookmarked clips
        //
        if (viewBookmarksDialogVisible) {
            if (key_value == "`") {
                if (key_marks_show()) return;
            }


            // Groups view
            //
            if (viewBookmarksShowingGroups) {
                if (filterBookmarksMode) {
                    if (key_value == "RETURN") {
                        if (key_marks_groups_clips()) return;
                    }
                    if (key_value == "ESCAPE") {
                        filterBookmarksMode = false;
                        filterBookmarksText.clear();
                        drawConsole();
                        return;
                    }
                    if (key_value == "BACKSPACE") {
                        if (!filterBookmarksText.empty()) {
                            filterBookmarksText.pop_back();
                            drawConsole();
                        }
                        return;
                    }
                    // Text input for filter
#ifdef _WIN32
                    char typedChar = getCharFromMsg(msg);
                    if (typedChar != 0) {
                        filterBookmarksText += typedChar;
                        drawConsole();
                    }
#else
                    char buffer[10];
                    int count = XLookupString(keyEvent, buffer, sizeof(buffer), nullptr, nullptr);
                    if (count > 0) {
                        filterBookmarksText += std::string(buffer, count);
                        drawConsole();
                    }
#endif
                    return;
                }

                if (key_value == "/") {
                    filterBookmarksMode = true;
                    filterBookmarksText.clear();
                    drawConsole();
                    return;
                }
                if (key_value == "j" || key_value == "DOWN") {
                    if (key_marks_groups_down()) return;
                }

                if (key_value == "k" || key_value == "UP") {
                    if (key_marks_groups_up()) return;
                }

                if (key_value == "g") {
                    if (key_marks_groups_top()) return;
                }

                if (key_value == "G") {
                    if (key_marks_groups_bottom()) return;
                }

                if (key_value == "D") {
                    if (key_marks_groups_delete()) return;
                }

                if (key_value == "RETURN") {
                    if (key_marks_groups_clips()) return;
                }
            }

            // Clips are being shown
            //
            else {
                if (key_value == "j" || key_value == "DOWN") {
                    if (key_marks_clips_down()) return;
                }

                if (key_value == "k" || key_value == "UP") {
                    if (key_marks_clips_up()) return;
                }

                if (key_value == "g") {
                    if (key_marks_clips_top()) return;
                }

                if (key_value == "G") {
                    if (key_marks_clips_bottom()) return;
                }

                if (key_value == "D") {
                    if (key_marks_clips_delete()) return;
                }

                if (key_value == "RETURN") {
                    if (key_marks_clips_copy()) return;
                }

                if (key_value == "h") {
                    if (key_marks_clips_groups()) return;
                }
            }
            return;
        }


        // Accessing pinned clips
        //
        if (pinnedDialogVisible) {
            if (key_value == "j" || key_value == "DOWN") {
                if (key_pin_down()) return;
            }

            if (key_value == "k" || key_value == "UP") {
                if (key_pin_up()) return;
            }

            if (key_value == "g") {
                if (key_pin_top()) return;
            }

            if (key_value == "G") {
                if (key_pin_bottom()) return;
            }

            if (key_value == "D") {
                if (key_pin_delete()) return;
            }

            if (key_value == "RETURN") {
                if (key_pin_copy()) return;
            }

            return;
        }


        // Adding the current clip to a bookmark group
        //
        if (addToBookmarkDialogVisible) {
            // Add to bookmark dialog is visible - handle dialog-specific keys
            if (filterAddBookmarksMode) {
                if (key_value == "RETURN") {
                    if (key_addmarks_add()) return;
                }
                if (key_value == "ESCAPE") {
                    filterAddBookmarksMode = false;
                    filterAddBookmarksText.clear();
                    drawConsole();
                    return;
                }
                if (key_value == "BACKSPACE") {
                    if (!filterAddBookmarksText.empty()) {
                        filterAddBookmarksText.pop_back();
                        drawConsole();
                    }
                    return;
                }
                // Text input for filter
#ifdef _WIN32
                char typedChar = getCharFromMsg(msg);
                if (typedChar != 0) {
                    filterAddBookmarksText += typedChar;
                    drawConsole();
                }
#else
                char buffer[10];
                int count = XLookupString(keyEvent, buffer, sizeof(buffer), nullptr, nullptr);
                if (count > 0) {
                    filterAddBookmarksText += std::string(buffer, count);
                    drawConsole();
                }
#endif
                return;
            }

            if (key_value == "/") {
                filterAddBookmarksMode = true;
                filterAddBookmarksText.clear();
                drawConsole();
                return;
            }

            if (key_value == "RETURN") {
                if (key_addmarks_add()) return;
            }

            if (key_value == "j" || key_value == "DOWN") {
                if (key_addmarks_down()) return;
            }

            if (key_value == "k" || key_value == "UP") {
                if (key_addmarks_up()) return;
            }

            if (key_value == "g") {
                if (key_addmarks_top()) return;
            }

            if (key_value == "G") {
                if (key_addmarks_bottom()) return;
            }

            return;
        }


        // Filter mode
        //
        if (filterMode) {
            if (key_value == "BACKSPACE") {
                // Remove last character from filter
                if (!filterText.empty()) {
                    filterText.pop_back();
                    updateFilteredItems();
                    selectedItem = 0;
                    drawConsole();
                }
                return;
            }

            if (key_value == "DELETE") {
                if (key_filter_delete()) return;
            }

            if (key_value == "RETURN") {
                if (key_filter_copy()) return;
            }

            if (key_value == "DOWN") {
                if (key_filter_down()) return;
            }

            if (key_value == "UP") {
                if (key_filter_up()) return;
            }

            
            // Free Text
#ifdef _WIN32
                // Handle character input from WM_KEYDOWN
                char typedChar = getCharFromMsg(msg); 
                if (typedChar != 0) {
                    bookmarkDialogInput += typedChar;  
                    // Don't add the triggering '/' as the first character
                    if (filterText.empty() && msg->wParam == '/') {
                        // do nothing
                    } else {
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
        if (commandMode) {
            if (key_value == "BACKSPACE") {
                if (!commandText.empty()) {
                    commandText.pop_back();
                    drawConsole();
                }
                return;
            }

            if (key_value == "RETURN") {
                if (key_command_execute()) return;
            }

            if (key_value == "DOWN") {
                if (key_command_down()) return;
            }

            if (key_value == "UP") {
                if (key_command_up()) return;
            }

            if (key_value == "SPACE") {
                if (key_command_detect()) return;
            }

            // Free Text
#ifdef _WIN32
                // Handle character input from WM_KEYDOWN
                char typedChar = getCharFromMsg(msg); 
                if (typedChar != 0) {
                    bookmarkDialogInput += typedChar;  
                    // Don't add the triggering ':' as the first character
                    if (commandText.empty() && msg->wParam == ':') {
                        // do nothing
                    } else {
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
        if (cmd_themeSelectMode) {
            if (key_value == "ESCAPE") {
                if (key_theme_cancel()) return;
            }

            if (key_value == "RETURN") {
                if (key_theme_apply()) return;
            }

            if (key_value == "j" || key_value == "DOWN") {
                if (key_theme_down()) return;
            }

            if (key_value == "k" || key_value == "UP") {
                if (key_theme_up()) return;
            }

            if (key_value == "g") {
                if (key_theme_top()) return;
            }

            if (key_value == "G") {
                if (key_theme_bottom()) return;
            }

            return;
        }

        // Config selection mode
        //
        if (cmd_configSelectMode) {
            if (key_value == "ESCAPE") {
                if (key_config_cancel()) return;
            }

            if (key_value == "RETURN") {
                if (key_config_select()) return;
            }

            if (key_value == "j" || key_value == "DOWN") {
                if (key_config_down()) return;
            }

            if (key_value == "k" || key_value == "UP") {
                if (key_config_up()) return;
            }

            if (key_value == "g") {
                if (key_config_top()) return;
            }

            if (key_value == "G") {
                if (key_config_bottom()) return;
            }

            return;
        }


        // General keys - main clips list
        //
        if (key_value == "j" || key_value == "DOWN") {
            if (key_main_down()) return;
        }

        if (key_value == "k" || key_value == "UP") {
            if (key_main_up()) return;
        }

        if (key_value == "g") {
            if (key_main_top()) return;
        }

        if (key_value == "G") {
            if (key_main_bottom()) return;
        }

        if (key_value == "D") {
            if (key_main_delete()) return;
        }

        if (key_value == "/") {
            if (key_main_filter_start()) return;
        }

        if (key_value == ":") {
            if (key_main_command_start()) return;
        }

        if (key_value == "RETURN") {
            if (key_main_copy()) return;
        }

        if (key_value == "M") {
            if (key_main_addgroup_start()) return;
        }

        if (key_value == "m") {
            if (key_main_addclip_start()) return;
        }

        if (key_value == "?") {
            if (key_main_help_start()) return;
        }

        if (key_value == "`") {
            if (key_main_accessmarks_start()) return;
        }

        if (key_value == "p") {
            if (key_main_pin_clip()) return;
        }

        if (key_value == "i") {
            if (key_main_edit_start()) return;
        }

        // Pinned clips dialog
        if (key_value == "'") {
            if (key_main_pins_start()) return;
        }
    }


    //// Key Press Methods /////////////////////////////////////////////////////
        bool key_global_escape() {
            if (editDialogVisible) {
                if (key_edit_escape()) return true;
            }
            if (filterBookmarksMode) {
                filterBookmarksMode = false;
                filterBookmarksText.clear();
                drawConsole();
                return true;
            }
            if (filterAddBookmarksMode) {
                filterAddBookmarksMode = false;
                filterAddBookmarksText.clear();
                drawConsole();
                return true;
            }
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
            } else if (cmd_configSelectMode) {
                // Exit config selection mode and return to command mode
                cmd_configSelectMode = false;
                commandMode = true;
                commandText = "";
                availableConfigs.clear();
                selectedItem = 0;
                drawConsole();
            } else {
                // Normal escape behavior - hide window
                hideWindow();
            }

            return true;
        }

        bool key_edit_escape() {
            editDialogVisible = false;
            drawConsole();
            return true;
        }

        bool key_edit_save() {
            if (!editDialogInput.empty()) {
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

        bool key_edit_backspace() {
            std::string& text = editDialogInput;
            size_t& line_num = editDialogCursorLine;
            size_t& char_pos = editDialogCursorPos;

            if (char_pos > 0) {
                // Find the position of the character to delete
                size_t deletion_pos = 0;
                std::istringstream iss(text);
                std::string current_line;
                for (size_t i = 0; i < line_num; ++i) {
                    std::getline(iss, current_line);
                    deletion_pos += current_line.length() + 1; // +1 for newline
                }
                deletion_pos += char_pos -1;
                text.erase(deletion_pos, 1);
                char_pos--;

            } else if (line_num > 0) {
                // Find the end of the previous line
                size_t prev_line_end = 0;
                std::istringstream iss(text);
                std::string current_line;
                for (size_t i = 0; i < line_num -1; ++i) {
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

        bool key_edit_delete() {
            std::string& text = editDialogInput;
            size_t& line_num = editDialogCursorLine;
            size_t& char_pos = editDialogCursorPos;

            std::istringstream iss(text);
            std::string current_line;
            size_t current_line_index = 0;
            size_t deletion_pos = 0;

            while (current_line_index <= line_num && std::getline(iss, current_line)) {
                if (current_line_index < line_num) {
                    deletion_pos += current_line.length() + 1; // +1 for newline
                }
                current_line_index++;
            }

            deletion_pos += char_pos;
            if (deletion_pos < text.length()) {
                text.erase(deletion_pos, 1);
                updateEditDialogScrollOffset();
                drawConsole();
            }
            return true;
        }

        void key_edit_add_char(char c) {
            std::string& text = editDialogInput;
            size_t& line_num = editDialogCursorLine;
            size_t& char_pos = editDialogCursorPos;

            std::istringstream iss(text);
            std::string current_line;
            size_t current_line_index = 0;
            size_t insertion_pos = 0;

            while (current_line_index <= line_num && std::getline(iss, current_line)) {
                if (current_line_index < line_num) {
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

        bool key_edit_add_newline() {
            key_edit_add_char('\n');
            editDialogCursorLine++;
            editDialogCursorPos = 0; // Ensure cursor is at the beginning of the new line
            updateEditDialogScrollOffset();
            return true;
        }

        bool key_edit_scroll_up() {
            if (editDialogScrollOffset > 0) {
                editDialogScrollOffset--;
                drawConsole();
            }
            return true;
        }

        bool key_edit_scroll_down() {
            // Need to calculate max scroll offset based on content and dialog height
            DialogDimensions dims = getEditDialogDimensions();
            const int lineHeight = 15;
            const int charWidth = 8;
            int maxCharsPerLine = (dims.width - 50) / charWidth;
            if (maxCharsPerLine < 1) maxCharsPerLine = 1;

            int totalVisualLines = 0;
            if (editDialogInput.empty()) {
                totalVisualLines = 1;
            } else {
                std::istringstream iss(editDialogInput);
                std::string logicalLine;
                while (std::getline(iss, logicalLine)) {
                    if (logicalLine.empty()) {
                        totalVisualLines++;
                    } else {
                        totalVisualLines += (logicalLine.length() + maxCharsPerLine - 1) / maxCharsPerLine;
                    }
                }
                if (editDialogInput.back() == '\n') {
                    totalVisualLines++;
                }
            }

            int maxVisibleLines = (dims.height - 70) / lineHeight;
            
            if (editDialogScrollOffset < totalVisualLines - maxVisibleLines) {
                editDialogScrollOffset++;
                drawConsole();
            }
            return true;
        }

        // Help
        bool key_help_hide() {
            helpDialogVisible = false;
            drawConsole();
            return true;
        }

        bool key_help_scroll_down() {
            updateHelpDialogScrollOffset(-1);
            drawConsole();
            return true;
        }

        bool key_help_scroll_up() {
            if (helpDialogScrollOffset > 0) {
                updateHelpDialogScrollOffset(1);
                drawConsole();
            }
            return true;
        }

        bool key_help_scroll_top() {
            helpDialogScrollOffset = 0;
            drawConsole();
            return true;
        }

        // Add Groups
        bool key_addgroup_add() {
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
                        std::cout << "Added clip to bookmark group: " << bookmarkDialogInput << "\n";
                    }
                } else {
                    // Add current clip to existing group
                    if (!items.empty() && selectedItem < getDisplayItemCount()) {
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

        bool key_addgroup_back() {
            if (!bookmarkDialogInput.empty()) {
                bookmarkDialogInput.pop_back();
                drawConsole();
            }
            return true;
        }

        // Bookmarks
        bool key_marks_show() {
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

            return true;
        }

        bool key_marks_groups_down() {
            if (selectedViewBookmarkGroup < bookmarkGroups.size() - 1) {
                selectedViewBookmarkGroup++;
                updateScrollOffset();
                drawConsole();
            }
            return true;
        }

        bool key_marks_groups_up() {
            if (selectedViewBookmarkGroup > 0) {
                selectedViewBookmarkGroup--;
                updateScrollOffset();
                drawConsole();
            }
            return true;
        }

        bool key_marks_groups_top() {
            selectedViewBookmarkGroup = 0;
            viewBookmarksScrollOffset = 0;
            drawConsole();
            return true;
        }

        bool key_marks_groups_bottom() {
            selectedViewBookmarkGroup = bookmarkGroups.size() - 1;
            updateScrollOffset();
            drawConsole();
            return true;
        }

        bool key_marks_groups_delete() {
            if (selectedViewBookmarkGroup < bookmarkGroups.size()) {
                std::string groupToDelete = bookmarkGroups[selectedViewBookmarkGroup];
                
                // Remove group from list
                bookmarkGroups.erase(bookmarkGroups.begin() + selectedViewBookmarkGroup);
                saveBookmarkGroups();
                
                // Delete bookmark file
                std::string bookmarkFile = bookmarksDir + "/bookmarks_" + groupToDelete + ".txt";
                unlink(bookmarkFile.c_str());
                
                std::cout << "Deleted bookmark group and all clips: " << groupToDelete << "\n";
                
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
            return true;
        }

        bool key_marks_groups_clips() {
            std::vector<std::string> displayedGroups;
            if (filterBookmarksMode) {
                for (const auto& group : bookmarkGroups) {
                    if (group.find(filterBookmarksText) != std::string::npos) {
                        displayedGroups.push_back(group);
                    }
                }
            } else {
                displayedGroups = bookmarkGroups;
            }

            if (selectedViewBookmarkGroup < displayedGroups.size()) {
                // Find the actual index in the original bookmarkGroups vector
                std::string selectedGroupName = displayedGroups[selectedViewBookmarkGroup];
                auto it = std::find(bookmarkGroups.begin(), bookmarkGroups.end(), selectedGroupName);
                if (it != bookmarkGroups.end()) {
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


        bool key_marks_clips_down() {
            selectedViewBookmarkItem++;
            updateScrollOffset();
            drawConsole();
            return true;
        }

        bool key_marks_clips_up() {
            if (selectedViewBookmarkItem > 0) {
                selectedViewBookmarkItem--;
                updateScrollOffset();
                drawConsole();
            }
            return true;
        }

        bool key_marks_clips_top() {
            selectedViewBookmarkItem = 0;
            viewBookmarksScrollOffset = 0;
            drawConsole();
            return true;
        }

        bool key_marks_clips_bottom() {
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
            return true;
        }

        bool key_marks_clips_delete() {
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
                            
                            std::cout << "Deleted bookmark item from group: " << selectedGroup << "\n";
                            
                            // Adjust selection
                            if (selectedViewBookmarkItem > 0 && selectedViewBookmarkItem >= lines.size()) {
                                selectedViewBookmarkItem = lines.size() - 1;
                            }
                            
                            drawConsole();
                        }
                    }
                }
            }
            return true;
        }

        bool key_marks_clips_copy() {
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
                            std::cout << "Copied " << lines << " lines from bookmark" << "\n";
                        } else {
                            std::cout << "Copied from bookmark: " << bookmarkItems[selectedViewBookmarkItem].substr(0, 50) << "..." << "\n";
                        }
                        viewBookmarksDialogVisible = false;
                        hideWindow();
                    }
                }
            }
            return true;
        }

        bool key_marks_clips_groups() {
            viewBookmarksShowingGroups = true;
            drawConsole();
            return true;
        }

        // Pinned Clips
        bool key_pin_down() {
            auto sortedItems = getSortedPinnedItems();
            if (selectedViewPinnedItem < sortedItems.size() - 1) {
                selectedViewPinnedItem++;
                updatePinnedScrollOffset();
                drawConsole();
            }
            return true;
        }

        bool key_pin_up() {
            if (selectedViewPinnedItem > 0) {
                selectedViewPinnedItem--;
                updatePinnedScrollOffset();
                drawConsole();
            }
            return true;
        }

        bool key_pin_top() {
            selectedViewPinnedItem = 0;
            viewPinnedScrollOffset = 0;
            drawConsole();
            return true;
        }

        bool key_pin_bottom() {
            auto sortedItems = getSortedPinnedItems();
            if (!sortedItems.empty()) {
                selectedViewPinnedItem = sortedItems.size() - 1;
                updatePinnedScrollOffset();
                drawConsole();
            }
            return true;
        }

        bool key_pin_delete() {
            auto sortedItems = getSortedPinnedItems();
            
            // Remove the selected item if valid
            if (selectedViewPinnedItem < sortedItems.size()) {
                sortedItems.erase(sortedItems.begin() + selectedViewPinnedItem);

                // Write back remaining lines
                std::ofstream outFile(pinnedFile);
                if (outFile.is_open()) {
                    for (const auto& item : sortedItems) {
                        outFile << item << "\n";
                    }
                    outFile.close();
                    
                    std::cout << "Deleted pinned clip\n";
                    
                    // Adjust selection
                    if (selectedViewPinnedItem > 0 && selectedViewPinnedItem >= sortedItems.size()) {
                        selectedViewPinnedItem = sortedItems.size() - 1;
                    }
                    
                    // Close dialog if no pinned clips left
                    if (sortedItems.empty()) {
                        pinnedDialogVisible = false;
                    }
                    
                    drawConsole();
                }
            }
            return true;
        }

        bool key_pin_copy() {
            auto sortedItems = getSortedPinnedItems();

            if (selectedViewPinnedItem < sortedItems.size()) {
                std::string selectedLine = sortedItems[selectedViewPinnedItem];
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
                        std::cout << "Copied " << lineCount << " lines from pinned clips\n";
                    } else {
                        std::cout << "Copied from pinned clips: " << contentToCopy.substr(0, 50) << "...\n";
                    }

                    // Remove the old line from sorted items
                    sortedItems.erase(sortedItems.begin() + selectedViewPinnedItem);

                    // Add the new line at the beginning
                    auto newTimestamp = std::chrono::system_clock::now().time_since_epoch().count();
                    std::string newLine = std::to_string(newTimestamp) + "|" + contentToSave;
                    sortedItems.insert(sortedItems.begin(), newLine);
                    
                    // Write the updated lines back to the file
                    std::ofstream outFile(pinnedFile);
                    for (const auto& l : sortedItems) {
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
                discoverThemes();
                // Store original theme and apply first theme for preview
                if (!availableThemes.empty()) {
                    originalTheme = theme;
                    selectedTheme = 0;
                    switchTheme(availableThemes[0]);
                }
                drawConsole();
                return true;
            }
            if (commandText == "config") {
                // Enter config selection mode
                commandMode = false;
                cmd_configSelectMode = true;
                discoverConfigs();
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
            if (!originalTheme.empty()) {
                switchTheme(originalTheme);
            }
            cmd_themeSelectMode = false;
            availableThemes.clear();
            originalTheme.clear();
            drawConsole();

            return true;
        }

        bool key_theme_apply() {
            if (selectedTheme < availableThemes.size()) {
                switchTheme(availableThemes[selectedTheme]);
                // Save to config
                saveConfig();
            }
            cmd_themeSelectMode = false;
            availableThemes.clear();
            originalTheme.clear();
            drawConsole();

            return true;
        }

        bool key_theme_down() {
            if (selectedTheme < availableThemes.size() - 1) {
                selectedTheme++;
                updateThemeSelectScrollOffset();
                // Apply live preview
                if (selectedTheme < availableThemes.size()) {
                    switchTheme(availableThemes[selectedTheme]);
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
                    switchTheme(availableThemes[selectedTheme]);
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
                std::string currentValue = getConfigValue(configKey);
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
            helpDialogScrollOffset = 0; // Reset scroll offset when opening help dialog
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
        setupConfigDir();
        loadConfig();
        loadTheme();
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
        std::cout << "Config directory: " << configDir << "\n";
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
    
            std::atomic<bool> running;
            std::atomic<bool> visible;
        // Window properties
        int windowWidth = 800;
        int windowHeight = 600;
        const int WINDOW_X = 100;
        const int WINDOW_Y = 100;
        const int LINE_HEIGHT = 25;
        const int WIN_SEL_RECT_HEIGHT = 26;
        const int WIN_SEL_RECT_OFFSET_Y = 4;
        const int FONT_SIZE = 16;
        const std::string FONT_NAME = "Consolas";
        
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

    // Config selection mode
    bool cmd_configSelectMode = false;
    std::vector<std::string> availableConfigs;
    size_t selectedConfig = 0;
    size_t configSelectScrollOffset = 0;
    

    
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
    int m_maxVisiblePinnedItems = 1; // Stores the number of currently visible pinned items
    
    // Add to bookmark dialog state
    bool addToBookmarkDialogVisible = false;
    size_t selectedAddBookmarkGroup = 0;
    size_t addBookmarkScrollOffset = 0; // For scrolling long lists
    
    // Help dialog state
    bool helpDialogVisible = false;
    size_t helpDialogScrollOffset = 0;

    // Edit dialog state
    bool editDialogVisible = false;
    std::string editDialogInput;
    int editDialogScrollOffset = 0;
    size_t editDialogCursorPos = 0;
    size_t editDialogCursorLine = 0;
    
    // View bookmarks dialog state
    bool viewBookmarksDialogVisible = false;
    bool viewBookmarksShowingGroups = true; // true = groups, false = clips
    size_t selectedViewBookmarkGroup = 0;
    size_t selectedViewBookmarkItem = 0;
    size_t viewBookmarksScrollOffset = 0; // For scrolling long lists

    // Bookmark dialog filtering
    bool filterBookmarksMode = false;
    std::string filterBookmarksText;

    bool filterAddBookmarksMode = false;
    std::string filterAddBookmarksText;
    
    // Helper methods

    // Pinned clips helper methods
    ////////////////////////////////////////////////////////////////////////////
    std::vector<std::string> getSortedPinnedItems() {
        std::vector<std::pair<long long, std::string>> pinnedItems;
        std::string line;
        std::ifstream inFile(pinnedFile);
        
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

        // Sort by timestamp (newest first) to match display order
        std::sort(pinnedItems.begin(), pinnedItems.end(),
                  [](const auto& a, const auto& b) { return a.first > b.first; });

        // Extract just the line strings in sorted order
        std::vector<std::string> sortedLines;
        for (const auto& item : pinnedItems) {
            sortedLines.push_back(item.second);
        }
        
        return sortedLines;
    }

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
    
    std::string wildcardToRegex(const std::string& wildcard_pattern) {
        std::string regex_pattern;
        regex_pattern.reserve(wildcard_pattern.size() * 2);
        for (char c : wildcard_pattern) {
            switch (c) {
                case '*':
                    regex_pattern += ".*";
                    break;
                // Escape other special regex characters
                case '.': case '+': case '?': case '^': case '$': case '(': case ')':
                case '[': case ']': case '{': case '}': case '|': case '\\':
                    regex_pattern += '\\';
                    regex_pattern += c;
                    break;
                default:
                    regex_pattern += c;
            }
        }
        return regex_pattern;
    }
    
    void updateFilteredItems() {
        filteredItems.clear();
        
        if (filterText.empty()) {
            for (size_t i = 0; i < items.size(); ++i) {
                filteredItems.push_back(i);
            }
        } else {
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
                // For now, we can just not filter, or log an error
                writeLog("Regex error: " + std::string(e.what()));
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
        DialogDimensions dims = getViewBookmarksDialogDimensions();
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
            if (selectedViewBookmarkItem < viewBookmarksScrollOffset) {
                viewBookmarksScrollOffset = selectedViewBookmarkItem;
            } else if (selectedViewBookmarkItem >= viewBookmarksScrollOffset + dynamicVisibleItems) {
                viewBookmarksScrollOffset = selectedViewBookmarkItem - dynamicVisibleItems + 1;
            }
        }
    }
    
    void updateEditDialogScrollOffset() {
        DialogDimensions dims = getEditDialogDimensions();
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

        writeLog("-- calculateDialogDimensions --");
        writeLog("windowHeight: " + std::to_string(windowHeight) + ", windowWidth: " + std::to_string(windowWidth));
        
        // Calculate maximum size that fits in window with margins
        int maxHeight = windowHeight - 40; // 20px margin on each side
        int maxWidth = windowWidth - 40;  // 20px margin on each side

        writeLog("maxHeight: " + std::to_string(maxHeight) + ", maxWidth: " + std::to_string(maxWidth));

        // Ensure minimum usable size
        int minDialogWidth = 200;
        int minDialogHeight = 100;
        
        // Use preferred size if it fits, otherwise scale down
        dims.height = std::min(preferredHeight, std::max(minDialogHeight, maxHeight));
        dims.width = std::min(preferredWidth, std::max(minDialogWidth, maxWidth));

        writeLog("dims.height: " + std::to_string(dims.height) + ", dims.width: " + std::to_string(dims.width));
        
        // Calculate content area (excluding borders and margins)
        dims.contentHeight = dims.height - 80; // 30px margin top/bottom for title and padding
        dims.contentWidth = dims.width - 40;  // 20px margin on each side

        // Ensure minimum content area
        if (dims.contentWidth < 200) dims.contentWidth = 200;

        writeLog("dims.contentHeight: " + std::to_string(dims.contentHeight) + ", dims.contentWidth: " + std::to_string(dims.contentWidth));

        // Center dialog in window
        dims.x = (windowWidth - dims.width) / 2;
        dims.y = (windowHeight - dims.height) / 2;

        writeLog("dims.x: " + std::to_string(dims.x) + ", dims.y: " + std::to_string(dims.y));


        
        return dims;
    }
    
    DialogDimensions getBookmarkDialogDimensions() const {
        return calculateDialogDimensions(400, 300);
    }

    DialogDimensions getPinnedDimensions() const {
        return calculateDialogDimensions(200, 150);
    }
    
    DialogDimensions getAddBookmarkDialogDimensions() const {
        return calculateDialogDimensions(400, 300);
    }
    
    DialogDimensions getHelpDialogDimensions() const {
        return calculateDialogDimensions(600, 500);
    }
    
    DialogDimensions getEditDialogDimensions() const {
        return calculateDialogDimensions(600, 400); // Or adjust preferred size as needed
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
            
#ifdef _WIN32
            // Windows COLORREF uses BGR format (0x00BBGGRR)
            return b * 256 * 256 + g * 256 + r;
#else
            // Linux/X11 uses RGB format
            return r * 256 * 256 + g * 256 + b;
#endif
        } catch (...) {
            return 0; // Default to black if conversion fails
        }
    }
    
    void loadTheme() {
        // Set default colors (console theme)
#ifdef _WIN32
        // Windows COLORREF uses BGR format
        backgroundColor = 0x000000; // Black (0x000000 works in both formats)
        textColor = 0xFFFFFF;      // White (0xFFFFFF works in both formats)
        selectionColor = 0x333333;  // Dark gray (0x333333 works in both formats)
        borderColor = 0x888888;    // Gray (0x888888 works in both formats)
#else
        // Linux/X11 uses RGB format
        backgroundColor = 0x000000; // Black
        textColor = 0xFFFFFF;      // White
        selectionColor = 0x333333;  // Dark gray
        borderColor = 0x888888;    // Gray
#endif
        
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
            std::cout << "Theme file not found for theme: " << theme << ", using default colors\n";
            return;
        }
        
        std::cout << "Loading theme from: " << themePath << "\n";
        
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
        struct stat st = {};
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
            outFile << "    \"text\": \"#FFFFFF\",\n";
            outFile << "    \"selection\": \"#333333\",\n";
            outFile << "    \"border\": \"#444444\"\n";
            outFile << "  }\n";
            outFile << "}\n";
            outFile.close();
            std::cout << "Created default theme file: " << themeFile << "\n";
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
                    if (updateConfigValue(configKey, configValue)) {
                        std::cout << "DEBUG: updateConfigValue returned true, calling saveConfig()\n";
                        saveConfig();
                        std::cout << "Updated " << configKey << " = " << configValue << "\n";
                    } else {
                        std::cout << "DEBUG: updateConfigValue returned false\n";
                        std::cout << "Invalid value for " << configKey << ". Expected type: " << getConfigType(configKey) << "\n";
                    }
                } else {
                    std::cout << "DEBUG: Failed to parse config key from args\n";
                }
            } else {
                // Enter config selection mode: "config"
                commandMode = false;
                cmd_configSelectMode = true;
                discoverConfigs();
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

    void discoverConfigs() {
        availableConfigs.clear();

        std::string configFile = configDir + "/config.json";
       
        std::ifstream file(configFile);
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                // Look for lines containing config keys (in quotes)
                size_t start = line.find('"');
                if (start != std::string::npos && start != line.rfind('"')) {
                    size_t end = line.find('"', start + 1);
                    if (end != std::string::npos) {
                        std::string configKey = line.substr(start + 1, end - start - 1);
                        if (!configKey.empty()) {
                            availableConfigs.push_back(configKey);
                        }
                    }
                }
            }
            file.close();
        }
        
        // Reset selection
        selectedConfig = 0;
        configSelectScrollOffset = 0;
    }

    void switchTheme(const std::string& themeName) {
        theme = themeName;
        loadTheme();
        std::cout << "Switched to theme: " << themeName << "\n";
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
        struct stat st = {};
        if (stat(configDir.c_str(), &st) == -1) {
            if (createDirectory(configDir)) {
                std::cout << "Created config directory: " << configDir << "\n";
            } else {
                std::cerr << "Failed to create config directory: " << configDir << "\n";
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
                std::cout << "Created themes directory: " << themesDir << "\n";
            } else {
                std::cerr << "Failed to create themes directory: " << themesDir << "\n";
            }
        }
        
        // Create bookmarks directory if it doesn't exist
        bookmarksDir = configDir + pathSep + "bookmarks";
        if (stat(bookmarksDir.c_str(), &st) == -1) {
            if (createDirectory(bookmarksDir)) {
                std::cout << "Created bookmarks directory: " << bookmarksDir << "\n";
            } else {
                std::cerr << "Failed to create bookmarks directory: " << bookmarksDir << "\n";
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
        struct stat st = {};
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
                std::cout << "Created bookmarks file: " << bookmarksFile << "\n";
            }
        }
        
        // Check and create clips.txt if needed
        if (stat(dataFile.c_str(), &st) == -1) {
            std::ofstream outFile(dataFile);
            if (outFile.is_open()) {
                outFile.close();
                std::cout << "Created clips file: " << dataFile << "\n";
            }
        }
        // Check and create pinned.txt if needed
        if (stat(pinnedFile.c_str(), &st) == -1) {
            std::ofstream outFile(pinnedFile);
            if (outFile.is_open()) {
                outFile.close();
                std::cout << "Created pinned clips file: " << pinnedFile << "\n";
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
    
    



//// HELP TOPICS ///////////////////////////////////////////////////////////////
///
///
#ifdef __linux__
    void drawHelpTopic(HDC /*hdc*/, int x, int y, int contentTop, int contentBottom, const std::string& topic) {
        if (y >= contentTop && y < contentBottom) {
            XDrawString(display, window, gc, x, y, topic.c_str(), topic.length());
        }
#endif

#ifdef _WIN32
    void drawHelpTopic(HDC hdc, int x, int y, int contentTop, int contentBottom, const std::string& topic) {
        if (hdc && y >= contentTop && y < contentBottom) {
            RECT rc;
            rc.left   = x;
            rc.top    = y - 12;     // baseline to bounding box adjustment
            rc.right  = x + 2000;   // wide; parent clipping restricts it
            rc.bottom = y + 4;

            DrawTextA(
                hdc,
                topic.c_str(),
                (int)topic.length(),
                &rc,
                DT_LEFT | DT_NOPREFIX | DT_SINGLELINE
            );
        }
#endif

#ifdef __APPLE__
        // (If you eventually implement macOS)
#endif
    }

    void drawAllHelpTopics(HDC hdc, int titleLeft, int topicLeft, int lineHeight, int gap, int y, int contentTop, int contentBottom) {
        // Main Window shortcuts
        drawHelpTopic(hdc, titleLeft, y, contentTop, contentBottom, "Main Window:");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "j/k            - Navigate items");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "g/G            - Top/bottom");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "/              - Filter mode");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "Shift+m        - Manage bookmark groups");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "m              - Add clip to group");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "`              - View bookmarks");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "p              - Pin clip");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "'              - View pinned clips");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "i              - Edit current clip");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "?              - This help");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "Shift+d        - Delete item");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "Shift+q        - Quit");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "Enter          - Copy item");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "Escape         - Hide window");
        y += lineHeight + gap;
        
        // Filter Mode shortcuts
        drawHelpTopic(hdc, titleLeft, y, contentTop, contentBottom, "Filter Mode:");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "Type text      - Filter items");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "Backspace      - Delete char");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "Up/down arrow  - Navigate items");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "Delete         - Delete item");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "Enter          - Copy item");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "Escape         - Exit filter");
        y += lineHeight + gap;

        // Pinned clips shortcuts
        drawHelpTopic(hdc, titleLeft, y, contentTop, contentBottom, "Pinned Clips:");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "j/k            - Navigate items");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "g/G            - Top/bottom");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "Shift+d        - Delete item");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "Enter          - Copy item");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "Escape         - Exit pinned clips");
        y += lineHeight + gap;
        
        // Add bookmark group shortcuts
        drawHelpTopic(hdc, titleLeft, y, contentTop, contentBottom, "Add Bookmark Group Dialog:");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "Type text      - Define Group Name / Filter Existing");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "Backspace      - Delete char");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "Enter          - Create group");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "Escape         - Exit dialog");
        y += lineHeight + gap;

        // Add clip to group shortcuts
        drawHelpTopic(hdc, titleLeft, y, contentTop, contentBottom, "Add Clip to Group Dialog:");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "j/k            - Navigate group");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "g/G            - Top/bottom");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "/              - Begin filtering groups");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "Enter          - Add clip to group");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "Escape         - Exit filtering / Exit dialog");
        y += lineHeight + gap;

        // View/Edit/Use bookmarks
        drawHelpTopic(hdc, titleLeft, y, contentTop, contentBottom, "View/Delete/Use Bookmarks Dialog");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "j/k            - Navigate groups/clips");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "g/G            - Top/bottom");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "h              - Back to groups list");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "Shift+d        - Delete item");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "/              - Begin filtering groups");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "Enter          - View group clips/copy clip");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "Escape         - Exit filtering / Exit dialog");
        y += lineHeight + gap + 5;

        // Commands
        drawHelpTopic(hdc, titleLeft, y, contentTop, contentBottom, "Commands");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, ":              - Activate commands");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "theme          - Select theme to apply");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "config         - Select config option or modify with: config key value");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "Enter          - Select config or apply change");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "Example: config max_clips 1000");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "Escape         - Cancel command");
        y += lineHeight + gap + 5;

        // Help
        drawHelpTopic(hdc, titleLeft, y, contentTop, contentBottom, "Help Window:");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "j/k            - Navigate topics");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "g              - Top");
        y += lineHeight + gap;

        // Global hotkey
        drawHelpTopic(hdc, titleLeft, y, contentTop, contentBottom, "Global Hotkey:");
        y += lineHeight;
        drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "Ctrl+Alt+C     - Show/hide window");
    }
///
///
//// END HELP TOPICS ///////////////////////////////////////////////////////////




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
            XSetWindowBackground(display, window, backgroundColor);
            XClearWindow(display, window);
            
            // Draw filter or command textbox if in respective mode
            int startY = 20;
            if (filterMode) {
                // Draw filter input
                std::string filterDisplay = "/" + filterText;
                XDrawString(display, window, gc, 10, startY, filterDisplay.c_str(), filterDisplay.length());
                startY += LINE_HEIGHT;
            }
            else if (commandMode) {
                // Draw command input
                std::string commandDisplay = ":" + commandText;
                XDrawString(display, window, gc, 10, startY, commandDisplay.c_str(), commandDisplay.length());
                startY += LINE_HEIGHT;
            }
            else if (cmd_themeSelectMode) {
                // Draw theme selection header
                std::string header = "Select theme (" + std::to_string(availableThemes.size()) + " total):";
                XDrawString(display, window, gc, 10, startY, header.c_str(), header.length());
                startY += LINE_HEIGHT;
                
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
            else if (cmd_configSelectMode) {
                // Draw config selection header
                std::string header = "Select config option (" + std::to_string(availableConfigs.size()) + " total):";
                XDrawString(display, window, gc, 10, startY, header.c_str(), header.length());
                startY += LINE_HEIGHT;
                
                // Draw config list
                const int VISIBLE_CONFIGS = 10;
                size_t startIdx = configSelectScrollOffset;
                size_t endIdx = std::min(startIdx + VISIBLE_CONFIGS, availableConfigs.size());
                
                for (size_t i = startIdx; i < endIdx; ++i) {
                    std::string configDisplay = (i == selectedConfig ? "> " : "  ") + availableConfigs[i];
                    XDrawString(display, window, gc, 10, startY, configDisplay.c_str(), configDisplay.length());
                    startY += LINE_HEIGHT;
                }
                
                // Show scroll indicator if there are more configs
                if (availableConfigs.size() > VISIBLE_CONFIGS) {
                    std::string scrollInfo = "Showing " + std::to_string(startIdx + 1) + "-" + std::to_string(endIdx) + " of " + std::to_string(availableConfigs.size());
                    XDrawString(display, window, gc, 10, startY, scrollInfo.c_str(), scrollInfo.length());
                }
                
                // Don't draw clipboard items in config selection mode
                return;
            }
            
            // Draw clipboard items
            // TODO: Remove all the hardcoded values
            int y = startY;
            size_t displayCount = getDisplayItemCount();
            const int SCROLL_INDICATOR_HEIGHT = 15;
            int availableHeight = windowHeight - startY - 10;

            // If we need a scroll indicator, account for its space
            if (static_cast<int>(displayCount) > (availableHeight / LINE_HEIGHT)) {
                availableHeight -= SCROLL_INDICATOR_HEIGHT;
            }

            int maxItems = availableHeight / LINE_HEIGHT;
            if (maxItems > 0) maxItems += 1;
            if (maxItems < 1) maxItems = 1;

            size_t startIdx = consoleScrollOffset;
            size_t endIdx = std::min(startIdx + maxItems, displayCount);

            // Adjust for the scroll indicator
            if (static_cast<int>(displayCount) > maxItems) {
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
                    if (static_cast<int>(content.length()) > maxContentLength) {
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
                    if (static_cast<int>(content.length()) > maxContentLength) {
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
            drawEditDialog();
        }

        void drawBookmarkDialog() {
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
        }

        void drawAddToBookmarkDialog() {
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

            // Filter groups if in filter mode
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

            // Adjust selection if out of bounds
            if (selectedAddBookmarkGroup >= displayedGroups.size() && !displayedGroups.empty()) {
                selectedAddBookmarkGroup = displayedGroups.size() - 1;
            }
            
            size_t startIdx = addBookmarkScrollOffset;
            size_t endIdx = std::min(startIdx + VISIBLE_ITEMS, displayedGroups.size());
            
            for (size_t i = startIdx; i < endIdx; ++i) {
                std::string displayText = "  " + displayedGroups[i];
                if (i == selectedAddBookmarkGroup) {
                    displayText = "> " + displayedGroups[i];
                    // Highlight selected
                    XSetForeground(display, gc, selectionColor);
                    XFillRectangle(display, window, gc, dims.x + 15, y - 12, dims.width - 30, 15);
                    XSetForeground(display, gc, textColor);
                }
                XDrawString(display, window, gc, dims.x + 20, y, displayText.c_str(), displayText.length());
                y += 18;
            }

            if (filterAddBookmarksMode) {
                std::string filterDisplay = "Filter: /" + filterAddBookmarksText + "_";
                XDrawString(display, window, gc, dims.x + 20, dims.y + dims.height - 20, filterDisplay.c_str(), filterDisplay.length());
            }
        }

        void drawViewBookmarksDialog() {
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
                            
                            // Filter groups if in filter mode
                            std::vector<std::string> displayedGroups;
                            if (filterBookmarksMode) {
                                for (const auto& group : bookmarkGroups) {
                                    if (group.find(filterBookmarksText) != std::string::npos) {
                                        displayedGroups.push_back(group);
                                    }
                                }
                            } else {
                                displayedGroups = bookmarkGroups;
                            }
            
                            // Adjust selection if out of bounds
                            if (selectedViewBookmarkGroup >= displayedGroups.size() && !displayedGroups.empty()) {
                                selectedViewBookmarkGroup = displayedGroups.size() - 1;
                            }
            
                            const int VISIBLE_ITEMS = 15;
                            size_t startIdx = viewBookmarksScrollOffset;
                            size_t endIdx = std::min(startIdx + VISIBLE_ITEMS, displayedGroups.size());
                            
                            for (size_t i = startIdx; i < endIdx; ++i) {
                                std::string displayText = displayedGroups[i];
                                
                                if (i == selectedViewBookmarkGroup) {
                                    displayText = "> " + displayText;
                                    XSetForeground(display, gc, selectionColor);
                                    XFillRectangle(display, window, gc, dims.x + 15, y - 12, dims.width - 30, 15);
                                    XSetForeground(display, gc, textColor);
                                } else {
                                    displayText = "  " + displayText;
                                }
                                
                                XDrawString(display, window, gc, dims.x + 20, y, displayText.c_str(), displayText.length());
                                y += 18;
                            }
            
                            if (filterBookmarksMode) {
                                std::string filterDisplay = "Filter: /" + filterBookmarksText + "_";
                                XDrawString(display, window, gc, dims.x + 20, dims.y + dims.height - 20, filterDisplay.c_str(), filterDisplay.length());
                            }
            
                        } else {
                            // Show clips for the selected group
                            std::string selectedGroup = bookmarkGroups[selectedViewBookmarkGroup];
                            std::string bookmarkFile = bookmarksDir + "/bookmarks_" + selectedGroup + ".txt";
                            std::ifstream file(bookmarkFile);
                            
                            if (file.is_open()) {
                                std::vector<std::string> bookmarkItems;
                                std::string line;
                                while (std::getline(file, line)) {
                                    size_t pos = line.find('|');
                                    if (pos != std::string::npos && pos > 0) {
                                        bookmarkItems.push_back(line.substr(pos + 1));
                                    }
                                }
                                file.close();
                                
                                XSetForeground(display, gc, textColor);
                                int y = dims.y + 60;
                                const int VISIBLE_ITEMS = 20;
                                int maxDialogContentLength = calculateDialogContentLength(dims);
            
                                size_t startIdx = viewBookmarksScrollOffset;
                                size_t endIdx = std::min(startIdx + VISIBLE_ITEMS, bookmarkItems.size());
            
                                if (selectedViewBookmarkItem >= bookmarkItems.size() && !bookmarkItems.empty()) {
                                     selectedViewBookmarkItem = bookmarkItems.size() - 1;
                                }
                                
                                for (size_t i = startIdx; i < endIdx; ++i) {
                                    std::string content = bookmarkItems[i];
                                    try {
                                        content = decrypt(content);
                                    } catch (...) {
                                        // Leave as is
                                    }
                                    
                                    std::string displayText = content;
                                    for (char& c : displayText) {
                                        if (c == '\n' || c == '\r') c = ' ';
                                    }
            
                                    if (static_cast<int>(displayText.length()) > maxDialogContentLength) {
                                        displayText = smartTrim(displayText, maxDialogContentLength);
                                    }
                                    
                                    if (i == selectedViewBookmarkItem) {
                                        displayText = "> " + displayText;
                                        XSetForeground(display, gc, selectionColor);
                                        XFillRectangle(display, window, gc, dims.x + 15, y - 12, dims.width - 30, 15);
                                        XSetForeground(display, gc, textColor);
                                    } else {
                                        displayText = "  " + displayText;
                                    }
                                    
                                    XDrawString(display, window, gc, dims.x + 20, y, displayText.c_str(), displayText.length());
                                    y += 18;
                                }
                            }
                        }
                    }
        void drawPinnedDialog() {
            if (!pinnedDialogVisible) return;
            
            // Get sorted pinned items using our helper method
            auto sortedItems = getSortedPinnedItems();
            
            // Get dynamic dialog dimensions
            int numItems = sortedItems.size();
            if (numItems == 0) {
                numItems = 1; // for the "No pinned clips" message
            }
            int preferredHeight = (numItems * LINE_HEIGHT) + 80;
            DialogDimensions dims = calculateDialogDimensions(windowWidth-40, preferredHeight);
            
            // Draw dialog background
            XSetForeground(display, gc, backgroundColor);
            XFillRectangle(display, window, gc, dims.x, dims.y, dims.width, dims.height);
            XSetForeground(display, gc, borderColor);
            XDrawRectangle(display, window, gc, dims.x, dims.y, dims.width, dims.height);
            
            // Draw title
            std::string title = "Pinned Clips";
            int titleWidth = XTextWidth(font, title.c_str(), title.length());
            XDrawString(display, window, gc, dims.x + (dims.width - titleWidth) / 2, dims.y + 25, title.c_str(), title.length());
            
            // Parse items for display (decrypt content)
            std::vector<std::pair<long long, std::string>> displayItems;
            for (const auto& line : sortedItems) {
                size_t pos = line.find('|');
                if (pos != std::string::npos && pos > 0) {
                    std::string timestampStr = line.substr(0, pos);
                    std::string content = line.substr(pos + 1);
                    try {
                        std::string decryptedContent = decrypt(content);
                        long long timestamp = std::stoll(timestampStr);
                        displayItems.push_back({timestamp, decryptedContent});
                    } catch (...) {
                        try {
                            long long timestamp = std::stoll(timestampStr);
                            displayItems.push_back({timestamp, content});
                        } catch (...) {
                            // If timestamp parsing fails, use current time
                            long long timestamp = std::chrono::system_clock::now().time_since_epoch().count();
                            displayItems.push_back({timestamp, content});
                        }
                    }
                }
            }
            
            // Draw items with scrolling
            int itemY = dims.y + 60;
            int maxVisibleItems = dims.contentHeight / LINE_HEIGHT;
            if (maxVisibleItems < 1) maxVisibleItems = 1; // Ensure at least one item is visible
            m_maxVisiblePinnedItems = maxVisibleItems; // Store for scrolling calculations
            
            size_t startIdx = viewPinnedScrollOffset;
            size_t endIdx = std::min(startIdx + maxVisibleItems, displayItems.size());
            
            for (size_t i = startIdx; i < endIdx; ++i) {
                std::string displayText = displayItems[i].second;
        
                // Truncate if too long
                int maxContentLength = calculateDialogContentLength(dims);
                if (static_cast<int>(displayText.length()) > maxContentLength) {
                    displayText = smartTrim(displayText, maxContentLength);
                }
                
                // Replace newlines with spaces for display
                for (char& c : displayText) {
                    if (c == '\n' || c == '\r') c = ' ';
                }
                
                // Add selection indicator
                if (i == selectedViewPinnedItem) {
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
                itemY += LINE_HEIGHT;
            }
            
            if (displayItems.empty()) {
                XDrawString(display, window, gc, dims.x + 20, itemY, "No pinned clips", 16);
            }
        }

        void drawHelpDialog() {
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

            drawAllHelpTopics(nullptr, titleLeft, topicLeft, lineHeight, gap, y, contentTop, contentBottom);
        }

        void drawEditDialog() {
            if (!editDialogVisible) return;
            
            // Get dynamic dialog dimensions
            DialogDimensions dims = getEditDialogDimensions();
            
            // Draw dialog background
            XSetForeground(display, gc, backgroundColor);
            XFillRectangle(display, window, gc, dims.x, dims.y, dims.width, dims.height);
            XSetForeground(display, gc, borderColor);
            XDrawRectangle(display, window, gc, dims.x, dims.y, dims.width, dims.height);
            
            // Draw title
            std::string title = "Edit Clip (CTRL+ENTER to save, ESC to cancel)";
            int titleWidth = XTextWidth(font, title.c_str(), title.length());
            XDrawString(display, window, gc, dims.x + (dims.width - titleWidth) / 2, dims.y + 25, title.c_str(), title.length());
            
            // Draw input box
            XSetForeground(display, gc, backgroundColor);
            XFillRectangle(display, window, gc, dims.x + 20, dims.y + 50, dims.width - 40, dims.height - 70);
            XSetForeground(display, gc, textColor);
            XDrawRectangle(display, window, gc, dims.x + 20, dims.y + 50, dims.width - 40, dims.height - 70);
            
            // Draw input text (multi-line with wrapping)
            const int lineHeight = 15;
            const int charWidth = 8; // Estimate for monospace font
            int maxCharsPerLine = (dims.width - 50) / charWidth; // 25px margin on each side
            if (maxCharsPerLine < 1) maxCharsPerLine = 1;

            std::istringstream iss(editDialogInput);
            std::string logicalLine;
            
            int logicalLineIndex = 0;

            std::vector<std::string> visualLines;
            std::vector<std::pair<int, int>> visualToLogicalMap;

            // First, break the text into visual lines
            while (std::getline(iss, logicalLine)) {
                if (logicalLine.empty()) {
                    visualLines.push_back("");
                    visualToLogicalMap.push_back({logicalLineIndex, 0});
                } else {
                    size_t offset = 0;
                    while (offset < logicalLine.length()) {
                        visualLines.push_back(logicalLine.substr(offset, maxCharsPerLine));
                        visualToLogicalMap.push_back({logicalLineIndex, (int)offset});
                        offset += maxCharsPerLine;
                    }
                }
                logicalLineIndex++;
            }
            if (editDialogInput.empty()) {
                 visualLines.push_back("");
                 visualToLogicalMap.push_back({0, 0});
            } else if (editDialogInput.back() == '\n') {
                 visualLines.push_back("");
                 visualToLogicalMap.push_back({logicalLineIndex, 0});
            }


            // Draw the visual lines
            for (size_t i = 0; i < visualLines.size(); ++i) {
                if ((int)i >= editDialogScrollOffset) {
                    // Calculate Y position accounting for scroll offset
                    int adjustedY = dims.y + 65 + ((int)i - editDialogScrollOffset) * lineHeight;
                    
                    // Only draw if within visible area
                    if (adjustedY < dims.y + dims.height - 20) {
                        std::string displayText = visualLines[i];
                        
                        // Check if cursor is on this line
                        auto logicalPos = visualToLogicalMap[i];
                        if (logicalPos.first == (int)editDialogCursorLine) {
                            size_t cursorInLine = editDialogCursorPos;
                            size_t lineStartOffset = logicalPos.second;
                            size_t lineEndOffset = lineStartOffset + visualLines[i].length();

                            if(cursorInLine >= lineStartOffset && cursorInLine <= lineEndOffset) {
                                size_t cursorInSub = cursorInLine - lineStartOffset;
                                 if (cursorInSub <= displayText.length()) {
                                    displayText.insert(cursorInSub, "^");
                                } else {
                                    displayText += "^";
                                }
                            }
                        }
                        XDrawString(display, window, gc, dims.x + 25, adjustedY, displayText.c_str(), displayText.length());
                    }
                }
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
            
            // Clear window with theme background
            HBRUSH hBgBrush = CreateSolidBrush(backgroundColor);
            FillRect(hdc, &clientRect, hBgBrush);
            DeleteObject(hBgBrush);
            
            // Set text color
            SetTextColor(hdc, textColor);
            SetBkMode(hdc, TRANSPARENT);
            
            // Draw filter or command textbox if in respective mode
            int startY = 20;
            if (filterMode) {
                // Draw filter input
                std::string filterDisplay = "/" + filterText;
                TextOut(hdc, 10, startY, filterDisplay.c_str(), filterDisplay.length());
                startY += LINE_HEIGHT;
            }
            else if (commandMode) {
                // Draw command input
                std::string commandDisplay = ":" + commandText;
                TextOut(hdc, 10, startY, commandDisplay.c_str(), commandDisplay.length());
                startY += LINE_HEIGHT;
            }
            else if (cmd_themeSelectMode) {
                // Draw theme selection header
                std::string header = "Select theme (" + std::to_string(availableThemes.size()) + " total):";
                TextOut(hdc, 10, startY, header.c_str(), header.length());
                startY += LINE_HEIGHT;
                
                // Draw theme list
                const int VISIBLE_THEMES = 10;
                size_t startIdx = themeSelectScrollOffset;
                size_t endIdx = std::min(startIdx + VISIBLE_THEMES, availableThemes.size());
                
                for (size_t i = startIdx; i < endIdx; ++i) {
                    std::string themeDisplay = (i == selectedTheme ? "> " : "  ") + availableThemes[i];
                    // Highlight selected theme
                    if (i == selectedTheme) {
                        RECT highlightRect = {5, startY - WIN_SEL_RECT_OFFSET_Y, getClipListWidth(), startY - WIN_SEL_RECT_OFFSET_Y + WIN_SEL_RECT_HEIGHT};
                        HBRUSH hHighlightBrush = CreateSolidBrush(selectionColor);
                        FillRect(hdc, &highlightRect, hHighlightBrush);
                        DeleteObject(hHighlightBrush);
                    }
                    
                    // Ensure text color is set before drawing
                    SetTextColor(hdc, textColor);
                    TextOut(hdc, 10, startY, themeDisplay.c_str(), themeDisplay.length());
                    startY += LINE_HEIGHT;
                }
                
                // Show scroll indicator if there are more themes
                if (availableThemes.size() > VISIBLE_THEMES) {
                    std::string scrollInfo = "Showing " + std::to_string(startIdx + 1) + "-" + std::to_string(endIdx) + " of " + std::to_string(availableThemes.size());
                    TextOut(hdc, 10, startY, scrollInfo.c_str(), scrollInfo.length());
                }
                
                // Cleanup and return
                SelectObject(hdc, hOldFont);
                DeleteObject(hFont);
                ReleaseDC(hwnd, hdc);
                return;
            }
            else if (cmd_configSelectMode) {
                // Create font for drawing
                HFONT hFont = CreateFont(FONT_SIZE, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, 
                                       DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 
                                       CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, FONT_NAME.c_str());
                HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
                
                // Draw config selection header
                std::string header = "Select config option (" + std::to_string(availableConfigs.size()) + " total):";
                TextOut(hdc, 10, startY, header.c_str(), header.length());
                startY += LINE_HEIGHT;
                
                // Draw config list
                const int VISIBLE_CONFIGS = 10;
                size_t startIdx = configSelectScrollOffset;
                size_t endIdx = std::min(startIdx + VISIBLE_CONFIGS, availableConfigs.size());
                
                for (size_t i = startIdx; i < endIdx; ++i) {
                    std::string configDisplay = (i == selectedConfig ? "> " : "  ") + availableConfigs[i];
                    // Highlight selected config
                    if (i == selectedConfig) {
                        RECT highlightRect = {5, startY - WIN_SEL_RECT_OFFSET_Y, getClipListWidth(), startY - WIN_SEL_RECT_OFFSET_Y + WIN_SEL_RECT_HEIGHT};
                        HBRUSH hHighlightBrush = CreateSolidBrush(selectionColor);
                        FillRect(hdc, &highlightRect, hHighlightBrush);
                        DeleteObject(hHighlightBrush);
                    }
                    
                    // Ensure text color is set before drawing
                    SetTextColor(hdc, textColor);
                    TextOut(hdc, 10, startY, configDisplay.c_str(), configDisplay.length());
                    startY += LINE_HEIGHT;
                }
                
                // Show scroll indicator if there are more configs
                if (availableConfigs.size() > VISIBLE_CONFIGS) {
                    std::string scrollInfo = "Showing " + std::to_string(startIdx + 1) + "-" + std::to_string(endIdx) + " of " + std::to_string(availableConfigs.size());
                    TextOut(hdc, 10, startY, scrollInfo.c_str(), scrollInfo.length());
                }
                
                // Cleanup and return
                SelectObject(hdc, hOldFont);
                DeleteObject(hFont);
                ReleaseDC(hwnd, hdc);
                return;
            }
            
            // Draw clipboard items
            int y = startY;
            size_t displayCount = getDisplayItemCount();
            const int SCROLL_INDICATOR_HEIGHT = 15;
            int availableHeight = windowHeight - startY - 10;

            // If we need a scroll indicator, account for its space
            if (static_cast<int>(displayCount) > (availableHeight / LINE_HEIGHT)) {
                availableHeight -= SCROLL_INDICATOR_HEIGHT;
            }

            int maxItems = availableHeight / LINE_HEIGHT;
            if (maxItems > 0) maxItems += 1;
            if (maxItems < 1) maxItems = 1;

            size_t startIdx = consoleScrollOffset;
            size_t endIdx = std::min(startIdx + maxItems, displayCount);

            // Adjust for the scroll indicator
            if (static_cast<int>(displayCount) > maxItems) {
                std::string scrollText = "[" + std::to_string(selectedItem + 1) + "/" + std::to_string(displayCount) + "]";
                TextOut(hdc, windowWidth - 80, 15, scrollText.c_str(), scrollText.length());
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
                    if (static_cast<int>(content.length()) > maxContentLength) {
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
                    if (static_cast<int>(content.length()) > maxContentLength) {
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
                    RECT highlightRect = {5, y - WIN_SEL_RECT_OFFSET_Y, getClipListWidth(), y - WIN_SEL_RECT_OFFSET_Y + WIN_SEL_RECT_HEIGHT};
                    HBRUSH hHighlightBrush = CreateSolidBrush(selectionColor);
                    FillRect(hdc, &highlightRect, hHighlightBrush);
                    DeleteObject(hHighlightBrush);
                }
                
                // Ensure text color is set before drawing (matches Linux behavior)
                SetTextColor(hdc, textColor);
                TextOut(hdc, 10, y, line.c_str(), line.length());
                
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
                // Ensure text color is set before drawing
                SetTextColor(hdc, textColor);
                TextOut(hdc, 10, y, empty.c_str(), empty.length());
            }
            
            // Draw dialogs if visible
            drawBookmarkDialog(hdc);
            drawAddToBookmarkDialog(hdc);
            drawViewBookmarksDialog(hdc);
            drawPinnedDialog(hdc);
            drawHelpDialog(hdc);
            drawEditDialog(hdc);
            
            // Cleanup
            SelectObject(hdc, hOldFont);
            DeleteObject(hFont);
            ReleaseDC(hwnd, hdc);
        }

        void drawBookmarkDialog(HDC hdc) {
            if (!bookmarkDialogVisible) return;
            
            // Create and select font
            HFONT hFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                   DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                   CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Consolas");
            HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
            SetBkMode(hdc, TRANSPARENT);
            
            // Get dynamic dialog dimensions
            DialogDimensions dims = getBookmarkDialogDimensions();
            
            // Draw dialog background
            HBRUSH hBgBrush = CreateSolidBrush(backgroundColor);
            RECT bgRect = {dims.x, dims.y, dims.x + dims.width, dims.y + dims.height};
            FillRect(hdc, &bgRect, hBgBrush);
            DeleteObject(hBgBrush);
            
            // --- Draw border safely (manually, to avoid Win32 Rectangle() quirks) ---
            HPEN hBorderPen = CreatePen(PS_SOLID, 1, borderColor);
            HPEN hOldPen = (HPEN)SelectObject(hdc, hBorderPen);
            HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

            MoveToEx(hdc, dims.x,               dims.y,                NULL);
            LineTo(hdc,   dims.x + dims.width,  dims.y);
            LineTo(hdc,   dims.x + dims.width,  dims.y + dims.height);
            LineTo(hdc,   dims.x,               dims.y + dims.height);
            LineTo(hdc,   dims.x,               dims.y);

            SelectObject(hdc, hOldBrush);
            SelectObject(hdc, hOldPen);
            DeleteObject(hBorderPen);
            
            // Draw title
            std::string title = "Bookmark Groups";
            SetTextColor(hdc, textColor);
            TextOut(hdc, dims.x + 20, dims.y + 25, title.c_str(), title.length());
            
            // Draw input field label
            TextOut(hdc, dims.x + 20, dims.y + 60, "New group name:", 16);
            
            // Draw input box
            HBRUSH hInputBrush = CreateSolidBrush(selectionColor);
            RECT inputRect = {dims.x + 20, dims.y + 70, dims.x + dims.width - 20, dims.y + 95};
            FillRect(hdc, &inputRect, hInputBrush);
            DeleteObject(hInputBrush);
            
            // Draw input box border
            HPEN hInputPen = CreatePen(PS_SOLID, 1, textColor);
            hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
            hOldPen = (HPEN)SelectObject(hdc, hInputPen);
            Rectangle(hdc, dims.x + 20, dims.y + 70, dims.x + dims.width - 20, dims.y + 95);
            SelectObject(hdc, hOldPen);
            SelectObject(hdc, hOldBrush);
            DeleteObject(hInputPen);
            
            // Draw input text
            std::string displayInput = bookmarkDialogInput + "_";
            TextOut(hdc, dims.x + 25, dims.y + 75, displayInput.c_str(), displayInput.length());
            
            // Draw existing groups label
            TextOut(hdc, dims.x + 20, dims.y + 120, "Existing groups:", 15);
            
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
                    // Highlight selected
                    HBRUSH hHighlightBrush = CreateSolidBrush(selectionColor);
                    RECT highlightRect = {dims.x + 15, y - WIN_SEL_RECT_OFFSET_Y, dims.x + dims.width - 15, y - WIN_SEL_RECT_OFFSET_Y + WIN_SEL_RECT_HEIGHT};
                    FillRect(hdc, &highlightRect, hHighlightBrush);
                    DeleteObject(hHighlightBrush);
                }
                
                SetTextColor(hdc, textColor);
                TextOut(hdc, dims.x + 20, y, displayText.c_str(), displayText.length());
                y += 18;
            }
            
            // Cleanup
            SelectObject(hdc, hOldFont);
            DeleteObject(hFont);
        }

        void drawAddToBookmarkDialog(HDC hdc) {
            if (!addToBookmarkDialogVisible) return;
            
            // Get dynamic dialog dimensions
            DialogDimensions dims = getAddBookmarkDialogDimensions();
            
            // Draw dialog background
            HBRUSH hBgBrush = CreateSolidBrush(backgroundColor);
            RECT bgRect = {dims.x, dims.y, dims.x + dims.width, dims.y + dims.height};
            FillRect(hdc, &bgRect, hBgBrush);
            DeleteObject(hBgBrush);
            
            // --- Draw border safely ---
            HPEN hBorderPen = CreatePen(PS_SOLID, 1, borderColor);
            HPEN hOldPen = (HPEN)SelectObject(hdc, hBorderPen);
            HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

            MoveToEx(hdc, dims.x,               dims.y,                NULL);
            LineTo(hdc,   dims.x + dims.width,  dims.y);
            LineTo(hdc,   dims.x + dims.width,  dims.y + dims.height);
            LineTo(hdc,   dims.x,               dims.y + dims.height);
            LineTo(hdc,   dims.x,               dims.y);

            SelectObject(hdc, hOldBrush);
            SelectObject(hdc, hOldPen);
            DeleteObject(hBorderPen);
            
            // Draw title
            std::string title = "Add to Bookmark Group";
            int titleWidth = 150; // Approximate width
            SetTextColor(hdc, textColor);
            TextOut(hdc, dims.x + (dims.width - titleWidth) / 2, dims.y + 25, title.c_str(), title.length());
            
            // Filter and display groups
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

            // Adjust selection if out of bounds
            if (selectedAddBookmarkGroup >= displayedGroups.size() && !displayedGroups.empty()) {
                selectedAddBookmarkGroup = displayedGroups.size() - 1;
            }

            int y = dims.y + 50;
            const int VISIBLE_ITEMS = 10;
            
            size_t startIdx = addBookmarkScrollOffset;
            size_t endIdx = std::min(startIdx + VISIBLE_ITEMS, displayedGroups.size());
            
            for (size_t i = startIdx; i < endIdx; ++i) {
                std::string displayText = "  " + displayedGroups[i];
                if (i == selectedAddBookmarkGroup) {
                    displayText = "> " + displayedGroups[i];
                    HBRUSH hHighlightBrush = CreateSolidBrush(selectionColor);
                    RECT highlightRect = {dims.x + 15, y - WIN_SEL_RECT_OFFSET_Y, dims.x + dims.width - 15, y - WIN_SEL_RECT_OFFSET_Y + WIN_SEL_RECT_HEIGHT};
                    FillRect(hdc, &highlightRect, hHighlightBrush);
                    DeleteObject(hHighlightBrush);
                }
                
                SetTextColor(hdc, textColor);
                TextOut(hdc, dims.x + 20, y, displayText.c_str(), displayText.length());
                y += 18;
            }

            if (filterAddBookmarksMode) {
                std::string filterDisplay = "Filter: /" + filterAddBookmarksText + "_";
                TextOut(hdc, dims.x + 20, dims.y + dims.height - 20, filterDisplay.c_str(), filterDisplay.length());
            }
        }

        void drawViewBookmarksDialog(HDC hdc) {
            if (!viewBookmarksDialogVisible) return;
            
            // Get dynamic dialog dimensions
            DialogDimensions dims = getViewBookmarksDialogDimensions();
            
            // Draw dialog background
            HBRUSH hBgBrush = CreateSolidBrush(backgroundColor);
            RECT bgRect = {dims.x, dims.y, dims.x + dims.width, dims.y + dims.height};
            FillRect(hdc, &bgRect, hBgBrush);
            DeleteObject(hBgBrush);
            
            // --- Draw border safely (manually, to avoid Win32 Rectangle() quirks) ---
            HPEN hBorderPen = CreatePen(PS_SOLID, 1, borderColor);
            HPEN hOldPen = (HPEN)SelectObject(hdc, hBorderPen);
            HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

            MoveToEx(hdc, dims.x,               dims.y,                NULL);
            LineTo(hdc,   dims.x + dims.width,  dims.y);
            LineTo(hdc,   dims.x + dims.width,  dims.y + dims.height);
            LineTo(hdc,   dims.x,               dims.y + dims.height);
            LineTo(hdc,   dims.x,               dims.y);

            SelectObject(hdc, hOldBrush);
            SelectObject(hdc, hOldPen);
            DeleteObject(hBorderPen);
            
            // Draw title
            std::string title = viewBookmarksShowingGroups ? "Select Bookmark Group" : "View Bookmarks: " + bookmarkGroups[selectedViewBookmarkGroup];
            int titleWidth = 200; // Approximate width
            SetTextColor(hdc, textColor);
            TextOut(hdc, dims.x + (dims.width - titleWidth) / 2, dims.y + 25, title.c_str(), title.length());
            
            if (viewBookmarksShowingGroups) {
                // Show bookmark groups list with scrolling
                SetTextColor(hdc, textColor);
                int y = dims.y + 60;
                
                // Filter groups if in filter mode
                std::vector<std::string> displayedGroups;
                if (filterBookmarksMode) {
                    for (const auto& group : bookmarkGroups) {
                        if (group.find(filterBookmarksText) != std::string::npos) {
                            displayedGroups.push_back(group);
                        }
                    }
                } else {
                    displayedGroups = bookmarkGroups;
                }

                // Adjust selection if out of bounds
                if (selectedViewBookmarkGroup >= displayedGroups.size() && !displayedGroups.empty()) {
                    selectedViewBookmarkGroup = displayedGroups.size() - 1;
                }

                int dynamicVisibleItems = std::max(1, (dims.contentHeight - (y - dims.y)) / LINE_HEIGHT);
                
                size_t startIdx = viewBookmarksScrollOffset;
                size_t endIdx = std::min(startIdx + dynamicVisibleItems, displayedGroups.size());
                
                for (size_t i = startIdx; i < endIdx; ++i) {
                    std::string displayText = displayedGroups[i];
                    
                    if (i == selectedViewBookmarkGroup) {
                        displayText = "> " + displayText;
                        HBRUSH hHighlightBrush = CreateSolidBrush(selectionColor);
                        RECT highlightRect = {dims.x + 15, y - WIN_SEL_RECT_OFFSET_Y, dims.x + dims.width - 15, y - WIN_SEL_RECT_OFFSET_Y + WIN_SEL_RECT_HEIGHT};
                        FillRect(hdc, &highlightRect, hHighlightBrush);
                        DeleteObject(hHighlightBrush);
                    } else {
                        displayText = "  " + displayText;
                    }
                    
                    SetTextColor(hdc, textColor);
                    TextOut(hdc, dims.x + 20, y, displayText.c_str(), displayText.length());
                    y += LINE_HEIGHT;
                }

                if (filterBookmarksMode) {
                    std::string filterDisplay = "Filter: /" + filterBookmarksText + "_";
                    TextOut(hdc, dims.x + 20, dims.y + dims.height - 20, filterDisplay.c_str(), filterDisplay.length());
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
                        // Calculate dynamic VISIBLE_ITEMS for clips view
                        // Uses global LINE_HEIGHT for consistent spacing across dialogs
                        int dynamicVisibleItems = std::max(1, (dims.contentHeight - (itemY - dims.y)) / LINE_HEIGHT);
                        
                        size_t startIdx = viewBookmarksScrollOffset;
                        size_t endIdx = std::min(startIdx + dynamicVisibleItems, bookmarkItems.size());
                        
                        for (size_t i = startIdx; i < endIdx; ++i) {
                            std::string displayText = bookmarkItems[i];
                            
                            // Truncate if too long
                            int maxContentLength = calculateDialogContentLength(dims);
                            if (static_cast<int>(displayText.length()) > maxContentLength) {
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
                                HBRUSH hHighlightBrush = CreateSolidBrush(selectionColor);
                                RECT highlightRect = {dims.x + 15, itemY - WIN_SEL_RECT_OFFSET_Y, dims.x + dims.width - 15, itemY - WIN_SEL_RECT_OFFSET_Y + WIN_SEL_RECT_HEIGHT};
                                FillRect(hdc, &highlightRect, hHighlightBrush);
                                DeleteObject(hHighlightBrush);
                            } else {
                                displayText = "  " + displayText;
                            }
                            
                            // Ensure text color is set before drawing
                            SetTextColor(hdc, textColor);
                            TextOut(hdc, dims.x + 20, itemY, displayText.c_str(), displayText.length());
                            itemY += LINE_HEIGHT;
                        }
                        
                        if (bookmarkItems.empty()) {
                            // Ensure text color is set before drawing
                            SetTextColor(hdc, textColor);
                            TextOut(hdc, dims.x + 20, itemY, "No bookmarks in this group", 26);
                        }
                    }
                }
            }
        }

        void drawPinnedDialog(HDC hdc) {
            if (!pinnedDialogVisible) return;
            
            // Get sorted pinned items using our helper method
            auto sortedItems = getSortedPinnedItems();
            
            // Get dynamic dialog dimensions
            int numItems = sortedItems.size();
            if (numItems == 0) {
                numItems = 1; // for the "No pinned clips" message
            }
            int preferredHeight = (numItems * LINE_HEIGHT) + 80;
            DialogDimensions dims = calculateDialogDimensions(windowWidth-40, preferredHeight);
            
            // Draw dialog background
            HBRUSH hBgBrush = CreateSolidBrush(backgroundColor);
            RECT bgRect = {dims.x, dims.y, dims.x + dims.width, dims.y + dims.height};
            FillRect(hdc, &bgRect, hBgBrush);
            DeleteObject(hBgBrush);
            
            // --- Draw border safely (manually, to avoid Win32 Rectangle() quirks) ---
            HPEN hBorderPen = CreatePen(PS_SOLID, 1, borderColor);
            HPEN hOldPen = (HPEN)SelectObject(hdc, hBorderPen);
            HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

            MoveToEx(hdc, dims.x,               dims.y,                NULL);
            LineTo(hdc,   dims.x + dims.width,  dims.y);
            LineTo(hdc,   dims.x + dims.width,  dims.y + dims.height);
            LineTo(hdc,   dims.x,               dims.y + dims.height);
            LineTo(hdc,   dims.x,               dims.y);

            SelectObject(hdc, hOldBrush);
            SelectObject(hdc, hOldPen);
            DeleteObject(hBorderPen);
            
            // Draw title
            std::string title = "Pinned Clips";
            int titleWidth = 100; // Approximate width
            SetTextColor(hdc, textColor);
            TextOut(hdc, dims.x + (dims.width - titleWidth) / 2, dims.y + 25, title.c_str(), title.length());
            
            // Parse items for display (decrypt content)
            std::vector<std::pair<long long, std::string>> displayItems;
            for (const auto& line : sortedItems) {
                size_t pos = line.find('|');
                if (pos != std::string::npos && pos > 0) {
                    std::string timestampStr = line.substr(0, pos);
                    std::string content = line.substr(pos + 1);
                    try {
                        std::string decryptedContent = decrypt(content);
                        long long timestamp = std::stoll(timestampStr);
                        displayItems.push_back({timestamp, decryptedContent});
                    } catch (...) {
                        try {
                            long long timestamp = std::stoll(timestampStr);
                            displayItems.push_back({timestamp, content});
                        } catch (...) {
                            // If timestamp parsing fails, use current time
                            long long timestamp = std::chrono::system_clock::now().time_since_epoch().count();
                            displayItems.push_back({timestamp, content});
                        }
                    }
                }
            }
            
            // Draw items with scrolling
            int itemY = dims.y + 60;
            int maxVisibleItems = dims.contentHeight / LINE_HEIGHT;
            if (maxVisibleItems < 1) maxVisibleItems = 1; // Ensure at least one item is visible
            m_maxVisiblePinnedItems = maxVisibleItems; // Store for scrolling calculations
            
            size_t startIdx = viewPinnedScrollOffset;
            size_t endIdx = std::min(startIdx + maxVisibleItems, displayItems.size());
            
            for (size_t i = startIdx; i < endIdx; ++i) {
                std::string displayText = displayItems[i].second;
        
                // Truncate if too long
                int maxContentLength = calculateDialogContentLength(dims);
                if (static_cast<int>(displayText.length()) > maxContentLength) {
                    displayText = smartTrim(displayText, maxContentLength);
                }
                
                // Replace newlines with spaces for display
                for (char& c : displayText) {
                    if (c == '\n' || c == '\r') c = ' ';
                }
                
                // Add selection indicator
                if (i == selectedViewPinnedItem) {
                    displayText = "> " + displayText;
                    // Highlight selected
                    HBRUSH hHighlightBrush = CreateSolidBrush(selectionColor);
                    RECT highlightRect = {dims.x + 15, itemY - WIN_SEL_RECT_OFFSET_Y, dims.x + dims.width - 15, itemY - WIN_SEL_RECT_OFFSET_Y + WIN_SEL_RECT_HEIGHT};
                    FillRect(hdc, &highlightRect, hHighlightBrush);
                    DeleteObject(hHighlightBrush);
                } else {
                    displayText = "  " + displayText;
                }
                
                // Ensure text color is set before drawing
                SetTextColor(hdc, textColor);
                TextOut(hdc, dims.x + 20, itemY, displayText.c_str(), displayText.length());
                itemY += LINE_HEIGHT;
            }
            
            if (displayItems.empty()) {
                // Ensure text color is set before drawing
                SetTextColor(hdc, textColor);
                TextOut(hdc, dims.x + 20, itemY, "No pinned clips", 16);
            }
        }

        void drawHelpDialog(HDC hdc) {
            if (!helpDialogVisible)
                return;

            DialogDimensions dims = getHelpDialogDimensions();

            // Background rectangle
            RECT bgRect = {
                dims.x,
                dims.y,
                dims.x + dims.width,
                dims.y + dims.height
            };

            // --- Draw background safely ---
            HBRUSH hBgBrush = CreateSolidBrush(backgroundColor);
            FillRect(hdc, &bgRect, hBgBrush);
            DeleteObject(hBgBrush);

            // --- Draw border safely (manually, to avoid Win32 Rectangle() quirks) ---
            HPEN hBorderPen = CreatePen(PS_SOLID, 1, borderColor);
            HPEN hOldPen = (HPEN)SelectObject(hdc, hBorderPen);
            HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

            MoveToEx(hdc, dims.x,               dims.y,                NULL);
            LineTo(hdc,   dims.x + dims.width,  dims.y);
            LineTo(hdc,   dims.x + dims.width,  dims.y + dims.height);
            LineTo(hdc,   dims.x,               dims.y + dims.height);
            LineTo(hdc,   dims.x,               dims.y);

            SelectObject(hdc, hOldBrush);
            SelectObject(hdc, hOldPen);
            DeleteObject(hBorderPen);

            // --- Setup clipping so text never draws outside dialog ---
            HRGN clipRegion = CreateRectRgn(
                dims.x + 5,              // a little inset from border
                dims.y + 5,
                dims.x + dims.width - 5,
                dims.y + dims.height - 5
            );

            int oldClip = SelectClipRgn(hdc, clipRegion);
            DeleteObject(clipRegion);

            // --- Text settings ---
            SetTextColor(hdc, textColor);
            SetBkMode(hdc, TRANSPARENT);

            const int contentTop = dims.y + 20;
            const int contentBottom = dims.y + dims.height;
            const int titleLeft = dims.x + 20;
            const int topicLeft = dims.x + 30;
            const int lineHeight = 15;
            const int gap = 10;

            // Scroll offset clamping (optional, but recommended)
            int y = contentTop + helpDialogScrollOffset;

            // --- Draw help topics ---
            drawAllHelpTopics(
                hdc,
                titleLeft,
                topicLeft,
                lineHeight,
                gap,
                y,
                contentTop,
                contentBottom
            );

            // Restore clipping region (VERY important)
            SelectClipRgn(hdc, oldClip == NULLREGION ? nullptr : reinterpret_cast<HRGN>(oldClip));
        }

        void drawEditDialog(HDC hdc) {
            if (!editDialogVisible) return;
            
            // Create and select font
            HFONT hFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                   DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                   CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Consolas");
            HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
            SetBkMode(hdc, TRANSPARENT);
            
            // Get dynamic dialog dimensions
            DialogDimensions dims = getEditDialogDimensions();
            
            // Draw dialog background
            HBRUSH hBgBrush = CreateSolidBrush(backgroundColor);
            RECT bgRect = {dims.x, dims.y, dims.x + dims.width, dims.y + dims.height};
            FillRect(hdc, &bgRect, hBgBrush);
            DeleteObject(hBgBrush);
            
            // --- Draw border safely (manually, to avoid Win32 Rectangle() quirks) ---
            HPEN hBorderPen = CreatePen(PS_SOLID, 1, borderColor);
            HPEN hOldPen_border = (HPEN)SelectObject(hdc, hBorderPen);
            HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

            MoveToEx(hdc, dims.x,               dims.y,                NULL);
            LineTo(hdc,   dims.x + dims.width,  dims.y);
            LineTo(hdc,   dims.x + dims.width,  dims.y + dims.height);
            LineTo(hdc,   dims.x,               dims.y + dims.height);
            LineTo(hdc,   dims.x,               dims.y);

            SelectObject(hdc, hOldBrush);
            SelectObject(hdc, hOldPen_border);
            DeleteObject(hBorderPen);
            
            // Draw title
            std::string title = "Edit Clip (CTRL+ENTER to save, ESC to cancel)";
            SetTextColor(hdc, textColor);
            TextOut(hdc, dims.x + 20, dims.y + 25, title.c_str(), title.length());
            
            // Draw input box
            HBRUSH hInputBrush = CreateSolidBrush(backgroundColor);
            RECT inputRect = {dims.x + 20, dims.y + 50, dims.x + dims.width - 20, dims.y + dims.height - 20};
            FillRect(hdc, &inputRect, hInputBrush);
            DeleteObject(hInputBrush);
            
            // Draw input box border
            HPEN hInputPen = CreatePen(PS_SOLID, 1, textColor);
            hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
            HPEN hOldPen_input = (HPEN)SelectObject(hdc, hInputPen);
            Rectangle(hdc, dims.x + 20, dims.y + 50, dims.x + dims.width - 20, dims.y + dims.height - 20);
            SelectObject(hdc, hOldPen_input);
            SelectObject(hdc, hOldBrush);
            DeleteObject(hInputPen);
            
            // Draw input text (multi-line with wrapping)
            const int lineHeight = 15;
            const int charWidth = 8; // Estimate for monospace font
            int maxCharsPerLine = (dims.width - 50) / charWidth;
            if (maxCharsPerLine < 1) maxCharsPerLine = 1;

            std::istringstream iss(editDialogInput);
            std::string logicalLine;
            
            int logicalLineIndex = 0;
            int y = dims.y + 55;

            std::vector<std::string> visualLines;
            std::vector<std::pair<int, int>> visualToLogicalMap;

            // First, break the text into visual lines
            while (std::getline(iss, logicalLine)) {
                if (logicalLine.empty()) {
                    visualLines.push_back("");
                    visualToLogicalMap.push_back({logicalLineIndex, 0});
                } else {
                    size_t offset = 0;
                    while (offset < logicalLine.length()) {
                        visualLines.push_back(logicalLine.substr(offset, maxCharsPerLine));
                        visualToLogicalMap.push_back({logicalLineIndex, (int)offset});
                        offset += maxCharsPerLine;
                    }
                }
                logicalLineIndex++;
            }
            if (editDialogInput.empty()) {
                 visualLines.push_back("");
                 visualToLogicalMap.push_back({0, 0});
            } else if (editDialogInput.back() == '\n') {
                 visualLines.push_back("");
                 visualToLogicalMap.push_back({logicalLineIndex, 0});
            }

            // Draw the visual lines
            for (size_t i = 0; i < visualLines.size(); ++i) {
                if ((int)i >= editDialogScrollOffset && y < dims.y + dims.height - 20 - lineHeight) {
                    std::string displayText = visualLines[i];
                    
                    // Check if cursor is on this line
                    auto logicalPos = visualToLogicalMap[i];
                    if (logicalPos.first == (int)editDialogCursorLine) {
                        size_t cursorInLine = editDialogCursorPos;
                        size_t lineStartOffset = logicalPos.second;
                        size_t lineEndOffset = lineStartOffset + visualLines[i].length();

                        if(cursorInLine >= lineStartOffset && cursorInLine <= lineEndOffset) {
                            size_t cursorInSub = cursorInLine - lineStartOffset;
                             if (cursorInSub <= displayText.length()) {
                                displayText.insert(cursorInSub, "^");
                            } else {
                                displayText += "^";
                            }
                        }
                    }
                    SetTextColor(hdc, textColor);
                    TextOut(hdc, dims.x + 25, y, displayText.c_str(), displayText.length());
                    y += lineHeight;
                }
            }
            
            // Cleanup
            SelectObject(hdc, hOldFont);
            DeleteObject(hFont);
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
        while (items.size() > maxClips) {
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
                // Parse debugging
                else if (line.find("\"debugging\"") != std::string::npos) {
                    m_debugging = line.find("true") != std::string::npos;
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
        std::cout << "DEBUG: Saving to " << configFile << "\n";
        std::cout << "DEBUG: maxClips before save = " << maxClips << "\n";
        
        std::ofstream outFile(configFile, std::ios::trunc);
        
        if (!outFile.is_open()) {
            std::cout << "DEBUG: Failed to open file for writing\n";
            return;
        }
        
        std::cout << "DEBUG: File opened successfully\n";
        
        // Use a map to store all config values dynamically
        std::map<std::string, std::string> configValues;
        configValues["verbose"] = verboseMode ? "true" : "false";
        configValues["debugging"] = m_debugging ? "true" : "false";
        configValues["max_clips"] = std::to_string(maxClips);
        configValues["encrypted"] = encrypted ? "true" : "false";
        configValues["encryption_key"] = encryptionKey;
        configValues["autostart"] = autoStart ? "true" : "false";
        configValues["theme"] = theme;
        
        std::cout << "DEBUG: About to write max_clips = " << configValues["max_clips"] << "\n";
        
        outFile << "{\n";
        bool first = true;
        int writeCount = 0;
        for (const auto& pair : configValues) {
            if (!first) {
                outFile << ",\n";
            }
            first = false;
            
            // Check if value should be quoted (string) or not (boolean/number)
            if (pair.second == "true" || pair.second == "false") {
                // Boolean - don't quote
                outFile << "    \"" << pair.first << "\": " << pair.second;
            } else if (pair.second.find_first_not_of("0123456789") == std::string::npos) {
                // Number - don't quote
                outFile << "    \"" << pair.first << "\": " << pair.second;
            } else {
                // String - quote it
                outFile << "    \"" << pair.first << "\": \"" << pair.second << "\"";
            }
            writeCount++;
            std::cout << "DEBUG: Wrote config entry " << writeCount << ": " << pair.first << " = " << pair.second << "\n";
        }
        outFile << "\n}\n";
        outFile.flush();
        outFile.close();
        
        std::cout << "DEBUG: Save completed, wrote " << writeCount << " entries\n";
    }
    
    // Config value helper functions
    std::string getConfigValue(const std::string& configKey) {
        if (configKey == "verbose") return verboseMode ? "true" : "false";
        if (configKey == "debugging") return m_debugging ? "true" : "false";
        if (configKey == "max_clips") return std::to_string(maxClips);
        if (configKey == "encrypted") return encrypted ? "true" : "false";
        if (configKey == "encryption_key") return encryptionKey;
        if (configKey == "autostart") return autoStart ? "true" : "false";
        if (configKey == "theme") return theme;
        return "";
    }
    
    std::string getConfigType(const std::string& configKey) {
        if (configKey == "verbose" || configKey == "debugging" || 
            configKey == "encrypted" || configKey == "autostart") {
            return "boolean (true/false)";
        }
        if (configKey == "max_clips") {
            return "number (positive integer)";
        }
        if (configKey == "encryption_key" || configKey == "theme") {
            return "string";
        }
        return "unknown";
    }
    
    bool updateConfigValue(const std::string& configKey, const std::string& newValue) {
        try {
            // Get current value to determine type
            std::string currentValue = getConfigValue(configKey);
            
            // Boolean values
            if (currentValue == "true" || currentValue == "false") {
                if (newValue == "true" || newValue == "false") {
                    // Update the specific boolean variable
                    if (configKey == "verbose") verboseMode = newValue == "true";
                    else if (configKey == "debugging") m_debugging = newValue == "true";
                    else if (configKey == "encrypted") encrypted = newValue == "true";
                    else if (configKey == "autostart") autoStart = newValue == "true";
                    return true;
                }
                return false;
            }
            
            // Try to parse as number first
            try {
                std::stoull(currentValue); // Just to check if it's a number
                // If current value is a number, expect new value to be a number
                size_t newNumValue = std::stoull(newValue);
                if (newNumValue > 0) {
                    // Update the specific numeric variable
                    if (configKey == "max_clips") {
                        std::cout << "DEBUG: Updating maxClips from " << maxClips << " to " << newNumValue << "\n";
                        maxClips = newNumValue;
                        std::cout << "DEBUG: maxClips is now " << maxClips << "\n";
                    }
                    return true;
                }
                return false;
            }
            catch (...) {
                // Not a number, treat as string
                if (configKey == "encryption_key") {
                    encryptionKey = newValue;
                    return true;
                }
                else if (configKey == "theme") {
                    theme = newValue;
                    loadTheme(); // Apply theme immediately
                    return true;
                }
                // For any other string values
                return true;
            }
        }
        catch (const std::exception& e) {
            return false;
        }
    }
    
    void createDefaultConfig() {
        std::string configFile = configDir + "/config.json";
        std::ofstream outFile(configFile);
        outFile << "{\n";
        outFile << "    \"debugging\": false,\n"; // Default debugging to true
        outFile << "    \"verbose\": false,\n";
        outFile << "    \"max_clips\": 500,\n";
        outFile << "    \"encrypted\": true,\n";
        outFile << "    \"encryption_key\": \"mmry_default_key_2024\",\n";
        outFile << "    \"autostart\": false,\n";
        outFile << "    \"theme\": \"console\"\n";
        outFile << "}\n";
        outFile.close();
        
        std::cout << "Created default config at: " << configFile << "\n";
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
                std::cout << "Created empty clips file: " << dataFile << "\n";
            } else {
                std::cerr << "Failed to create clips file: " << dataFile << "\n";
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
        case WM_SIZE:
            if (manager) {
                int newWidth = LOWORD(lParam);
                int newHeight = HIWORD(lParam);
                manager->updateWindowDimensions(newWidth, newHeight);
                manager->drawConsole();
            }
            return 0;
        case WM_KEYDOWN:
            // Process WM_KEYDOWN only - this prevents double processing
            if (manager) {
                MSG winMsg = {hwnd, msg, wParam, lParam, 0, 0, 0};
                manager->handleKeyPressCommon(&winMsg);
            }
            return 0;
        case WM_CHAR:
            // Skip WM_CHAR to prevent double processing
            // All key handling is done via WM_KEYDOWN
            return 0;
        case WM_PAINT:
            if (manager) {
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
            if (manager) {
                if (OpenClipboard(hwnd)) {
                    if (IsClipboardFormatAvailable(CF_TEXT)) {
                        HANDLE hData = GetClipboardData(CF_TEXT);
                        if (hData) {
                            char* pszText = static_cast<char*>(GlobalLock(hData));
                            if (pszText) {
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
            if (manager) {
                RemoveClipboardFormatListener(hwnd);
            }
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
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        std::cerr << "Another instance is already running. Exiting.\n";
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
