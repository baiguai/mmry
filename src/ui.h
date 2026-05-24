#ifndef UI_H
#define UI_H

#include <string>
#include <vector>
#include <algorithm>
#include <utility>

struct ConsoleDrawData {
    bool filterMode;
    bool commandMode;
    bool themeSelectMode;
    bool configSelectMode;
    std::string filterText;
    std::string commandText;

    std::vector<std::string> themeItems;
    size_t selectedTheme;
    size_t themeScrollOffset;

    std::vector<std::string> configItems;
    size_t selectedConfig;
    size_t configScrollOffset;

    std::vector<std::string> clipLines;
    size_t selectedItem;
    size_t clipScrollOffset;
    size_t totalClipCount;
    int clipListWidth;

    int startY;
    int windowWidth;
    int windowHeight;
    int lineHeight;

    unsigned long bgColor;
    unsigned long textColor;
    unsigned long selColor;
};

struct DialogDimensions {
    int width;
    int height;
    int x;
    int y;
    int contentWidth;
    int contentHeight;
};

#ifdef __linux__
#include <X11/Xlib.h>
void drawPinnedDialog(
    Display* display, Window window, GC gc, XFontStruct* font,
    const std::vector<std::pair<long long, std::string>>& displayItems,
    const DialogDimensions& dims,
    size_t& selectedItem, size_t scrollOffset, int& maxVisibleItems,
    unsigned long bgColor, unsigned long textColor,
    unsigned long selColor, unsigned long borderColor,
    int lineHeight);

void drawBookmarkDialog(
    Display* display, Window window, GC gc, XFontStruct* font,
    const DialogDimensions& dims,
    const std::string& inputText,
    const std::vector<std::string>& filteredGroups,
    size_t selectedGroup, size_t scrollOffset,
    unsigned long bgColor, unsigned long textColor,
    unsigned long selColor, unsigned long borderColor);

void drawAddToBookmarkDialog(
    Display* display, Window window, GC gc, XFontStruct* font,
    const DialogDimensions& dims,
    const std::vector<std::string>& displayedGroups,
    size_t selectedGroup, size_t scrollOffset,
    bool filterMode, const std::string& filterText,
    unsigned long bgColor, unsigned long textColor,
    unsigned long selColor, unsigned long borderColor);

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
    unsigned long selColor, unsigned long borderColor);

void drawEditDialog(
    Display* display, Window window, GC gc, XFontStruct* font,
    const DialogDimensions& dims,
    const std::string& inputText,
    size_t cursorLine, size_t cursorPos,
    int scrollOffset,
    unsigned long bgColor, unsigned long textColor,
    unsigned long borderColor);

void drawHelpDialog(
    Display* display, Window window, GC gc,
    const DialogDimensions& dims,
    bool filterMode, const std::string& filterText,
    int scrollOffset,
    unsigned long bgColor, unsigned long textColor,
    unsigned long borderColor);

void drawConsole(
    Display* display, Window window, GC gc,
    const ConsoleDrawData& data);
#endif

#ifdef _WIN32
#include <windows.h>
void drawPinnedDialog(
    HDC hdc,
    const std::vector<std::pair<long long, std::string>>& displayItems,
    const DialogDimensions& dims,
    size_t& selectedItem, size_t scrollOffset, int& maxVisibleItems,
    unsigned long bgColor, unsigned long textColor,
    unsigned long selColor, unsigned long borderColor,
    int lineHeight, int winSelRectHeight, int winSelRectOffsetY);

void drawBookmarkDialog(
    HDC hdc,
    const DialogDimensions& dims,
    const std::string& inputText,
    const std::vector<std::string>& filteredGroups,
    size_t selectedGroup, size_t scrollOffset,
    unsigned long bgColor, unsigned long textColor,
    unsigned long selColor, unsigned long borderColor,
    int winSelRectHeight, int winSelRectOffsetY);

void drawAddToBookmarkDialog(
    HDC hdc,
    const DialogDimensions& dims,
    const std::vector<std::string>& displayedGroups,
    size_t selectedGroup, size_t scrollOffset,
    bool filterMode, const std::string& filterText,
    unsigned long bgColor, unsigned long textColor,
    unsigned long selColor, unsigned long borderColor,
    int winSelRectHeight, int winSelRectOffsetY);

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
    int winSelRectHeight, int winSelRectOffsetY);

void drawEditDialog(
    HDC hdc,
    const DialogDimensions& dims,
    const std::string& inputText,
    size_t cursorLine, size_t cursorPos,
    int scrollOffset,
    unsigned long bgColor, unsigned long textColor,
    unsigned long borderColor);

void drawHelpDialog(
    HDC hdc,
    const DialogDimensions& dims,
    bool filterMode, const std::string& filterText,
    int scrollOffset,
    unsigned long bgColor, unsigned long textColor,
    unsigned long borderColor);

void drawConsole(
    HDC hdc,
    const ConsoleDrawData& data,
    int winSelRectHeight, int winSelRectOffsetY);

void drawConsole();

#endif

#endif
