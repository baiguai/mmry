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
#include <fstream>
#include <sstream>
#include <iomanip>

struct ClipboardItem {
    std::string content;
    std::chrono::system_clock::time_point timestamp;
    size_t id;
    
    ClipboardItem(const std::string& content, size_t id) 
        : content(content), id(id), timestamp(std::chrono::system_clock::now()) {}
};

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
    
    // Clipboard data
    std::vector<ClipboardItem> items;
    size_t nextId;
    std::string lastClipboardContent;
    bool verboseMode;
    Atom clipboardAtom;
    Atom utf8Atom;
    Atom textAtom;
    
public:
    ClipboardManager() : running(false), visible(false), nextId(1), verboseMode(false) {
        display = XOpenDisplay(nullptr);
        if (!display) {
            std::cerr << "Cannot open display" << std::endl;
            exit(1);
        }
        
        screen = DefaultScreen(display);
        root = RootWindow(display, screen);
        
        // Initialize clipboard atoms
        clipboardAtom = XInternAtom(display, "CLIPBOARD", False);
        utf8Atom = XInternAtom(display, "UTF8_STRING", False);
        textAtom = XInternAtom(display, "TEXT", False);
        
        loadFromFile();
        createWindow();
        setupHotkeys();
        startClipboardMonitoring();
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
        XStoreName(display, window, "MMRY");
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
        
        // Draw mode indicator
        std::string modeLine = "Mode: " + std::string(verboseMode ? "Verbose" : "Normal") + " (V to toggle)";
        XDrawString(display, window, gc, 10, 20, modeLine.c_str(), modeLine.length());
        
        // Draw clipboard items
        int y = 40;
        int maxItems = (WINDOW_HEIGHT - 60) / 15; // Approximate lines that fit
        
        for (size_t i = 0; i < items.size() && i < maxItems; ++i) {
            const auto& item = items[i];
            
            std::string line;
            
            if (verboseMode) {
                // Verbose mode: [id] timestamp | lines | content
                auto time_t = std::chrono::system_clock::to_time_t(item.timestamp);
                auto tm = *std::localtime(&time_t);
                
                std::ostringstream timeStream;
                timeStream << std::put_time(&tm, "%H:%M:%S");
                
                // Count lines in content
                size_t lineCount = 1;
                for (char c : item.content) {
                    if (c == '\n') lineCount++;
                }
                
                line = "[" + std::to_string(item.id) + "] " + timeStream.str() + " | " + 
                       std::to_string(lineCount) + " lines | ";
                
                // Truncate content if too long
                std::string content = item.content;
                if (content.length() > 50) {
                    content = content.substr(0, 47) + "...";
                }
                
                // Replace newlines with spaces for display
                for (char& c : content) {
                    if (c == '\n' || c == '\r') c = ' ';
                }
                
                line += content;
            } else {
                // Normal mode: [id] content (line count if > 1)
                line = "[" + std::to_string(item.id) + "] ";
                
                // Count lines in content
                size_t lineCount = 1;
                for (char c : item.content) {
                    if (c == '\n') lineCount++;
                }
                
                // Truncate content if too long
                std::string content = item.content;
                if (content.length() > 80) {
                    content = content.substr(0, 77) + "...";
                }
                
                // Replace newlines with spaces for display
                for (char& c : content) {
                    if (c == '\n' || c == '\r') c = ' ';
                }
                
                line += content;
                
                // Add line count if more than 1 line
                if (lineCount > 1) {
                    line += " (" + std::to_string(lineCount) + " lines)";
                }
            }
            
            XDrawString(display, window, gc, 10, y, line.c_str(), line.length());
            y += 15;
        }
        
        if (items.empty()) {
            std::string empty = "No clipboard items yet...";
            XDrawString(display, window, gc, 10, y, empty.c_str(), empty.length());
        }
    }
    
    void startClipboardMonitoring() {
        // Start a separate thread for clipboard monitoring
        std::thread([this]() {
            while (running) {
                checkClipboard();
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
        }).detach();
    }
    
    void checkClipboard() {
        // Get clipboard selection
        Window owner = XGetSelectionOwner(display, clipboardAtom);
        if (owner == None || owner == window) {
            return; // No clipboard owner or we own it
        }
        
        // Request clipboard selection
        XConvertSelection(display, clipboardAtom, utf8Atom, None, window, CurrentTime);
        XFlush(display);
        
        // Wait for selection notification
        XEvent event;
        while (XPending(display)) {
            XNextEvent(display, &event);
            if (event.type == SelectionNotify) {
                XSelectionEvent* selEvent = (XSelectionEvent*)&event;
                if (selEvent->property != None) {
                    // Get the actual clipboard data
                    Atom actualType;
                    int actualFormat;
                    unsigned long nitems, bytesAfter;
                    unsigned char* prop = nullptr;
                    
                    if (XGetWindowProperty(display, window, selEvent->property, 0, 1024*1024, False,
                                          AnyPropertyType, &actualType, &actualFormat, &nitems, &bytesAfter, &prop) == Success) {
                        if (prop && nitems > 0) {
                            std::string content(reinterpret_cast<char*>(prop));
                            XFree(prop);
                            
                            if (!content.empty() && content != lastClipboardContent) {
                                lastClipboardContent = content;
                                
                                // Check for duplicates
                                bool isDuplicate = false;
                                for (const auto& item : items) {
                                    if (item.content == content) {
                                        isDuplicate = true;
                                        break;
                                    }
                                }
                                
                                if (!isDuplicate) {
                                    items.emplace(items.begin(), content, nextId++);
                                    if (items.size() > 100) { // Keep only last 100 items
                                        items.pop_back();
                                    }
                                    saveToFile();
                                    
                                    std::cout << "New clipboard item added: [" << (nextId-1) << "]" << std::endl;
                                    
                                    // Refresh display if window is visible
                                    if (visible) {
                                        drawConsole();
                                    }
                                }
                            }
                        }
                    }
                    
                    // Clean up the property
                    XDeleteProperty(display, window, selEvent->property);
                }
                break;
            }
        }
    }
    
    void saveToFile() {
        std::ofstream file("/tmp/mmry_data.txt");
        if (file.is_open()) {
            for (const auto& item : items) {
                file << item.id << "|" << item.content << "\n";
            }
            file.close();
        }
    }
    
    void loadFromFile() {
        std::ifstream file("/tmp/mmry_data.txt");
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                size_t pos = line.find('|');
                if (pos != std::string::npos) {
                    size_t id = std::stoull(line.substr(0, pos));
                    std::string content = line.substr(pos + 1);
                    items.emplace_back(content, id);
                    if (id >= nextId) {
                        nextId = id + 1;
                    }
                }
            }
            file.close();
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
                        } else if (keysym == XK_v || keysym == XK_V) {
                            verboseMode = !verboseMode;
                            drawConsole();
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