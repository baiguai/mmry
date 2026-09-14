#include "../clipboard_manager.h"

#ifdef __linux__
    void ClipboardManager::handleKeyPress(XEvent* event)
    {
        handleKeyPressCommon(event);
    }
#endif

    void ClipboardManager::handleKeyPressCommon(void* eventPtr)
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


        //---- Repeat Tracking -------------------------------------------------
        if (key_value.size() == 1 && key_value[0] >= '1' && key_value[0] <= '9')
        {
            pendingRepeatCount = std::stoi(key_value);
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
                if (key_value == "CONTROL_DELETE")
                {
                    if (key_marks_groups_delete()) return;
                }

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

            if (key_value == "D")
            {
                if (key_marks_groups_delete()) return;
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
                    if (filterText.empty() || filterText[0] != '!')
                    {
                        updateFilteredItems();
                        selectedItem = 0;
                    }
                    else
                    {
                        regexSubmitted = false;
                    }
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
                if (!filterText.empty() && filterText[0] == '!')
                {
                    if (!regexSubmitted)
                    {
                        // First Enter: execute the regex search
                        updateFilteredItems();
                        regexSubmitted = true;
                        drawConsole();
                        return;
                    }
                    // Second Enter: copy the selected item
                    if (key_filter_copy()) return;
                }
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
                        if (filterText[0] != '!')
                        {
                            updateFilteredItems();
                            selectedItem = 0;
                        }
                        else
                        {
                            regexSubmitted = false;
                        }
                        drawConsole();
                    }
                }
#else
                // Original Linux part
                char buffer[10];
                int count = XLookupString(keyEvent, buffer, sizeof(buffer), nullptr, nullptr);
                filterText += std::string(buffer, count);
                if (filterText[0] != '!')
                {
                    updateFilteredItems();
                    selectedItem = 0;
                }
                else
                {
                    regexSubmitted = false;
                }
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
        int count = pendingRepeatCount > 0 ? pendingRepeatCount : 1;
        bool doReturn { false };


        if (key_value == "j" || key_value == "DOWN")
        {
            doReturn = false;
            std::cout << "count: " << count << "\n";
            for (int i = 0; i < count; i++)
            {
                if (key_main_down()) doReturn = true;
            }
            if (doReturn)
            {
                pendingRepeatCount = 0;
                return;
            }
        }

        if (key_value == "k" || key_value == "UP")
        {
            doReturn = false;
            for (int i = 0; i < count; i++)
            {
                if (key_main_up()) doReturn = true;
            }
            if (doReturn)
            {
                pendingRepeatCount = 0;
                return;
            }
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
