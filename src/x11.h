#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <string>
#include <vector>


#ifdef __linux__

class X11ClipboardMonitor {
private:
    Display* display;
    Window window;
    Atom clipboard_atom;
    Atom property_atom;
    bool initialized;
    
public:
    X11ClipboardMonitor() : display(nullptr), window(0), initialized(false) {}
    
    ~X11ClipboardMonitor() {
        if (initialized) {
            if (window) XDestroyWindow(display, window);
            if (display) XCloseDisplay(display);
        }
    }
    
    bool init() {
        display = XOpenDisplay(nullptr);
        if (!display) return false;
        
        int screen = DefaultScreen(display);
        window = XCreateSimpleWindow(display, RootWindow(display, screen),
                                   0, 0, 1, 1, 0, 0, 0);
        
        if (!window) {
            XCloseDisplay(display);
            return false;
        }
        
        clipboard_atom = XInternAtom(display, "CLIPBOARD", False);
        property_atom = XInternAtom(display, "CLIPBOARD_MONITOR", False);
        
        // Select for PropertyChange events on the clipboard
        XSelectInput(display, window, PropertyChangeMask);
        
        initialized = true;
        return true;
    }
    
    std::string getClipboardContent() {
        if (!initialized) return "";
        
        // Request clipboard ownership
        XConvertSelection(display, clipboard_atom, XA_STRING, 
                         property_atom, window, CurrentTime);
        XFlush(display);
        
        // Wait for selection notification (with small timeout)
        XEvent event;
        bool got_selection = false;
        int attempts = 0;
        const int max_attempts = 10;
        
        while (!got_selection && attempts < max_attempts) {
            if (XCheckTypedEvent(display, SelectionNotify, &event)) {
                if (event.xselection.property == property_atom) {
                    got_selection = true;
                    break;
                }
            }
            usleep(10000); // 10ms
            attempts++;
        }
        
        if (!got_selection) return "";
        
        // Read the property data
        Atom actual_type;
        int actual_format;
        unsigned long nitems, bytes_after;
        unsigned char* prop = nullptr;
        
        int result = XGetWindowProperty(display, window, property_atom,
                                       0, 4096/4, False, AnyPropertyType,
                                       &actual_type, &actual_format, &nitems,
                                       &bytes_after, &prop);
        
        std::string content;
        if (result == Success && prop && nitems > 0) {
            content = std::string(reinterpret_cast<char*>(prop), nitems);
        }
        
        if (prop) XFree(prop);
        XDeleteProperty(display, window, property_atom);
        
        return content;
    }
    
    bool hasClipboardChanged() {
        if (!initialized) return false;
        
        // Simple check: try to get current content and compare
        static std::string last_content;
        std::string current = getClipboardContent();
        
        if (current != last_content && !current.empty()) {
            last_content = current;
            return true;
        }
        return false;
    }
};

// Temporary error handler to swallow BadAccess errors
int ignore_x11_errors(Display* d, XErrorEvent* e) {
    (void)d; // Suppress unused parameter warning
    // 10 = BadAccess
    if (e->error_code == BadAccess) {
        return 0; // ignore the error
    }
    return 0; // ignore all other errors too
}

#endif
