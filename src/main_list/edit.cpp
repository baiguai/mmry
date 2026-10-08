#include "../clipboard_manager.h"

bool ClipboardManager::key_main_edit_start()
{
    if (!items.empty() && selectedItem < getDisplayItemCount())
    {
        size_t actualIndex = getActualItemIndex(selectedItem);
        editDialogInput = items[actualIndex].content;
        editDialogVisible = true;
        editDialogScrollOffset = 0;

        // Initialize cursor position
        editDialogCursorLine = 0;
        editDialogCursorPos = 0;
        std::string lastLine;
        for (char c : editDialogInput)
        {
            if (c == '\n')
            {
                editDialogCursorLine++;
                lastLine.clear();
            }
            else
            {
                lastLine += c;
            }
        }
        editDialogCursorPos = lastLine.length();

        updateEditDialogScrollOffset();
        drawConsole();
    }
    return true;
}

bool ClipboardManager::key_edit_cursor_left()
{
    if (editDialogCursorPos > 0)
    {
        editDialogCursorPos--;
        drawConsole();
    }
    return true;
}

bool ClipboardManager::key_edit_cursor_right()
{
    std::string currentLine = "";
    std::istringstream iss(editDialogInput);
    for (size_t i = 0; i <= editDialogCursorLine; ++i)
    {
        std::getline(iss, currentLine);
    }

    if (editDialogCursorPos < currentLine.length())
    {
        editDialogCursorPos++;
        drawConsole();
    }
    return true;
}

bool ClipboardManager::key_edit_cursor_up()
{
    if (editDialogCursorLine > 0)
    {
        editDialogCursorLine--;
        std::string currentLine = "";
        std::istringstream iss(editDialogInput);
        for (size_t i = 0; i <= editDialogCursorLine; ++i)
        {
            std::getline(iss, currentLine);
        }
        if (editDialogCursorPos > currentLine.length())
        {
            editDialogCursorPos = currentLine.length();
        }
        updateEditDialogScrollOffset();
        drawConsole();
    }
    return true;
}

bool ClipboardManager::key_edit_cursor_down()
{
    size_t totalLines = 1;
    for (char c : editDialogInput)
    {
        if (c == '\n') totalLines++;
    }
    if (editDialogCursorLine < totalLines - 1)
    {
        editDialogCursorLine++;
        std::string currentLine = "";
        std::istringstream iss(editDialogInput);
        for (size_t i = 0; i <= editDialogCursorLine; ++i)
        {
            std::getline(iss, currentLine);
        }
        if (editDialogCursorPos > currentLine.length())
        {
            editDialogCursorPos = currentLine.length();
        }
        updateEditDialogScrollOffset();
        drawConsole();
    }
    return true;
}

bool ClipboardManager::key_edit_home()
{
    editDialogCursorPos = 0;
    updateEditDialogScrollOffset();
    drawConsole();
    return true;
}

bool ClipboardManager::key_edit_end()
{
    std::string currentLine = "";
    std::istringstream iss(editDialogInput);
    for (size_t i = 0; i <= editDialogCursorLine; ++i)
    {
        std::getline(iss, currentLine);
    }
    editDialogCursorPos = currentLine.length();
    updateEditDialogScrollOffset();
    drawConsole();
    return true;
}
