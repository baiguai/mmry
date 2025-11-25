#include "ui_manager.h"
#include <iostream>
#include <fstream>
#include "json.hpp" // Assuming you have a JSON library like nlohmann/json

using json = nlohmann::json;

UIManager::UIManager()
    : display(nullptr), window(0), root(0), screen(0), gc(nullptr), font(nullptr),
      visible(false), windowWidth(800), windowHeight(600),
      backgroundColor(0), textColor(0), selectionColor(0), borderColor(0) {
}

UIManager::~UIManager() {
#ifdef __linux__
    if (font) XFreeFont(display, font);
    if (gc) XFreeGC(display, gc);
    if (display) XCloseDisplay(display);
#endif
}

void UIManager::createWindow() {
#ifdef __linux__
    // Simplified window creation
    window = XCreateSimpleWindow(display, root, 100, 100, windowWidth, windowHeight, 1,
                                 borderColor, backgroundColor);
    XSelectInput(display, window, ExposureMask | KeyPressMask | StructureNotifyMask);
    XStoreName(display, window, "MMRY Clipboard Manager");

    // Create Graphics Context
    gc = XCreateGC(display, window, 0, nullptr);

    // Load font
    font = XLoadQueryFont(display, "fixed");
    if (font) {
        XSetFont(display, gc, font->fid);
    }
#endif
}

void UIManager::loadTheme(const std::string& theme_name) {
    std::string theme_path = "themes/" + theme_name + "_theme.json";
    std::ifstream file(theme_path);
    if (!file.is_open()) {
        // Fallback to default colors
        backgroundColor = 0x000000; // Black
        textColor = 0xFFFFFF; // White
        selectionColor = 0x444444; // Grey
        borderColor = 0xFFFFFF; // White
        return;
    }

    json theme;
    file >> theme;

    auto hex_to_color = [](const std::string& hex) {
        return std::stoul(hex.substr(1), nullptr, 16);
    };

    backgroundColor = hex_to_color(theme["background"]);
    textColor = hex_to_color(theme["text"]);
    selectionColor = hex_to_color(theme["selection"]);
    borderColor = hex_to_color(theme["border"]);
}


void UIManager::showWindow() {
#ifdef __linux__
    if (!visible) {
        XMapRaised(display, window);
        XFlush(display);
        visible = true;
    }
#endif
}

void UIManager::hideWindow() {
#ifdef __linux__
    if (visible) {
        XUnmapWindow(display, window);
        XFlush(display);
        visible = false;
    }
#endif
}

void UIManager::drawConsole(const std::vector<ClipboardItem>& items, size_t selectedItem) {
#ifdef __linux__
    if (!visible) return;

    XClearWindow(display, window);

    int y = 20;
    for (size_t i = 0; i < items.size(); ++i) {
        if (i == selectedItem) {
            XSetForeground(display, gc, selectionColor);
            XFillRectangle(display, window, gc, 5, y - 15, windowWidth - 10, 20);
        }

        XSetForeground(display, gc, textColor);
        XDrawString(display, window, gc, 10, y, items[i].content.c_str(), items[i].content.length());
        y += 20;
    }

    XFlush(display);
#endif
}

void UIManager::handleKeyPress(XEvent* event) {
    // This will be expanded to handle UI-specific key presses
}

void UIManager::run() {
#ifdef __linux__
    display = XOpenDisplay(nullptr);
    if (!display) {
        std::cerr << "Cannot open display" << std::endl;
        return;
    }
    screen = DefaultScreen(display);
    root = RootWindow(display, screen);

    // Theme will be loaded by ClipboardManager after config is loaded
    createWindow();

    // The main event loop will be handled by the ClipboardManager class
#endif
}
