#include "ui.h"

#ifdef __linux__

void drawPinnedDialog(
    Display* display, Window window, GC gc, XFontStruct* font,
    const std::vector<std::pair<long long, std::string>>& displayItems,
    const DialogDimensions& dims,
    size_t& selectedItem, size_t scrollOffset, int& maxVisibleItems,
    unsigned long bgColor, unsigned long textColor,
    unsigned long selColor, unsigned long borderColor,
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

#endif
