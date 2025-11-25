#ifndef HOTKEY_MANAGER_H
#define HOTKEY_MANAGER_H

#ifdef __linux__
#include <X11/Xlib.h>
#endif

#include <functional>

class HotkeyManager {
public:
    HotkeyManager();
    ~HotkeyManager();

    void registerHotkey(int keycode, int modifiers, std::function<void()> callback);
    void unregisterAllHotkeys();
    void start();

private:
#ifdef __linux__
    Display* display;
    Window root;
#endif

    // This is a simplified representation. A real implementation would need more robust handling.
    struct Hotkey {
        int keycode;
        int modifiers;
        std::function<void()> callback;
    };
    std::vector<Hotkey> hotkeys;
};

#endif // HOTKEY_MANAGER_H