#include "ui.h"
#include "help.h"
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

void drawHelpDialog(
    HDC hdc,
    const DialogDimensions& dims,
    bool filterMode, const std::string& filterText,
    int scrollOffset,
    unsigned long bgColor, unsigned long textColor,
    unsigned long borderColor)
{
    RECT bgRect = {dims.x, dims.y, dims.x + dims.width, dims.y + dims.height};

    HBRUSH hBgBrush = CreateSolidBrush(bgColor);
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

    HRGN clipRegion = CreateRectRgn(
        dims.x + 5,
        dims.y + 5,
        dims.x + dims.width - 5,
        dims.y + dims.height - 5
    );

    int oldClip = SelectClipRgn(hdc, clipRegion);
    DeleteObject(clipRegion);

    SetTextColor(hdc, textColor);
    SetBkMode(hdc, TRANSPARENT);

    const int titleLeft = dims.x + 20;
    const int topicLeft = dims.x + 30;
    const int lineHeight = 15;
    const int gap = 10;

    int inputY = dims.y + 20;

    HPEN hInputPen = CreatePen(PS_SOLID, 1, filterMode ? textColor : borderColor);
    HPEN hOldPen_input = (HPEN)SelectObject(hdc, hInputPen);
    HBRUSH hOldBrush_input = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, dims.x + 20, inputY, dims.x + dims.width - 20, inputY + 20);
    SelectObject(hdc, hOldBrush_input);
    SelectObject(hdc, hOldPen_input);
    DeleteObject(hInputPen);

    std::string filterDisplay = "/" + filterText;
    TextOut(hdc, dims.x + 25, inputY + 4, filterDisplay.c_str(), filterDisplay.length());

    int y = dims.y + 20 + 25 + gap;
    const int contentTop = y;
    const int contentBottom = dims.y + dims.height;

    y = y + scrollOffset;

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

    SelectClipRgn(hdc, oldClip == NULLREGION ? nullptr : reinterpret_cast<HRGN>(oldClip));
}

void drawConsole(
    HDC hdc,
    const ConsoleDrawData& data,
    int winSelRectHeight, int winSelRectOffsetY)
{
    int y = data.startY;

    SetTextColor(hdc, data.textColor);
    SetBkMode(hdc, TRANSPARENT);

    if (data.filterMode) {
        std::string filterDisplay = "/" + data.filterText;
        TextOut(hdc, 10, y, filterDisplay.c_str(), filterDisplay.length());
        y += data.lineHeight;
    } else if (data.commandMode) {
        std::string commandDisplay = ":" + data.commandText;
        TextOut(hdc, 10, y, commandDisplay.c_str(), commandDisplay.length());
        y += data.lineHeight;
    }

    if (data.themeSelectMode) {
        std::string header = "Select theme (" + std::to_string(data.themeItems.size()) + " total):";
        TextOut(hdc, 10, y, header.c_str(), header.length());
        y += data.lineHeight;

        const int VISIBLE_THEMES = 10;
        size_t startIdx = data.themeScrollOffset;
        size_t endIdx = std::min(startIdx + VISIBLE_THEMES, data.themeItems.size());

        for (size_t i = startIdx; i < endIdx; ++i) {
            std::string themeDisplay = (i == data.selectedTheme ? "> " : "  ") + data.themeItems[i];

            if (i == data.selectedTheme) {
                RECT highlightRect = {5, y - winSelRectOffsetY, data.clipListWidth, y - winSelRectOffsetY + winSelRectHeight};
                HBRUSH hHighlightBrush = CreateSolidBrush(data.selColor);
                FillRect(hdc, &highlightRect, hHighlightBrush);
                DeleteObject(hHighlightBrush);
            }

            SetTextColor(hdc, data.textColor);
            TextOut(hdc, 10, y, themeDisplay.c_str(), themeDisplay.length());
            y += data.lineHeight;
        }

        if (data.themeItems.size() > VISIBLE_THEMES) {
            std::string scrollInfo = "Showing " + std::to_string(startIdx + 1) + "-" + std::to_string(endIdx) + " of " + std::to_string(data.themeItems.size());
            TextOut(hdc, 10, y, scrollInfo.c_str(), scrollInfo.length());
        }
        return;
    }

    if (data.configSelectMode) {
        std::string header = "Select config option (" + std::to_string(data.configItems.size()) + " total):";
        TextOut(hdc, 10, y, header.c_str(), header.length());
        y += data.lineHeight;

        const int VISIBLE_CONFIGS = 10;
        size_t startIdx = data.configScrollOffset;
        size_t endIdx = std::min(startIdx + VISIBLE_CONFIGS, data.configItems.size());

        for (size_t i = startIdx; i < endIdx; ++i) {
            std::string configDisplay = (i == data.selectedConfig ? "> " : "  ") + data.configItems[i];

            if (i == data.selectedConfig) {
                RECT highlightRect = {5, y - winSelRectOffsetY, data.clipListWidth, y - winSelRectOffsetY + winSelRectHeight};
                HBRUSH hHighlightBrush = CreateSolidBrush(data.selColor);
                FillRect(hdc, &highlightRect, hHighlightBrush);
                DeleteObject(hHighlightBrush);
            }

            SetTextColor(hdc, data.textColor);
            TextOut(hdc, 10, y, configDisplay.c_str(), configDisplay.length());
            y += data.lineHeight;
        }

        if (data.configItems.size() > VISIBLE_CONFIGS) {
            std::string scrollInfo = "Showing " + std::to_string(startIdx + 1) + "-" + std::to_string(endIdx) + " of " + std::to_string(data.configItems.size());
            TextOut(hdc, 10, y, scrollInfo.c_str(), scrollInfo.length());
        }
        return;
    }

    const int SCROLL_INDICATOR_HEIGHT = 15;

    bool needScrollIndicator = data.totalClipCount > data.clipLines.size();
    if (needScrollIndicator) {
        std::string scrollText = "[" + std::to_string(data.selectedItem + 1) + "/" + std::to_string(data.totalClipCount) + "]";
        SetTextColor(hdc, data.textColor);
        TextOut(hdc, data.windowWidth - 80, 15, scrollText.c_str(), scrollText.length());
        y += SCROLL_INDICATOR_HEIGHT;
    }

    for (size_t i = 0; i < data.clipLines.size(); ++i) {
        bool isSelected = (i + data.clipScrollOffset == data.selectedItem);

        if (isSelected) {
            RECT highlightRect = {5, y - winSelRectOffsetY, data.clipListWidth, y - winSelRectOffsetY + winSelRectHeight};
            HBRUSH hHighlightBrush = CreateSolidBrush(data.selColor);
            FillRect(hdc, &highlightRect, hHighlightBrush);
            DeleteObject(hHighlightBrush);
        }

        SetTextColor(hdc, data.textColor);
        TextOut(hdc, 10, y, data.clipLines[i].c_str(), data.clipLines[i].length());
        y += data.lineHeight;
    }

    if (data.clipLines.empty()) {
        std::string empty;
        if (data.filterMode) {
            empty = "No matching items...";
        } else if (data.commandMode) {
            empty = "Enter command...";
        } else {
            empty = "No clipboard items yet...";
        }
        SetTextColor(hdc, data.textColor);
        TextOut(hdc, 10, y, empty.c_str(), empty.length());
    }
}

#endif
