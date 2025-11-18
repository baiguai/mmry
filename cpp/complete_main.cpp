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
#include <cstdlib>
#include <sys/stat.h>

#ifdef __linux__
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#endif

struct ClipboardItem {
    std::string content;
    std::chrono::system_clock::time_point timestamp;
    
    ClipboardItem(const std::string& content) 
        : content(content), timestamp(std::chrono::system_clock::now()) {}
};

class ClipboardManager {
private:
#ifdef __linux__
    Display* display;
    Window window;
    Window root;
    int screen;
    GC gc;
    XFontStruct* font;
    Atom clipboardAtom;
    Atom utf8Atom;
    Atom textAtom;
#endif

#ifdef _WIN32
    HWND hwnd;
    HFONT font;
#endif

    std::atomic<bool> running;
    std::atomic<bool> visible;
    
    // Window properties
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;
    const int WINDOW_X = 100;
    const int WINDOW_Y = 100;
    
    // Clipboard data
    std::vector<ClipboardItem> items;
    std::string lastClipboardContent;
    bool verboseMode;
    std::string configDir;
    std::string dataFile;
    
    // Navigation
    size_t selectedItem = 0;
    
public:
    ClipboardManager() : running(false), visible(false), verboseMode(false) {
        setupConfigDir();
        
#ifdef __linux__
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
#endif

#ifdef _WIN32
        // Windows initialization
#endif

#ifdef __APPLE__
        // macOS initialization
#endif
        
        loadConfig();
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
    
    void setupConfigDir() {
        const char* home = getenv("HOME");
        if (!home) home = getenv("USERPROFILE");
        if (!home) home = ".";
        
        configDir = std::string(home) + "/.config/mmry";
        
        // Create config directory if it doesn't exist
        struct stat st = {0};
        if (stat(configDir.c_str(), &st) == -1) {
            mkdir(configDir.c_str(), 0755);
        }
        
        dataFile = configDir + "/clips.txt";
    }
    
    void createWindow() {
#ifdef __linux__
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
#endif

#ifdef _WIN32
        // Windows window creation
#endif

#ifdef __APPLE__
        // macOS window creation
#endif
    }
    
    void setupHotkeys() {
#ifdef __linux__
        // Try to grab global hotkeys with error handling
        int grabResult;
        
        // Grab Ctrl+Alt+C globally
        grabResult = XGrabKey(display, XKeysymToKeycode(display, XK_c), 
                             ControlMask | Mod1Mask, root, True, GrabModeAsync, GrabModeAsync);
        if (grabResult == BadAccess) {
            std::cerr << "Warning: Could not grab Ctrl+Alt+C hotkey (already in use)" << std::endl;
        }
        
        // Grab Escape globally
        grabResult = XGrabKey(display, XKeysymToKeycode(display, XK_Escape), 
                             0, root, True, GrabModeAsync, GrabModeAsync);
        if (grabResult == BadAccess) {
            std::cerr << "Warning: Could not grab Escape hotkey (already in use)" << std::endl;
        }
        
        XSelectInput(display, root, KeyPressMask);
#endif

#ifdef _WIN32
        // Windows hotkey setup
#endif

#ifdef __APPLE__
        // macOS hotkey setup
#endif
    }
    
    void showWindow() {
        if (!visible) {
#ifdef __linux__
            XMapWindow(display, window);
#endif
#ifdef _WIN32
            ShowWindow(hwnd, SW_SHOW);
#endif
            visible = true;
            std::cout << "Window shown" << std::endl;
        }
    }
    
    void hideWindow() {
        if (visible) {
#ifdef __linux__
            XUnmapWindow(display, window);
#endif
#ifdef _WIN32
            ShowWindow(hwnd, SW_HIDE);
#endif
            visible = false;
            std::cout << "Window hidden" << std::endl;
        }
    }
    
    void drawConsole() {
        if (!visible) return;
        
#ifdef __linux__
        // Clear window with black background
        XSetWindowBackground(display, window, BlackPixel(display, screen));
        XClearWindow(display, window);
        
        // Draw mode indicator
        std::string modeLine = "Mode: " + std::string(verboseMode ? "Verbose" : "Normal");
        XDrawString(display, window, gc, 10, 20, modeLine.c_str(), modeLine.length());
        
        // Draw clipboard items
        int y = 40;
        int maxItems = (WINDOW_HEIGHT - 60) / 15; // Approximate lines that fit
        
        for (size_t i = 0; i < items.size() && i < maxItems; ++i) {
            const auto& item = items[i];
            
            std::string line;
            
            // Add selection indicator
            if (i == selectedItem) {
                line = "> ";
            } else {
                line = "  ";
            }
            
            if (verboseMode) {
                // Verbose mode: timestamp | lines | content
                auto time_t = std::chrono::system_clock::to_time_t(item.timestamp);
                auto tm = *std::localtime(&time_t);
                
                std::ostringstream timeStream;
                timeStream << std::put_time(&tm, "%H:%M:%S");
                
                // Count lines in content
                size_t lineCount = 1;
                for (char c : item.content) {
                    if (c == '\n') lineCount++;
                }
                
                line += timeStream.str() + " | " + std::to_string(lineCount) + " lines | ";
                
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
                // Normal mode: content (line count if > 1)
                
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
            
            // Highlight selected item with dim gray
            if (i == selectedItem) {
                // Create dim gray color (RGB: 64, 64, 64)
                unsigned long grayPixel = 64 * 256 * 256 + 64 * 256 + 64;
                XSetForeground(display, gc, grayPixel);
                XFillRectangle(display, window, gc, 5, y - 12, WINDOW_WIDTH - 10, 15);
                XSetForeground(display, gc, WhitePixel(display, screen));
            } else {
                XSetForeground(display, gc, WhitePixel(display, screen));
            }
            
            XDrawString(display, window, gc, 10, y, line.c_str(), line.length());
            
            y += 15;
        }
        
        if (items.empty()) {
            std::string empty = "No clipboard items yet...";
            XDrawString(display, window, gc, 10, y, empty.c_str(), empty.length());
        }
#endif

#ifdef _WIN32
        // Windows drawing
#endif

#ifdef __APPLE__
        // macOS drawing
#endif
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
        std::string content;
        
#ifdef __linux__
        // Linux: use xclip command
        FILE* pipe = popen("xclip -selection clipboard -o 2>/dev/null", "r");
        if (!pipe) return;
        
        char buffer[4096];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            content += buffer;
        }
        pclose(pipe);
#endif

#ifdef _WIN32
        // Windows: use Windows API
        if (OpenClipboard(nullptr)) {
            HANDLE hData = GetClipboardData(CF_TEXT);
            if (hData) {
                char* text = static_cast<char*>(GlobalLock(hData));
                if (text) {
                    content = text;
                    GlobalUnlock(hData);
                }
            }
            CloseClipboard();
        }
#endif

#ifdef __APPLE__
        // macOS: use pbpaste command
        FILE* pipe = popen("pbpaste 2>/dev/null", "r");
        if (!pipe) return;
        
        char buffer[4096];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            content += buffer;
        }
        pclose(pipe);
#endif
        
        // Trim trailing newlines
        while (!content.empty() && (content.back() == '\n' || content.back() == '\r')) {
            content.pop_back();
        }
        
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
            items.emplace(items.begin(), content);
            if (items.size() > 100) { // Keep only last 100 items
                items.pop_back();
            }
            
            // Reset selection to top when new item is added
            selectedItem = 0;
            
            saveToFile();
            
            std::cout << "New clipboard item added" << std::endl;
            
            // Refresh display if window is visible
            if (visible) {
                drawConsole();
            }
        }
        }
    }
    
    void loadConfig() {
        std::string configFile = configDir + "/config.json";
        std::ifstream file(configFile);
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                // Simple JSON parsing for "verbose": true/false
                if (line.find("\"verbose\"") != std::string::npos) {
                    if (line.find("true") != std::string::npos) {
                        verboseMode = true;
                    } else {
                        verboseMode = false;
                    }
                    break;
                }
            }
            file.close();
        } else {
            // Create default config
            std::ofstream outFile(configFile);
            outFile << "{\n    \"verbose\": false\n}\n";
            outFile.close();
        }
    }
    
    void saveToFile() {
        std::ofstream file(dataFile);
        if (file.is_open()) {
            for (const auto& item : items) {
                // Store timestamp and content
                auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                    item.timestamp.time_since_epoch()).count();
                file << timestamp << "|" << item.content << "\n";
            }
            file.close();
        }
    }
    
    void loadFromFile() {
        std::ifstream file(dataFile);
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                size_t pos = line.find('|');
                if (pos != std::string::npos) {
                    std::string timestampStr = line.substr(0, pos);
                    std::string content = line.substr(pos + 1);
                    
                    ClipboardItem item(content);
                    auto timestamp = std::chrono::seconds(std::stoll(timestampStr));
                    item.timestamp = std::chrono::system_clock::time_point(timestamp);
                    
                    items.push_back(item);
                }
            }
            file.close();
        }
    }
    
    void run() {
        running = true;
        
        std::cout << "MMRY Clipboard Manager started" << std::endl;
        std::cout << "Config directory: " << configDir << std::endl;
        std::cout << "Press Ctrl+Alt+C to show window, Escape to hide" << std::endl;
        std::cout << "Press Ctrl+C in terminal to exit" << std::endl;
        
#ifdef __linux__
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
                        } else if (keysym == XK_j) {
                            // Move down
                            if (!items.empty() && selectedItem < items.size() - 1) {
                                selectedItem++;
                                drawConsole();
                            }
                        } else if (keysym == XK_k) {
                            // Move up
                            if (selectedItem > 0) {
                                selectedItem--;
                                drawConsole();
                            }
                        } else if (keysym == XK_g) {
                            // gg - go to top (need to detect double g)
                            static auto lastGTime = std::chrono::steady_clock::now();
                            auto now = std::chrono::steady_clock::now();
                            auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastGTime);
                            
                            if (diff.count() < 500) { // Double g within 500ms
                                selectedItem = 0;
                                drawConsole();
                            }
                            lastGTime = now;
                        } else if (keysym == XK_G) {
                            // Go to bottom
                            if (!items.empty()) {
                                selectedItem = items.size() - 1;
                                drawConsole();
                            }
                        }
                    }
                    break;
                }
            }
        }
#endif

#ifdef _WIN32
        // Windows event loop
        MSG msg;
        while (running && GetMessage(&msg, nullptr, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
#endif

#ifdef __APPLE__
        // macOS event loop
        // This would require NSApplication setup
#endif
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