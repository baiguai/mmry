#ifndef UI_H
#define UI_H

#include <string>
#include <vector>
#include <algorithm>
#include <utility>

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
#endif

#endif
