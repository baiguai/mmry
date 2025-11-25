#include "clipboard_manager.h"
#include <iostream>
#include <X11/keysym.h>

ClipboardManager::ClipboardManager() : running(false) {
}

void ClipboardManager::run() {
    running = true;

    configManager.setupConfigDir();
    configManager.loadConfig();

    clipboardHistory.loadFromFile(
        configManager.getDataFile(),
        configManager.isEncrypted(),
        configManager.getEncryptionKey()
    );

    uiManager.run();
    hotkeyManager.registerHotkey(XKeysymToKeycode(XOpenDisplay(nullptr), XK_C), ControlMask | Mod1Mask, [this]() {
        onHotkeyShow();
    });
    hotkeyManager.start();

    // Main event loop
    // This is a simplified event loop. A real application would need a more robust one.
    while (running) {
        // This is where the event loop from complete_main.cpp would be integrated.
        // It would check for X11 events, hotkey events, and clipboard events.
        // For simplicity, this is left as a placeholder.
    }

    clipboardHistory.saveToFile(
        configManager.getDataFile(),
        configManager.isEncrypted(),
        configManager.getEncryptionKey()
    );
}

void ClipboardManager::onHotkeyShow() {
    uiManager.showWindow();
}

void ClipboardManager::onClipboardChange() {
    // This would be called when the clipboard content changes.
    // It would get the new content and add it to the clipboard history.
}
