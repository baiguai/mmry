#include "ui.h"
#include "help.h"
#include <sstream>

#ifdef __linux__

void drawPinnedDialog(
    Display* display, Window window, GC gc, XFontStruct* font,
    const std::vector<std::pair<long long, std::string>>& displayItems,
    const DialogDimensions& dims,
    size_t& selectedItem,
    size_t scrollOffset,
    int& maxVisibleItems,
    unsigned long bgColor,
    unsigned long textColor,
    unsigned long selColor,
    unsigned long borderColor,
    int lineHeight)
{
    XSetForeground(display, gc, bgColor);
    XFillRectangle(display, window, gc, dims.x, dims.y, dims.width, dims.height);
    XSetForeground(display, gc, borderColor);
    XDrawRectangle(display, window, gc, dims.x, dims.y, dims.width, dims.height);

    std::string title = "Pinned Clips";
    int titleWidth = XTextWidth(font, title.c_str(), title.length());
    XDrawString(display, window, gc, dims.x + (dims.width - titleWidth) / 2, dims.y + 25, title.c_str(), title.length());

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
            XSetForeground(display, gc, selColor);
            XFillRectangle(display, window, gc, dims.x + 15, itemY - 12, dims.width - 30, 15);
            XSetForeground(display, gc, textColor);
        } else {
            XSetForeground(display, gc, selColor);
            displayText = "  " + displayText;
        }

        XDrawString(display, window, gc, dims.x + 20, itemY, displayText.c_str(), displayText.length());
        itemY += lineHeight;
    }

    if (displayItems.empty()) {
        XDrawString(display, window, gc, dims.x + 20, itemY, "No pinned clips", 16);
    }
}

void drawBookmarkDialog(
    Display* display, Window window, GC gc, XFontStruct* font,
    const DialogDimensions& dims,
    const std::string& inputText,
    const std::vector<std::string>& filteredGroups,
    size_t selectedGroup, size_t scrollOffset,
    unsigned long bgColor, unsigned long textColor,
    unsigned long selColor, unsigned long borderColor)
{
    XSetForeground(display, gc, bgColor);
    XFillRectangle(display, window, gc, dims.x, dims.y, dims.width, dims.height);
    XSetForeground(display, gc, borderColor);
    XDrawRectangle(display, window, gc, dims.x, dims.y, dims.width, dims.height);

    std::string title = "Bookmark Groups";
    int titleWidth = XTextWidth(font, title.c_str(), title.length());
    XDrawString(display, window, gc, dims.x + (dims.width - titleWidth) / 2, dims.y + 25, title.c_str(), title.length());

    XSetForeground(display, gc, textColor);
    XDrawString(display, window, gc, dims.x + 20, dims.y + 60, "New group name:", 16);

    XSetForeground(display, gc, selColor);
    XFillRectangle(display, window, gc, dims.x + 20, dims.y + 70, dims.width - 40, 25);
    XSetForeground(display, gc, textColor);
    XDrawRectangle(display, window, gc, dims.x + 20, dims.y + 70, dims.width - 40, 25);

    std::string displayInput = inputText + "_";
    XDrawString(display, window, gc, dims.x + 25, dims.y + 87, displayInput.c_str(), displayInput.length());

    XSetForeground(display, gc, textColor);
    XDrawString(display, window, gc, dims.x + 20, dims.y + 120, "Existing groups:", 15);

    int y = dims.y + 140;
    const int VISIBLE_ITEMS = 8;

    size_t startIdx = scrollOffset;
    size_t endIdx = std::min(startIdx + VISIBLE_ITEMS, filteredGroups.size());

    for (size_t i = startIdx; i < endIdx; ++i) {
        std::string displayText = "  " + filteredGroups[i];
        if (i == selectedGroup) {
            displayText = "  " + filteredGroups[i];
            XSetForeground(display, gc, selColor);
            XFillRectangle(display, window, gc, dims.x + 15, y - 12, dims.width - 30, 15);
            XSetForeground(display, gc, textColor);
        }
        XDrawString(display, window, gc, dims.x + 20, y, displayText.c_str(), displayText.length());
        y += 18;
    }
}

void drawAddToBookmarkDialog(
    Display* display, Window window, GC gc, XFontStruct* font,
    const DialogDimensions& dims,
    const std::vector<std::string>& displayedGroups,
    size_t selectedGroup, size_t scrollOffset,
    bool filterMode, const std::string& filterText,
    unsigned long bgColor, unsigned long textColor,
    unsigned long selColor, unsigned long borderColor)
{
    XSetForeground(display, gc, bgColor);
    XFillRectangle(display, window, gc, dims.x, dims.y, dims.width, dims.height);
    XSetForeground(display, gc, borderColor);
    XDrawRectangle(display, window, gc, dims.x, dims.y, dims.width, dims.height);

    std::string title = "Add to Bookmark Group";
    int titleWidth = XTextWidth(font, title.c_str(), title.length());
    XDrawString(display, window, gc, dims.x + (dims.width - titleWidth) / 2, dims.y + 25, title.c_str(), title.length());

    XSetForeground(display, gc, textColor);

    int y = dims.y + 50;
    const int VISIBLE_ITEMS = 10;

    size_t startIdx = scrollOffset;
    size_t endIdx = std::min(startIdx + VISIBLE_ITEMS, displayedGroups.size());

    for (size_t i = startIdx; i < endIdx; ++i) {
        std::string displayText = "  " + displayedGroups[i];
        if (i == selectedGroup) {
            displayText = "> " + displayedGroups[i];
            XSetForeground(display, gc, selColor);
            XFillRectangle(display, window, gc, dims.x + 15, y - 12, dims.width - 30, 15);
            XSetForeground(display, gc, textColor);
        }
        XDrawString(display, window, gc, dims.x + 20, y, displayText.c_str(), displayText.length());
        y += 18;
    }

    if (filterMode) {
        std::string filterDisplay = "Filter: /" + filterText + "_";
        XDrawString(display, window, gc, dims.x + 20, dims.y + dims.height - 20, filterDisplay.c_str(), filterDisplay.length());
    }
}

void drawViewBookmarksDialog(
    Display* display, Window window, GC gc, XFontStruct* font,
    const DialogDimensions& dims,
    const std::string& title,
    const std::vector<std::string>& items,
    size_t selectedItem, size_t scrollOffset,
    bool filterActive, const std::string& filterText,
    int itemLineHeight,
    const std::string& emptyMessage,
    unsigned long bgColor, unsigned long textColor,
    unsigned long selColor, unsigned long borderColor)
{
    XSetForeground(display, gc, bgColor);
    XFillRectangle(display, window, gc, dims.x, dims.y, dims.width, dims.height);
    XSetForeground(display, gc, borderColor);
    XDrawRectangle(display, window, gc, dims.x, dims.y, dims.width, dims.height);

    int titleWidth = XTextWidth(font, title.c_str(), title.length());
    XDrawString(display, window, gc, dims.x + (dims.width - titleWidth) / 2, dims.y + 25, title.c_str(), title.length());

    XSetForeground(display, gc, textColor);
    int y = dims.y + 60;

    if (items.empty() && !emptyMessage.empty()) {
        XDrawString(display, window, gc, dims.x + 20, y, emptyMessage.c_str(), emptyMessage.length());
    } else {
        const int VISIBLE_ITEMS = 15;
        size_t startIdx = scrollOffset;
        size_t endIdx = std::min(startIdx + VISIBLE_ITEMS, items.size());

        for (size_t i = startIdx; i < endIdx; ++i) {
            std::string displayText = items[i];

            if (i == selectedItem) {
                displayText = "> " + displayText;
                XSetForeground(display, gc, selColor);
                XFillRectangle(display, window, gc, dims.x + 15, y - 12, dims.width - 30, 15);
                XSetForeground(display, gc, textColor);
            } else {
                displayText = "  " + displayText;
            }

            XDrawString(display, window, gc, dims.x + 20, y, displayText.c_str(), displayText.length());
            y += itemLineHeight;
        }
    }

    if (filterActive) {
        std::string filterDisplay = "Filter: /" + filterText + "_";
        XDrawString(display, window, gc, dims.x + 20, dims.y + dims.height - 20, filterDisplay.c_str(), filterDisplay.length());
    }
}

void drawEditDialog(
    Display* display, Window window, GC gc, XFontStruct* font,
    const DialogDimensions& dims,
    const std::string& inputText,
    size_t cursorLine, size_t cursorPos,
    int scrollOffset,
    unsigned long bgColor, unsigned long textColor,
    unsigned long borderColor)
{
    XSetForeground(display, gc, bgColor);
    XFillRectangle(display, window, gc, dims.x, dims.y, dims.width, dims.height);
    XSetForeground(display, gc, borderColor);
    XDrawRectangle(display, window, gc, dims.x, dims.y, dims.width, dims.height);

    std::string title = "Edit Clip (CTRL+ENTER to save, ESC to cancel)";
    int titleWidth = XTextWidth(font, title.c_str(), title.length());
    XDrawString(display, window, gc, dims.x + (dims.width - titleWidth) / 2, dims.y + 25, title.c_str(), title.length());

    XSetForeground(display, gc, bgColor);
    XFillRectangle(display, window, gc, dims.x + 20, dims.y + 50, dims.width - 40, dims.height - 70);
    XSetForeground(display, gc, textColor);
    XDrawRectangle(display, window, gc, dims.x + 20, dims.y + 50, dims.width - 40, dims.height - 70);

    const int lineHeight = 15;
    const int charWidth = 8;
    int maxCharsPerLine = (dims.width - 50) / charWidth;
    if (maxCharsPerLine < 1) maxCharsPerLine = 1;

    std::istringstream iss(inputText);
    std::string logicalLine;

    int logicalLineIndex = 0;

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
        if ((int)i >= scrollOffset) {
            int adjustedY = dims.y + 65 + ((int)i - scrollOffset) * lineHeight;

            if (adjustedY < dims.y + dims.height - 20) {
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
                XDrawString(display, window, gc, dims.x + 25, adjustedY, displayText.c_str(), displayText.length());
            }
        }
    }
}

void drawHelpDialog(
    Display* display, Window window, GC gc,
    const DialogDimensions& dims,
    bool filterMode, const std::string& filterText,
    int scrollOffset,
    unsigned long bgColor, unsigned long textColor,
    unsigned long borderColor)
{
    XSetForeground(display, gc, bgColor);
    XFillRectangle(display, window, gc, dims.x, dims.y, dims.width, dims.height);
    XSetForeground(display, gc, borderColor);
    XDrawRectangle(display, window, gc, dims.x, dims.y, dims.width, dims.height);

    XSetForeground(display, gc, textColor);
    const int titleLeft = dims.x + 20;
    const int topicLeft = dims.x + 30;
    const int lineHeight = 15;
    const int gap = 10;

    int inputY = dims.y + 20;

    XSetForeground(display, gc, filterMode ? textColor : borderColor);
    XDrawRectangle(display, window, gc, dims.x + 20, inputY, dims.width - 40, 20);

    std::string filterDisplay = "/" + filterText;
    XSetForeground(display, gc, textColor);
    XDrawString(display, window, gc, dims.x + 25, inputY + 14, filterDisplay.c_str(), filterDisplay.length());

    int y = dims.y + 20 + 25 + gap;
    const int contentTop = y;
    const int contentBottom = dims.y + dims.height;

    y = y + scrollOffset;

    drawAllHelpTopics(nullptr, titleLeft, topicLeft, lineHeight, gap, y, contentTop, contentBottom);
}

void drawConsole(
    Display* display, Window window, GC gc,
    const ConsoleDrawData& data)
{
    int y = data.startY;

    if (data.filterMode) {
        std::string filterDisplay = "/" + data.filterText;
        XDrawString(display, window, gc, 10, y, filterDisplay.c_str(), filterDisplay.length());
        y += data.lineHeight;
    } else if (data.commandMode) {
        std::string commandDisplay = ":" + data.commandText;
        XDrawString(display, window, gc, 10, y, commandDisplay.c_str(), commandDisplay.length());
        y += data.lineHeight;
    }

    if (data.themeSelectMode) {
        std::string header = "Select theme (" + std::to_string(data.themeItems.size()) + " total):";
        XDrawString(display, window, gc, 10, y, header.c_str(), header.length());
        y += data.lineHeight;

        const int VISIBLE_THEMES = 10;
        size_t startIdx = data.themeScrollOffset;
        size_t endIdx = std::min(startIdx + VISIBLE_THEMES, data.themeItems.size());

        for (size_t i = startIdx; i < endIdx; ++i) {
            std::string themeDisplay = (i == data.selectedTheme ? "> " : "  ") + data.themeItems[i];
            XDrawString(display, window, gc, 10, y, themeDisplay.c_str(), themeDisplay.length());
            y += data.lineHeight;
        }

        if (data.themeItems.size() > VISIBLE_THEMES) {
            std::string scrollInfo = "Showing " + std::to_string(startIdx + 1) + "-" + std::to_string(endIdx) + " of " + std::to_string(data.themeItems.size());
            XDrawString(display, window, gc, 10, y, scrollInfo.c_str(), scrollInfo.length());
        }
        return;
    }

    if (data.configSelectMode) {
        std::string header = "Select config option (" + std::to_string(data.configItems.size()) + " total):";
        XDrawString(display, window, gc, 10, y, header.c_str(), header.length());
        y += data.lineHeight;

        const int VISIBLE_CONFIGS = 10;
        size_t startIdx = data.configScrollOffset;
        size_t endIdx = std::min(startIdx + VISIBLE_CONFIGS, data.configItems.size());

        for (size_t i = startIdx; i < endIdx; ++i) {
            std::string configDisplay = (i == data.selectedConfig ? "> " : "  ") + data.configItems[i];
            XDrawString(display, window, gc, 10, y, configDisplay.c_str(), configDisplay.length());
            y += data.lineHeight;
        }

        if (data.configItems.size() > VISIBLE_CONFIGS) {
            std::string scrollInfo = "Showing " + std::to_string(startIdx + 1) + "-" + std::to_string(endIdx) + " of " + std::to_string(data.configItems.size());
            XDrawString(display, window, gc, 10, y, scrollInfo.c_str(), scrollInfo.length());
        }
        return;
    }

    const int SCROLL_INDICATOR_HEIGHT = 15;

    bool needScrollIndicator = data.totalClipCount > data.clipLines.size();
    if (needScrollIndicator) {
        std::string scrollText = "[" + std::to_string(data.selectedItem + 1) + "/" + std::to_string(data.totalClipCount) + "]";
        XDrawString(display, window, gc, data.windowWidth - 80, 15, scrollText.c_str(), scrollText.length());
        y += SCROLL_INDICATOR_HEIGHT;
    }

    for (size_t i = 0; i < data.clipLines.size(); ++i) {
        bool isSelected = (i + data.clipScrollOffset == data.selectedItem);

        if (isSelected) {
            XSetForeground(display, gc, data.selColor);
            XFillRectangle(display, window, gc, 5, y - 12, data.clipListWidth, 15);
            XSetForeground(display, gc, data.textColor);
        } else {
            XSetForeground(display, gc, data.textColor);
        }

        XDrawString(display, window, gc, 10, y, data.clipLines[i].c_str(), data.clipLines[i].length());
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
        XDrawString(display, window, gc, 10, y, empty.c_str(), empty.length());
    }
}

#endif
