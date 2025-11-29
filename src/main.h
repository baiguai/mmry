#ifndef MAIN_H
#define MAIN_H

#include <iostream>
#include <unistd.h>
#include <sys/file.h>
#include <fcntl.h>
#include <cstring>
#include <cerrno>
#include <thread>
#include <atomic>
#include <string>
#include <regex>
#include <algorithm>
#include <vector>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <sys/stat.h>
#include <limits.h>
#include <signal.h>


#ifdef __linux__
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xfixes.h>
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



class SingleInstance {
private:
#ifdef _WIN32
    HANDLE mutex;
#else
    int lockFile;
    std::string lockPath;
#endif

public:
    SingleInstance(const std::string& appName) {
#ifdef _WIN32
        // Windows: Use a named mutex
        std::string mutexName = "Global\\" + appName + "_SingleInstance";
        mutex = CreateMutexA(NULL, FALSE, mutexName.c_str());
        
        if (GetLastError() == ERROR_ALREADY_EXISTS) {
            CloseHandle(mutex);
            mutex = NULL;
        }
#else
        // Unix/Linux/macOS: Use a lock file
        lockPath = "/tmp/" + appName + ".lock";
        lockFile = open(lockPath.c_str(), O_CREAT | O_RDWR, 0666);
        
        if (lockFile == -1) {
            lockFile = -1;
            return;
        }
        
        // Try to acquire exclusive lock
        if (flock(lockFile, LOCK_EX | LOCK_NB) == -1) {
            close(lockFile);
            lockFile = -1;
        }
#endif
    }

    ~SingleInstance() {
#ifdef _WIN32
        if (mutex) {
            CloseHandle(mutex);
        }
#else
        if (lockFile != -1) {
            flock(lockFile, LOCK_UN);
            close(lockFile);
            unlink(lockPath.c_str());
        }
#endif
    }

    bool isAnotherInstanceRunning() const {
#ifdef _WIN32
        return (mutex == NULL);
#else
        return (lockFile == -1);
#endif
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

#endif MAIN_H
