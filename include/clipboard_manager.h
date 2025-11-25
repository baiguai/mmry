#ifndef CLIPBOARD_MANAGER_H
#define CLIPBOARD_MANAGER_H

#include "config_manager.h"
#include "clipboard_history.h"
#include "ui_manager.h"
#include "hotkey_manager.h"

class ClipboardManager {
public:
    ClipboardManager();
    void run();

private:
    ConfigManager configManager;
    ClipboardHistory clipboardHistory;
    UIManager uiManager;
    HotkeyManager hotkeyManager;
    bool running;

    void onHotkeyShow();
    void onClipboardChange();
};

#endif // CLIPBOARD_MANAGER_H