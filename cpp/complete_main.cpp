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
    size_t maxClips = 500;
    bool encrypted = false;
    std::string encryptionKey = "";
    std::string theme = "console";
    
    // Navigation
    size_t selectedItem = 0;
    bool filterMode = false;
    std::string filterText;
    std::vector<size_t> filteredItems;
    
    // Helper methods
    size_t getDisplayItemCount() {
        if (filterMode) {
            return filteredItems.size();
        }
        return items.size();
    }
    
    size_t getActualItemIndex(size_t displayIndex) {
        if (filterMode && displayIndex < filteredItems.size()) {
            return filteredItems[displayIndex];
        }
        return displayIndex;
    }
    
    void updateFilteredItems() {
        filteredItems.clear();
        
        if (filterText.empty()) {
            for (size_t i = 0; i < items.size(); ++i) {
                filteredItems.push_back(i);
            }
        } else {
            std::string lowerFilter = filterText;
            for (char& c : lowerFilter) {
                c = tolower(c);
            }
            
            for (size_t i = 0; i < items.size(); ++i) {
                std::string lowerContent = items[i].content;
                for (char& c : lowerContent) {
                    c = tolower(c);
                }
                
                if (lowerContent.find(lowerFilter) != std::string::npos) {
                    filteredItems.push_back(i);
                }
            }
        }
        
        // Reset selection if no items match
        if (filteredItems.empty()) {
            selectedItem = 0;
        } else if (selectedItem >= filteredItems.size()) {
            selectedItem = filteredItems.size() - 1;
        }
    }
    
    // Simple XOR encryption helper functions
    std::string encrypt(const std::string& data) {
        if (!encrypted || encryptionKey.empty()) {
            return data;
        }
        
        std::string encrypted;
        encrypted.resize(data.length());
        
        for (size_t i = 0; i < data.length(); ++i) {
            encrypted[i] = data[i] ^ encryptionKey[i % encryptionKey.length()];
        }
        
        // Simple base64 encoding
        const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string result;
        int val = 0, valb = -6;
        
        for (char c : encrypted) {
            val = (val << 8) + (c & 0xff);
            valb += 8;
            while (valb >= 0) {
                result.push_back(chars[(val >> valb) & 0x3f]);
                valb -= 6;
            }
        }
        
        if (valb > -6) {
            result.push_back(chars[((val << 8) >> (valb + 8)) & 0x3f]);
        }
        
        // Pad with '='
        while (result.length() % 4) {
            result.push_back('=');
        }
        
        return result;
    }
    
    std::string decrypt(const std::string& data) {
        if (!encrypted || encryptionKey.empty()) {
            return data;
        }
        
        // Simple base64 decoding
        const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string decoded;
        int val = 0, valb = -8;
        
        for (char c : data) {
            if (c == '=') break;
            
            size_t pos = chars.find(c);
            if (pos == std::string::npos) continue;
            
            val = (val << 6) + pos;
            valb += 6;
            
            if (valb >= 0) {
                decoded.push_back((val >> valb) & 0xff);
                valb -= 8;
            }
        }
        
        // XOR decrypt
        std::string decrypted;
        decrypted.resize(decoded.length());
        
        for (size_t i = 0; i < decoded.length(); ++i) {
            decrypted[i] = decoded[i] ^ encryptionKey[i % encryptionKey.length()];
        }
        
        return decrypted;
    }
    
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
#ifdef __linux__
        if (font) XFreeFont(display, font);
        if (gc) XFreeGC(display, gc);
        if (window) XDestroyWindow(display, window);
        if (display) XCloseDisplay(display);
#endif
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
        
        // Don't grab Escape globally - let window handle it
        // This allows Escape to work properly in filter mode
        
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
        
        // Draw filter textbox if in filter mode
        int startY = 20;
        if (filterMode) {
            // Draw filter input
            std::string filterDisplay = "/" + filterText;
            XDrawString(display, window, gc, 10, startY, filterDisplay.c_str(), filterDisplay.length());
            startY += 20;
        }
        
        // Draw clipboard items
        int y = startY;
        int maxItems = (WINDOW_HEIGHT - startY - 20) / 15; // Approximate lines that fit
        
        size_t displayCount = getDisplayItemCount();
        for (size_t i = 0; i < displayCount && i < maxItems; ++i) {
            size_t actualIndex = getActualItemIndex(i);
            const auto& item = items[actualIndex];
            
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
                
                // Add line count if more than one line
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
        
        if (displayCount == 0) {
            std::string empty = filterMode ? "No matching items..." : "No clipboard items yet...";
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
                if (items.size() > maxClips) { // Keep only last maxClips items
                    items.pop_back();
                }
                
                // Reset selection to top when new item is added
                selectedItem = 0;
                
                // Update filtered items if in filter mode
                if (filterMode) {
                    updateFilteredItems();
                }
                
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
                // Parse verbose
                if (line.find("\"verbose\"") != std::string::npos) {
                    verboseMode = line.find("true") != std::string::npos;
                }
                // Parse max_clips
                else if (line.find("\"max_clips\"") != std::string::npos) {
                    size_t colon = line.find(':');
                    if (colon != std::string::npos) {
                        std::string value = line.substr(colon + 1);
                        // Remove whitespace and commas
                        value.erase(0, value.find_first_not_of(" \t"));
                        value.erase(value.find_last_not_of(" \t,") + 1);
                        maxClips = std::stoull(value);
                    }
                }
                // Parse encrypted
                else if (line.find("\"encrypted\"") != std::string::npos) {
                    encrypted = line.find("true") != std::string::npos;
                }
                // Parse encryption_key
                else if (line.find("\"encryption_key\"") != std::string::npos) {
                    size_t start = line.find('"', line.find(':'));
                    size_t end = line.find('"', start + 1);
                    if (start != std::string::npos && end != std::string::npos) {
                        encryptionKey = line.substr(start + 1, end - start - 1);
                    }
                }
                // Parse theme
                else if (line.find("\"theme\"") != std::string::npos) {
                    size_t start = line.find('"', line.find(':'));
                    size_t end = line.find('"', start + 1);
                    if (start != std::string::npos && end != std::string::npos) {
                        theme = line.substr(start + 1, end - start - 1);
                    }
                }
            }
            file.close();
        } else {
            // Create default config with all settings
            createDefaultConfig();
        }
    }
    
    void createDefaultConfig() {
        std::string configFile = configDir + "/config.json";
        std::ofstream outFile(configFile);
        outFile << "{\n";
        outFile << "    \"verbose\": false,\n";
        outFile << "    \"max_clips\": 500,\n";
        outFile << "    \"encrypted\": true,\n";
        outFile << "    \"encryption_key\": \"mmry_default_key_2024\",\n";
        outFile << "    \"theme\": \"console\"\n";
        outFile << "}\n";
        outFile.close();
        
        std::cout << "Created default config at: " << configFile << std::endl;
    }
    
    int countLines(const std::string& content) {
        if (content.empty()) return 0;
        int lines = 1;
        for (char c : content) {
            if (c == '\n') lines++;
        }
        return lines;
    }
    
    void copyToClipboard(const std::string& content) {
#ifdef __linux__
        // Use xclip to copy to clipboard
        FILE* pipe = popen("xclip -selection clipboard", "w");
        if (pipe) {
            fwrite(content.c_str(), 1, content.length(), pipe);
            pclose(pipe);
        }
#endif

#ifdef _WIN32
        // Windows clipboard
        if (OpenClipboard(nullptr)) {
            EmptyClipboard();
            HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, content.length() + 1);
            if (hMem) {
                memcpy(GlobalLock(hMem), content.c_str(), content.length() + 1);
                GlobalUnlock(hMem);
                SetClipboardData(CF_TEXT, hMem);
            }
            CloseClipboard();
        }
#endif

#ifdef __APPLE__
        // macOS clipboard using pbcopy
        FILE* pipe = popen("pbcopy", "w");
        if (pipe) {
            fwrite(content.c_str(), 1, content.length(), pipe);
            pclose(pipe);
        }
#endif
    }
    
    void saveToFile() {
        std::ofstream file(dataFile);
        if (file.is_open()) {
            for (const auto& item : items) {
                // Store timestamp and content (encrypted if enabled)
                auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                    item.timestamp.time_since_epoch()).count();
                std::string contentToSave = encrypt(item.content);
                file << timestamp << "|" << contentToSave << "\n";
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
                if (pos != std::string::npos && pos > 0) {
                    std::string timestampStr = line.substr(0, pos);
                    std::string content = line.substr(pos + 1);
                    
                    try {
                        std::string decryptedContent;
                        
                        // Try to decrypt first
                        try {
                            decryptedContent = decrypt(content);
                            // Check if decryption produced reasonable results (no control characters)
                            bool hasControlChars = false;
                            for (char c : decryptedContent) {
                                if (c < 32 && c != '\n' && c != '\r' && c != '\t') {
                                    hasControlChars = true;
                                    break;
                                }
                            }
                            
                            // If decryption produced garbage, assume the content was never encrypted
                            if (hasControlChars || decryptedContent.empty()) {
                                decryptedContent = content;
                            }
                        } catch (...) {
                            // If decryption fails, assume content was never encrypted
                            decryptedContent = content;
                        }
                        
                        ClipboardItem item(decryptedContent);
                        auto timestamp = std::chrono::seconds(std::stoll(timestampStr));
                        item.timestamp = std::chrono::system_clock::time_point(timestamp);
                        
                        items.push_back(item);
                    } catch (const std::exception& e) {
                        // Skip invalid entries
                        continue;
                    }
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
                        }
                    } 
                    // Check if it's a local window hotkey
                    else if (keyEvent->window == window) {
                        if (keysym == XK_Escape) {
                            if (filterMode) {
                                // Exit filter mode but keep window open
                                filterMode = false;
                                filterText = "";
                                filteredItems.clear();
                                selectedItem = 0;
                                drawConsole();
                            } else {
                                hideWindow();
                            }
                        } else if (keysym == XK_q && (keyEvent->state & ShiftMask) && !filterMode) {
                            stop();
                        } else if (filterMode) {
                            // In filter mode, allow navigation and text input
                            if (keysym == XK_BackSpace) {
                                // Handle backspace in filter mode
                                if (!filterText.empty()) {
                                    filterText.pop_back();
                                    updateFilteredItems();
                                    selectedItem = 0;
                                    drawConsole();
                                }
                            } else if (keysym == XK_Up) {
                                // Navigate up in filtered results
                                if (getDisplayItemCount() > 0 && selectedItem > 0) {
                                    selectedItem--;
                                    drawConsole();
                                }
                            } else if (keysym == XK_Down) {
                                // Navigate down in filtered results
                                if (getDisplayItemCount() > 0 && selectedItem < getDisplayItemCount() - 1) {
                                    selectedItem++;
                                    drawConsole();
                                }
                            } else if (keysym == XK_Return) {
                                // Copy selected item to clipboard and hide window
                                if (!items.empty() && selectedItem < getDisplayItemCount()) {
                                    size_t actualIndex = getActualItemIndex(selectedItem);
                                    copyToClipboard(items[actualIndex].content);
                                    int lines = countLines(items[actualIndex].content);
                                    if (lines > 1) {
                                        std::cout << "Copied " << lines << " lines to clipboard" << std::endl;
                                    } else {
                                        std::cout << "Copied to clipboard: " << items[actualIndex].content.substr(0, 50) << "..." << std::endl;
                                    }
                                    filterMode = false;
                                    filterText = "";
                                    filteredItems.clear();
                                    hideWindow();
                                }
                            } else {
                                // Handle text input in filter mode
                                char buffer[10];
                                int count = XLookupString(keyEvent, buffer, sizeof(buffer), nullptr, nullptr);
                                if (count > 0) {
                                    filterText += std::string(buffer, count);
                                    updateFilteredItems();
                                    selectedItem = 0;
                                    drawConsole();
                                }
                            }
                        } else {
                            // Not in filter mode - handle navigation keys
                            if (keysym == XK_j && !(keyEvent->state & ShiftMask)) {
                                // Move down
                                if (getDisplayItemCount() > 0 && selectedItem < getDisplayItemCount() - 1) {
                                    selectedItem++;
                                    drawConsole();
                                }
                            } else if (keysym == XK_k && !(keyEvent->state & ShiftMask)) {
                                // Move up
                                if (selectedItem > 0) {
                                    selectedItem--;
                                    drawConsole();
                                }
                            } else if (keysym == XK_g && !(keyEvent->state & ShiftMask)) {
                                // gg - go to top (need to detect double g)
                                static auto lastGTime = std::chrono::steady_clock::now();
                                auto now = std::chrono::steady_clock::now();
                                auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastGTime);
                                
                                if (diff.count() < 500) { // Double g within 500ms
                                    selectedItem = 0;
                                    drawConsole();
                                }
                                lastGTime = now;
                            } else if (keysym == XK_g && (keyEvent->state & ShiftMask)) {
                                // Go to bottom (Shift+G)
                                if (getDisplayItemCount() > 0) {
                                    selectedItem = getDisplayItemCount() - 1;
                                    drawConsole();
                                }
                            } else if (keysym == XK_d && (keyEvent->state & ShiftMask)) {
                                // Delete selected item
                                if (!items.empty() && selectedItem < getDisplayItemCount()) {
                                    size_t actualIndex = getActualItemIndex(selectedItem);
                                    items.erase(items.begin() + actualIndex);
                                    if (selectedItem >= getDisplayItemCount() && selectedItem > 0) {
                                        selectedItem--;
                                    }
                                    saveToFile();
                                    drawConsole();
                                    std::cout << "Item deleted" << std::endl;
                                }
                            } else if (keysym == XK_slash) {
                                // Enter filter mode
                                filterMode = true;
                                filterText = "";
                                updateFilteredItems();
                                drawConsole();
                            } else if (keysym == XK_Return) {
                                // Copy selected item to clipboard and hide window
                                if (!items.empty() && selectedItem < getDisplayItemCount()) {
                                    size_t actualIndex = getActualItemIndex(selectedItem);
                                    copyToClipboard(items[actualIndex].content);
                                    int lines = countLines(items[actualIndex].content);
                                    if (lines > 1) {
                                        std::cout << "Copied " << lines << " lines to clipboard" << std::endl;
                                    } else {
                                        std::cout << "Copied to clipboard: " << items[actualIndex].content.substr(0, 50) << "..." << std::endl;
                                    }
                                    hideWindow();
                                }
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
    manager.run();
    return 0;
}