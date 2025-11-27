#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <string>
#include <vector>
#include "clipboard_history.h"

#ifdef __linux__
#include <X11/Xlib.h>
#endif

class UIManager {
public:
    UIManager();
    ~UIManager();

    void showWindow();
    void hideWindow();
    void drawConsole(const std::vector<ClipboardItem>& items, size_t selectedItem);
    void handleKeyPress(XEvent* event);
    void run();

private:
#ifdef __linux__
    Display* display;
    Window window;
    Window root;
    int screen;
    GC gc;
    XFontStruct* font;
#endif

    bool visible;
    int windowWidth;
    int windowHeight;
    unsigned long backgroundColor;
    unsigned long textColor;
    unsigned long selectionColor;
    unsigned long borderColor;

    void createWindow();
    void loadTheme(const std::string& theme);
};

#endif // UI_MANAGER_H