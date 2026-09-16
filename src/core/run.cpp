#include "../clipboard_manager.h"

void ClipboardManager::run()
{
    running = true;
    visible = false;
    
    // Initialize X11
#ifdef __linux__
    display = XOpenDisplay(nullptr);
    if (!display)
    {
        std::cerr << "Cannot open display" << std::endl;
        return;
    }
    screen = DefaultScreen(display);
    root = RootWindow(display, screen);

    if (!XFixesQueryExtension(display, &xfixes_event_base, &xfixes_error_base))
    {
        std::cerr << "XFixes extension not available" << std::endl;
        // Fallback to polling or exit? For now, we exit.
        XCloseDisplay(display);
        return;
    }

    clipboardAtom = XInternAtom(display, "CLIPBOARD", False);
    utf8Atom = XInternAtom(display, "UTF8_STRING", False);
    textAtom = XInternAtom(display, "TEXT", False);
    
    // Create window
    createWindow();
    
    // Setup hotkey
    setupHotkeys();
#endif
    
    // Initialize configuration and theme
    config.setupConfigDir();
    config.loadConfig();
    config.loadTheme();
#ifdef __linux__
    // Window was created before config was loaded, so apply the saved position now
    restoreWindowPosition();
#endif
#ifdef __linux__
    if (gc)
    {
        XSetForeground(display, gc, config.textColor);
    }
#endif
    loadFromFile();
    loadBookmarkGroups();


    #ifdef _WIN32
        std::cout << "Running on Windows.\n";
        // Add Windows-specific auto-start code here
    #elif __APPLE__
        std::cout << "Running on macOS.\n";
        // Add macOS-specific auto-start code here
    #elif __linux__
        std::string dir = std::string(getenv("HOME")) + "/.config/autostart";
        std::string filePath = dir + "/mmry.desktop";

        if (config.autoStart)
        {
            std::string appName = "mmry";
            std::string appLabel = "Mmry";

            // ensure directory exists
            std::string mkdirCmd = "mkdir -p " + dir;
            int mkdirResult = system(mkdirCmd.c_str());
            (void)mkdirResult; // Suppress unused result warning


            char result[PATH_MAX];
            ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
            if (count != -1)
            {
                result[count] = '\0'; // Null-terminate the string
                std::cout << "Full path: " << result << "\n";
            }
            else
            {
                std::cerr << "Error getting path" << "\n";
            }

            std::ofstream file(filePath);
            file <<
R"([Desktop Entry]
Type=Application
Exec=)" << result << R"(
Hidden=false
NoDisplay=false
X-GNOME-Autostart-enabled=true
Name=)" << appLabel << R"(
Comment=Autostart for )" << appLabel << R"(
)";
            file.close();
        }
        else
        {
            if (remove(filePath.c_str()) != 0)
            {
                std::cout << "Autostart already disabled\n";
            }
            else
            {
                std::cout << "Autostart disabled\n";
            }
        }
    #else
        std::cout << "Unknown operating system.\n";
    #endif

    
    std::cout << "MMRY Clipboard Manager started\n";
    std::cout << "Config directory: " << config.configDir << "\n";
    std::cout << "Press Ctrl+Alt+C to show window, Escape to hide\n";
    std::cout << "Press Shift+Q in window to quit application\n";
    std::cout << "Press Ctrl+C in terminal to exit\n";
    
#ifdef __linux__
    // --- Reliable X11 global hotkey setup for Ctrl+Alt+C ----------
    auto grab_global_hotkey = [&](Display* dpy, Window rootWin, KeySym keysym)
    {
        if (!dpy)
        {
            std::cout << "!dpy - returning\n";
            return;
        }
        // Ensure root receives KeyPress events
        XSelectInput(dpy, rootWin, KeyPressMask);
        XFlush(dpy);

        KeyCode kc = XKeysymToKeycode(dpy, keysym);

        const unsigned int baseMods[] =
        {
            ControlMask | Mod1Mask,                 // Ctrl + Alt
            ControlMask | Mod1Mask | Mod2Mask,     // + NumLock
            ControlMask | Mod1Mask | LockMask,     // + CapsLock
            ControlMask | Mod1Mask | Mod5Mask,     // + Mod5 (often ISO Level3)
            ControlMask | Mod1Mask | Mod2Mask | LockMask,
            ControlMask | Mod1Mask | Mod2Mask | Mod5Mask,
            ControlMask | Mod1Mask | LockMask | Mod5Mask,
            ControlMask | Mod1Mask | Mod2Mask | LockMask | Mod5Mask
        };

        for (unsigned int m : baseMods)
        {
            // Passive grabs on the root window
            XGrabKey(dpy, kc, m, rootWin, True, GrabModeAsync, GrabModeAsync);
            // Also grab with NumLock explicitly OR'd (sometimes necessary)
            XGrabKey(dpy, kc, m | Mod2Mask, rootWin, True, GrabModeAsync, GrabModeAsync);
        }
        XFlush(dpy);
    };

    auto ungrab_global_hotkey = [&](Display* dpy, Window rootWin, KeySym keysym)
    {
        if (!dpy) return;
        KeyCode kc = XKeysymToKeycode(dpy, keysym);
        const unsigned int baseMods[] =
        {
            ControlMask | Mod1Mask,
            ControlMask | Mod1Mask | Mod2Mask,
            ControlMask | Mod1Mask | LockMask,
            ControlMask | Mod1Mask | Mod5Mask,
            ControlMask | Mod1Mask | Mod2Mask | LockMask,
            ControlMask | Mod1Mask | Mod2Mask | Mod5Mask,
            ControlMask | Mod1Mask | LockMask | Mod5Mask,
            ControlMask | Mod1Mask | Mod2Mask | LockMask | Mod5Mask
        };
        for (unsigned int m : baseMods)
        {
            XUngrabKey(dpy, kc, m, rootWin);
            XUngrabKey(dpy, kc, m | Mod2Mask, rootWin);
        }
        XFlush(dpy);
    };

    // Must call this once after creating display & root
    grab_global_hotkey(display, root, XK_C);

    // Helpful: determine the grabbed keycode for runtime checks
    KeyCode grabbed_keycode = XKeysymToKeycode(display, XK_C);

    // Listen for clipboard changes
    XFixesSelectSelectionInput(display, root, clipboardAtom, XFixesSetSelectionOwnerNotifyMask);
    
    // --- Event loop: blocking, waits for next event -----------
    while (running)
    {
        XEvent event;
        XNextEvent(display, &event);

        // Handle clipboard change event
        if (event.type == xfixes_event_base + XFixesSelectionNotify)
        {
            XFixesSelectionNotifyEvent *selection_event = (XFixesSelectionNotifyEvent*)&event;
            if (selection_event->selection == clipboardAtom)
            {
                requestClipboardContent();
            }
            continue;
        }

        // Handle clipboard content arrival
        if (event.type == SelectionNotify)
        {
            handleSelectionNotify(&event);
            continue;
        }
        
        // Handle global hotkey
        if (event.type == KeyPress && event.xkey.keycode == grabbed_keycode)
        {
            if ((event.xkey.state & ControlMask) && (event.xkey.state & Mod1Mask))
            {
                // Hotkey triggered
                std::cout << "Hotkey triggered: Ctrl+Alt+C\n";
                showWindow();
                continue;
            }
        }

        // Events for the application window
        if (event.xany.window == window)
        {
            switch (event.type)
            {
                case Expose:
                    drawConsole();
                    break;
                case KeyPress:
                    handleKeyPress(&event);
                    break;
                case ConfigureNotify:
                    // Window resize event
                    updateWindowDimensions(event.xconfigure.width, event.xconfigure.height);
                    drawConsole();
                    break;
                default:
                    break;
            }
        }
    }

    // Save window position on exit
    updateWindowPosition();
    config.saveConfig();

    // Unregister our grabs on exit
    ungrab_global_hotkey(display, root, XK_C);
#endif

#ifdef _WIN32
    std::cout << "Windows: registering global hotkey Ctrl+Alt+C...\n" << std::endl;

    // Register Ctrl+Alt+C (ID: 1)
    if (!RegisterHotKey(NULL, 1, MOD_CONTROL | MOD_ALT, 'C'))
    {
        std::cerr << "Failed to register global hotkey." << std::endl;
    }

    // --- Win32 window class (blank window for now) ---
    WNDCLASS wc = {};
    wc.lpfnWndProc   = MMRYWndProc;
    wc.hInstance     = GetModuleHandle(NULL);
    wc.lpszClassName = "MMRY_Window_Class";
    wc.hbrBackground = NULL; // We'll handle background painting ourselves

    RegisterClass(&wc);

    // 'hwnd' is now a class member, initialized to nullptr.
    // The check '!hwnd' on first hotkey press will create it.

    // --- Windows Message Loop ---
    MSG msg;
    while (running)
    {
        BOOL result = GetMessage(&msg, NULL, 0, 0);
        if (result <= 0) break;
        
        if (msg.message == WM_HOTKEY && msg.wParam == 1)
        {
            // Hotkey handling
            if (!hwnd)
            {
                // Create window with proper styles for keyboard input
                hwnd = CreateWindowEx(
                    WS_EX_CLIENTEDGE,
                    "MMRY_Window_Class",
                    "MMRY Clipboard Window",
                    WS_OVERLAPPEDWINDOW,
                    config.windowX, config.windowY,
                    800, 450,
                    NULL, NULL, GetModuleHandle(NULL), 
                    this); // Pass 'this' as lpParam
                
                if (hwnd)
                {
                    AddClipboardFormatListener(hwnd);

                    // Apply saved window position
                    restoreWindowPosition();

                    // Create and select a font
                    font = CreateFontA(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, 
                                       DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, 
                                       CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                                       FIXED_PITCH | FF_MODERN, "Courier New");

                    RECT clientRect;
                    GetClientRect(hwnd, &clientRect);
                    int actualClientWidth = clientRect.right - clientRect.left;
                    int actualClientHeight = clientRect.bottom - clientRect.top;
                    writeLog("run(): Before updateWindowDimensions. actualClientWidth=" + std::to_string(actualClientWidth) + ", actualClientHeight=" + std::to_string(actualClientHeight));
                    updateWindowDimensions(actualClientWidth, actualClientHeight);
                }
            }
            visible = true;
            ShowWindow(hwnd, SW_SHOW);
            SetForegroundWindow(hwnd);
            SetFocus(hwnd); 
            drawConsole();
        }
        
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Save window position on exit
    updateWindowPosition();
    config.saveConfig();

    UnregisterHotKey(NULL, 1);
#endif

#ifdef __APPLE__
    // macOS event loop
    // This would require NSApplication setup
#endif
}

void ClipboardManager::setRunning(bool state)
{
    running = state;
}

void ClipboardManager::stop()
{
    running = false;
    
    // Join threads to prevent memory leaks


    
#ifdef __linux__
    // Clean up X11 resources
    if (font)
    {
        XFreeFont(display, font);
        font = nullptr;
    }
    if (gc)
    {
        XFreeGC(display, gc);
        gc = nullptr;
    }
    if (display)
    {
        XCloseDisplay(display);
        display = nullptr;
    }
#endif
}

#ifdef _WIN32
    char ClipboardManager::getCharFromMsg(MSG* msg)
    {
        // Get the scan code from lParam
        UINT scanCode = (msg->lParam >> 16) & 0xFF;

        // Get current keyboard state
        BYTE keyboardState[256];
        GetKeyboardState(keyboardState);

        // Convert virtual key to character
        char charBuffer[2]; // Needs space for null terminator
        int result = ToAscii(msg->wParam, scanCode, keyboardState, (LPWORD)charBuffer, 0);

        if (result == 1)
        {
            return charBuffer[0];
        }
        return 0; // Return null character if conversion fails
    }
#endif
