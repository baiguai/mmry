#include "../clipboard_manager.h"

void ClipboardManager::key_edit_add_char(char c)
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

bool ClipboardManager::key_edit_add_newline()
{
    key_edit_add_char('\n');
    editDialogCursorLine++;
    editDialogCursorPos = 0; // Ensure cursor is at the beginning of the new line
    updateEditDialogScrollOffset();
    return true;
}

bool ClipboardManager::key_edit_backspace()
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

bool ClipboardManager::key_edit_delete()
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

bool ClipboardManager::key_edit_save()
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

bool ClipboardManager::key_edit_scroll_up()
{
    if (editDialogScrollOffset > 0)
    {
        editDialogScrollOffset--;
        drawConsole();
    }
    return true;
}

bool ClipboardManager::key_edit_scroll_down()
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
