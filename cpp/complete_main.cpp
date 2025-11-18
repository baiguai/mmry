#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <iostream>
#include <unistd.h>
#include <thread>
#include <atomic>
#include <string>
#include <vector>
#include <chrono>

class ClipboardManager {
private:
    Display* display;
    Window window;
    Window root;
    std::atomic<bool> running;
    std::atomic<bool> visible;
    int screen;
    GC gc;
    XFontStruct* font;
    
    // Window properties
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;
    const int WINDOW_X = 100;
    const int WINDOW_Y = 100;
    
public:
    ClipboardManager() : running(false), visible(false) {
        display = XOpenDisplay(nullptr);
        if (!display) {
            std::cerr << "Cannot open display" << std::endl;
            exit(1);
        }
        
        screen = DefaultScreen(display);
        root = RootWindow(display, screen);
        
        createWindow();
        setupHotkeys();
    }
    
    ~ClipboardManager() {
        if (font) XFreeFont(display, font);
        if (gc) XFreeGC(display, gc);
        if (window) XDestroyWindow(display, window);
        if (display) XCloseDisplay(display);
    }
    
    void createWindow() {
        // Create window
        unsigned long black = BlackPixel(display, screen);
        unsigned long white = WhitePixel(display, screen);
        
        window = XCreateSimpleWindow(display, root, 
                                   WINDOW_X, WINDOW_Y, 
                                   WINDOW_WIDTH, WINDOW_HEIGHT,
                                   2, white, black);
        
        // Set window properties
        XStoreName(display, window, "MMRY - Clipboard Manager");
        XSelectInput(display, window, ExposureMask | KeyPressMask);
        
        // Create graphics context
        gc = XCreateGC(display, window, 0, nullptr);
        XSetForeground(display, gc, white);
        
        // Load font (try to find a monospace font)
        font = XLoadQueryFont(display, "-*-fixed-medium-r-*-*-13-*-*-*-*-*-*-*");
        if (!font) {
            font = XLoadQueryFont(display, "fixed");
        }
        if (font) {
            XSetFont(display, gc, font->fid);
        }
        
        // Set window to be always on top and skip taskbar
        XWMHints hints;
        hints.flags = InputHint | StateHint;
        hints.input = True;
        hints.initial_state = NormalState;
        XSetWMHints(display, window, &hints);
        
        // Set window type to dialog
        Atom atom_type = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
        Atom atom_dialog = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DIALOG", False);
        XChangeProperty(display, window, atom_type, XA_ATOM, 32, PropModeReplace, 
                       (unsigned char*)&atom_dialog, 1);
    }
    
    void setupHotkeys() {
        // Grab Ctrl+Alt+C globally
        XGrabKey(display, XKeysymToKeycode(display, XK_c), 
                 ControlMask | Mod1Mask, root, True, GrabModeAsync, GrabModeAsync);
        
        // Grab Escape globally
        XGrabKey(display, XKeysymToKeycode(display, XK_Escape), 
                 0, root, True, GrabModeAsync, GrabModeAsync);
        
        XSelectInput(display, root, KeyPressMask);
    }
    
    void showWindow() {
        if (!visible) {
            XMapWindow(display, window);
            visible = true;
            std::cout << "Window shown" << std::endl;
        }
    }
    
    void hideWindow() {
        if (visible) {
            XUnmapWindow(display, window);
            visible = false;
            std::cout << "Window hidden" << std::endl;
        }
    }
    
    void drawConsole() {
        if (!visible) return;
        
        // Clear window with black background
        XSetWindowBackground(display, window, BlackPixel(display, screen));
        XClearWindow(display, window);
        
        // Draw console text
        std::vector<std::string> lines = {
            "MMRY Clipboard Manager",
            "",
            "Press Ctrl+Alt+C to toggle window",
            "Press Escape to hide",
            "Press Q to quit",
            "",
            "Clipboard items will appear here...",
            "",
            "> "
        };
        
        int y = 20;
        for (const auto& line : lines) {
            XDrawString(display, window, gc, 10, y, line.c_str(), line.length());
            y += 15;
        }
    }
    
    void run() {
        running = true;
        
        std::cout << "MMRY Clipboard Manager started" << std::endl;
        std::cout << "Press Ctrl+Alt+C to show window, Escape to hide" << std::endl;
        std::cout << "Press Ctrl+C in terminal to exit" << std::endl;
        
        XEvent event;
        while (running) {
            XNextEvent(display, &event);
            
            switch (event.type) {
                case Expose:
                    if (event.xexpose.count == 0) {
                        drawConsole();
                    }
                    break;
                    
                case KeyPress: {
                    XKeyEvent* keyEvent = reinterpret_cast<XKeyEvent*>(&event);
                    KeySym keysym = XLookupKeysym(keyEvent, 0);
                    
                    // Check if it's a global hotkey (root window)
                    if (keyEvent->window == root) {
                        if (keysym == XK_c && (keyEvent->state & ControlMask) && (keyEvent->state & Mod1Mask)) {
                            showWindow();
                        } else if (keysym == XK_Escape) {
                            hideWindow();
                        }
                    } 
                    // Check if it's a local window hotkey
                    else if (keyEvent->window == window) {
                        if (keysym == XK_Escape) {
                            hideWindow();
                        } else if (keysym == XK_q || keysym == XK_Q) {
                            stop();
                        }
                    }
                    break;
                }
            }
        }
    }
    
    void stop() {
        running = false;
    }
};

int main() {
    ClipboardManager manager;
    
    // Run in main thread
    manager.run();
    
    return 0;
}