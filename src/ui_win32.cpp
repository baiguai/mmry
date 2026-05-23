#include "ui.h"
#include <sstream>

#ifdef _WIN32

void drawPinnedDialog(
    HDC hdc,
    const std::vector<std::pair<long long, std::string>>& displayItems,
    const DialogDimensions& dims,
    size_t& selectedItem, size_t scrollOffset, int& maxVisibleItems,
    unsigned long bgColor, unsigned long textColor,
    unsigned long selColor, unsigned long borderColor,
    int lineHeight, int winSelRectHeight, int winSelRectOffsetY)
{
    HBRUSH hBgBrush = CreateSolidBrush(bgColor);
    RECT bgRect = {dims.x, dims.y, dims.x + dims.width, dims.y + dims.height};
    FillRect(hdc, &bgRect, hBgBrush);
    DeleteObject(hBgBrush);

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

    std::string title = "Pinned Clips";
    int titleWidth = 100;
    SetTextColor(hdc, textColor);
    TextOut(hdc, dims.x + (dims.width - titleWidth) / 2, dims.y + 25, title.c_str(), title.length());

    int itemY = dims.y + 60;
    int visibleCount = dims.contentHeight / lineHeight;
    if (visibleCount < 1) visibleCount = 1;
    maxVisibleItems = visibleCount;

    size_t startIdx = scrollOffset;
    size_t endIdx = std::min(startIdx + visibleCount, displayItems.size());

    for (size_t i = startIdx; i < endIdx; ++i) {
        std::string displayText = displayItems[i].second;

        if (i == selectedItem) {
            displayText = "> " + displayText;
            HBRUSH hHighlightBrush = CreateSolidBrush(selColor);
            RECT highlightRect = {dims.x + 15, itemY - winSelRectOffsetY, dims.x + dims.width - 15, itemY - winSelRectOffsetY + winSelRectHeight};
            FillRect(hdc, &highlightRect, hHighlightBrush);
            DeleteObject(hHighlightBrush);
        } else {
            displayText = "  " + displayText;
        }

        SetTextColor(hdc, textColor);
        TextOut(hdc, dims.x + 20, itemY, displayText.c_str(), displayText.length());
        itemY += lineHeight;
    }

    if (displayItems.empty()) {
        SetTextColor(hdc, textColor);
        TextOut(hdc, dims.x + 20, itemY, "No pinned clips", 16);
    }
}

void drawBookmarkDialog(
    HDC hdc,
    const DialogDimensions& dims,
    const std::string& inputText,
    const std::vector<std::string>& filteredGroups,
    size_t selectedGroup, size_t scrollOffset,
    unsigned long bgColor, unsigned long textColor,
    unsigned long selColor, unsigned long borderColor,
    int winSelRectHeight, int winSelRectOffsetY)
{
    HFONT hFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                           CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Consolas");
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
    SetBkMode(hdc, TRANSPARENT);

    HBRUSH hBgBrush = CreateSolidBrush(bgColor);
    RECT bgRect = {dims.x, dims.y, dims.x + dims.width, dims.y + dims.height};
    FillRect(hdc, &bgRect, hBgBrush);
    DeleteObject(hBgBrush);

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

    std::string title = "Bookmark Groups";
    SetTextColor(hdc, textColor);
    TextOut(hdc, dims.x + 20, dims.y + 25, title.c_str(), title.length());

    TextOut(hdc, dims.x + 20, dims.y + 60, "New group name:", 16);

    HBRUSH hInputBrush = CreateSolidBrush(selColor);
    RECT inputRect = {dims.x + 20, dims.y + 70, dims.x + dims.width - 20, dims.y + 95};
    FillRect(hdc, &inputRect, hInputBrush);
    DeleteObject(hInputBrush);

    HPEN hInputPen = CreatePen(PS_SOLID, 1, textColor);
    hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    hOldPen = (HPEN)SelectObject(hdc, hInputPen);
    Rectangle(hdc, dims.x + 20, dims.y + 70, dims.x + dims.width - 20, dims.y + 95);
    SelectObject(hdc, hOldPen);
    SelectObject(hdc, hOldBrush);
    DeleteObject(hInputPen);

    std::string displayInput = inputText + "_";
    TextOut(hdc, dims.x + 25, dims.y + 75, displayInput.c_str(), displayInput.length());

    TextOut(hdc, dims.x + 20, dims.y + 120, "Existing groups:", 15);

    int y = dims.y + 140;
    const int VISIBLE_ITEMS = 8;

    size_t startIdx = scrollOffset;
    size_t endIdx = std::min(startIdx + VISIBLE_ITEMS, filteredGroups.size());

    for (size_t i = startIdx; i < endIdx; ++i) {
        std::string displayText = "  " + filteredGroups[i];

        if (i == selectedGroup) {
            HBRUSH hHighlightBrush = CreateSolidBrush(selColor);
            RECT highlightRect = {dims.x + 15, y - winSelRectOffsetY, dims.x + dims.width - 15, y - winSelRectOffsetY + winSelRectHeight};
            FillRect(hdc, &highlightRect, hHighlightBrush);
            DeleteObject(hHighlightBrush);
        }

        SetTextColor(hdc, textColor);
        TextOut(hdc, dims.x + 20, y, displayText.c_str(), displayText.length());
        y += 18;
    }

    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
}

void drawAddToBookmarkDialog(
    HDC hdc,
    const DialogDimensions& dims,
    const std::vector<std::string>& displayedGroups,
    size_t selectedGroup, size_t scrollOffset,
    bool filterMode, const std::string& filterText,
    unsigned long bgColor, unsigned long textColor,
    unsigned long selColor, unsigned long borderColor,
    int winSelRectHeight, int winSelRectOffsetY)
{
    HFONT hFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                           CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Consolas");
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
    SetBkMode(hdc, TRANSPARENT);

    HBRUSH hBgBrush = CreateSolidBrush(bgColor);
    RECT bgRect = {dims.x, dims.y, dims.x + dims.width, dims.y + dims.height};
    FillRect(hdc, &bgRect, hBgBrush);
    DeleteObject(hBgBrush);

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

    std::string title = "Add to Bookmark Group";
    int titleWidth = 150;
    SetTextColor(hdc, textColor);
    TextOut(hdc, dims.x + (dims.width - titleWidth) / 2, dims.y + 25, title.c_str(), title.length());

    int y = dims.y + 50;
    const int VISIBLE_ITEMS = 10;

    size_t startIdx = scrollOffset;
    size_t endIdx = std::min(startIdx + VISIBLE_ITEMS, displayedGroups.size());

    for (size_t i = startIdx; i < endIdx; ++i) {
        std::string displayText = "  " + displayedGroups[i];
        if (i == selectedGroup) {
            displayText = "> " + displayedGroups[i];
            HBRUSH hHighlightBrush = CreateSolidBrush(selColor);
            RECT highlightRect = {dims.x + 15, y - winSelRectOffsetY, dims.x + dims.width - 15, y - winSelRectOffsetY + winSelRectHeight};
            FillRect(hdc, &highlightRect, hHighlightBrush);
            DeleteObject(hHighlightBrush);
        }

        SetTextColor(hdc, textColor);
        TextOut(hdc, dims.x + 20, y, displayText.c_str(), displayText.length());
        y += 18;
    }

    if (filterMode) {
        std::string filterDisplay = "Filter: /" + filterText + "_";
        TextOut(hdc, dims.x + 20, dims.y + dims.height - 20, filterDisplay.c_str(), filterDisplay.length());
    }

    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
}

void drawViewBookmarksDialog(
    HDC hdc,
    const DialogDimensions& dims,
    const std::string& title,
    const std::vector<std::string>& items,
    size_t selectedItem, size_t scrollOffset,
    bool filterActive, const std::string& filterText,
    int itemLineHeight,
    const std::string& emptyMessage,
    unsigned long bgColor, unsigned long textColor,
    unsigned long selColor, unsigned long borderColor,
    int winSelRectHeight, int winSelRectOffsetY)
{
    HFONT hFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                           CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Consolas");
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
    SetBkMode(hdc, TRANSPARENT);

    HBRUSH hBgBrush = CreateSolidBrush(bgColor);
    RECT bgRect = {dims.x, dims.y, dims.x + dims.width, dims.y + dims.height};
    FillRect(hdc, &bgRect, hBgBrush);
    DeleteObject(hBgBrush);

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

    int titleWidth = 200;
    SetTextColor(hdc, textColor);
    TextOut(hdc, dims.x + (dims.width - titleWidth) / 2, dims.y + 25, title.c_str(), title.length());

    SetTextColor(hdc, textColor);
    int y = dims.y + 60;

    if (items.empty() && !emptyMessage.empty()) {
        TextOut(hdc, dims.x + 20, y, emptyMessage.c_str(), emptyMessage.length());
    } else {
        int visibleCount = std::max(1, (dims.contentHeight - (y - dims.y)) / itemLineHeight);

        size_t startIdx = scrollOffset;
        size_t endIdx = std::min(startIdx + visibleCount, items.size());

        for (size_t i = startIdx; i < endIdx; ++i) {
            std::string displayText = items[i];

            if (i == selectedItem) {
                displayText = "> " + displayText;
                HBRUSH hHighlightBrush = CreateSolidBrush(selColor);
                RECT highlightRect = {dims.x + 15, y - winSelRectOffsetY, dims.x + dims.width - 15, y - winSelRectOffsetY + winSelRectHeight};
                FillRect(hdc, &highlightRect, hHighlightBrush);
                DeleteObject(hHighlightBrush);
            } else {
                displayText = "  " + displayText;
            }

            SetTextColor(hdc, textColor);
            TextOut(hdc, dims.x + 20, y, displayText.c_str(), displayText.length());
            y += itemLineHeight;
        }
    }

    if (filterActive) {
        std::string filterDisplay = "Filter: /" + filterText + "_";
        TextOut(hdc, dims.x + 20, dims.y + dims.height - 20, filterDisplay.c_str(), filterDisplay.length());
    }

    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
}

void drawEditDialog(
    HDC hdc,
    const DialogDimensions& dims,
    const std::string& inputText,
    size_t cursorLine, size_t cursorPos,
    int scrollOffset,
    unsigned long bgColor, unsigned long textColor,
    unsigned long borderColor)
{
    HFONT hFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                           CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Consolas");
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
    SetBkMode(hdc, TRANSPARENT);

    HBRUSH hBgBrush = CreateSolidBrush(bgColor);
    RECT bgRect = {dims.x, dims.y, dims.x + dims.width, dims.y + dims.height};
    FillRect(hdc, &bgRect, hBgBrush);
    DeleteObject(hBgBrush);

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

    std::string title = "Edit Clip (CTRL+ENTER to save, ESC to cancel)";
    SetTextColor(hdc, textColor);
    TextOut(hdc, dims.x + 20, dims.y + 25, title.c_str(), title.length());

    HBRUSH hInputBrush = CreateSolidBrush(bgColor);
    RECT inputRect = {dims.x + 20, dims.y + 50, dims.x + dims.width - 20, dims.y + dims.height - 20};
    FillRect(hdc, &inputRect, hInputBrush);
    DeleteObject(hInputBrush);

    HPEN hInputPen = CreatePen(PS_SOLID, 1, textColor);
    hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    HPEN hOldPen_input = (HPEN)SelectObject(hdc, hInputPen);
    Rectangle(hdc, dims.x + 20, dims.y + 50, dims.x + dims.width - 20, dims.y + dims.height - 20);
    SelectObject(hdc, hOldPen_input);
    SelectObject(hdc, hOldBrush);
    DeleteObject(hInputPen);

    const int lineHeight = 15;
    const int charWidth = 8;
    int maxCharsPerLine = (dims.width - 50) / charWidth;
    if (maxCharsPerLine < 1) maxCharsPerLine = 1;

    std::istringstream iss(inputText);
    std::string logicalLine;

    int logicalLineIndex = 0;
    int y = dims.y + 55;

    std::vector<std::string> visualLines;
    std::vector<std::pair<int, int>> visualToLogicalMap;

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
    if (inputText.empty()) {
         visualLines.push_back("");
         visualToLogicalMap.push_back({0, 0});
    } else if (inputText.back() == '\n') {
         visualLines.push_back("");
         visualToLogicalMap.push_back({logicalLineIndex, 0});
    }

    for (size_t i = 0; i < visualLines.size(); ++i) {
        if ((int)i >= scrollOffset && y < dims.y + dims.height - 20 - lineHeight) {
            std::string displayText = visualLines[i];

            auto logicalPos = visualToLogicalMap[i];
            if (logicalPos.first == (int)cursorLine) {
                size_t cursorInLine = cursorPos;
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

    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
}

#endif
