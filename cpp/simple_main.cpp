#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <iostream>
#include <unistd.h>
#include <thread>
#include <atomic>

class HotkeyManager {
private:
    Display* display;
    Window root;
    std::atomic<bool> running;
    
public:
    HotkeyManager() : running(false) {
        display = XOpenDisplay(nullptr);
        if (!display) {
            std::cerr << "Cannot open display" << std::endl;
            exit(1);
        }
        root = DefaultRootWindow(display);
    }
    
    ~HotkeyManager() {
        if (display) {
            XCloseDisplay(display);
        }
    }
    
    void grabHotkeys() {
        // Grab Ctrl+Alt+C
        XGrabKey(display, XKeysymToKeycode(display, XK_c), 
                 ControlMask | Mod1Mask, root, True, GrabModeAsync, GrabModeAsync);
        
        // Grab Escape
        XGrabKey(display, XKeysymToKeycode(display, XK_Escape), 
                 0, root, True, GrabModeAsync, GrabModeAsync);
        
        XSelectInput(display, root, KeyPressMask);
    }
    
    void run() {
        running = true;
        grabHotkeys();
        
        std::cout << "Hotkey manager started. Press Ctrl+Alt+C to show window, Escape to hide." << std::endl;
        
        XEvent event;
        while (running) {
            XNextEvent(display, &event);
            
            if (event.type == KeyPress) {
                XKeyEvent* keyEvent = reinterpret_cast<XKeyEvent*>(&event);
                KeySym keysym = XLookupKeysym(keyEvent, 0);
                
                if (keysym == XK_c && (keyEvent->state & ControlMask) && (keyEvent->state & Mod1Mask)) {
                    std::cout << "Ctrl+Alt+C pressed - Show window" << std::endl;
                    // TODO: Show window
                } else if (keysym == XK_Escape) {
                    std::cout << "Escape pressed - Hide window" << std::endl;
                    // TODO: Hide window
                }
            }
        }
    }
    
    void stop() {
        running = false;
    }
};

int main() {
    HotkeyManager hotkeyManager;
    
    // Run hotkey manager in a separate thread
    std::thread hotkeyThread([&hotkeyManager]() {
        hotkeyManager.run();
    });
    
    std::cout << "MMRY Clipboard Manager started" << std::endl;
    std::cout << "Press Ctrl+C to exit" << std::endl;
    
    // Main thread waits
    hotkeyThread.join();
    
    return 0;
}