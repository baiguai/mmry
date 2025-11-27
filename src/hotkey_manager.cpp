#include "hotkey_manager.h"
#include <iostream>

HotkeyManager::HotkeyManager() {
#ifdef __linux__
    display = XOpenDisplay(nullptr);
    if (!display) {
        std::cerr << "Cannot open display for hotkeys" << std::endl;
        return;
    }
    root = DefaultRootWindow(display);
#endif
}

HotkeyManager::~HotkeyManager() {
#ifdef __linux__
    if (display) {
        unregisterAllHotkeys();
        XCloseDisplay(display);
    }
#endif
}

void HotkeyManager::registerHotkey(int keycode, int modifiers, std::function<void()> callback) {
#ifdef __linux__
    if (!display) return;
    XGrabKey(display, keycode, modifiers, root, True, GrabModeAsync, GrabModeAsync);
    hotkeys.push_back({keycode, modifiers, callback});
#endif
}

void HotkeyManager::unregisterAllHotkeys() {
#ifdef __linux__
    if (!display) return;
    for (const auto& hotkey : hotkeys) {
        XUngrabKey(display, hotkey.keycode, hotkey.modifiers, root);
    }
    hotkeys.clear();
#endif
}

void HotkeyManager::start() {
    // This part is tricky. A real implementation would need a dedicated thread
    // to listen for hotkey events or integrate into an existing event loop.
    // For now, this function is a placeholder to show where the logic would go.
    // The main event loop in ClipboardManager will handle these events.
}
