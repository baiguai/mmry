#ifndef MAIN_H
#define MAIN_H

/*!
    Main Header Includes

*/
//>>
#include <iostream>
#include <string>
#include <cstdio>
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
#include <dirent.h>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <regex>


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

#if defined(__linux__) || defined(__APPLE__)
typedef void* HDC;
#endif
//<<
//!


/*!
    Clipboard Item Struct
*/
//>>
struct ClipboardItem
{
    std::string content;
    std::string lowercase_content;
    std::chrono::system_clock::time_point timestamp;
    
    ClipboardItem(const std::string& content) 
        : content(content), timestamp(std::chrono::system_clock::now())
    {
        lowercase_content.reserve(content.length());
        std::transform(content.begin(), content.end(), std::back_inserter(lowercase_content),
                       [](unsigned char c){ return std::tolower(c); });
    }
};
//<<
//!



/*!
    Single Instance Class
*/
class SingleInstance
{

    /*!
        Private Members
    */
    private:
#ifdef _WIN32
            /*!
                Windows
            */
            //>>
        HANDLE mutex;
            //<<
            //!
#else
            /*!
                Linux
            */
            //>>
        int lockFile;
        std::string lockPath;
            //<<
            //!
#endif
        //!

        /*!
            Public Members
        */
    public:

        /*!
            Single Instance

            !_method
        */
        SingleInstance(const std::string& appName)
        {

            /*!
                Windows Single Instance
            */
#ifdef _WIN32
            //>>
            // Windows: Use a named mutex
            std::string mutexName = "Global\\" + appName + "_SingleInstance";
            mutex = CreateMutexA(NULL, FALSE, mutexName.c_str());
            
            if (GetLastError() == ERROR_ALREADY_EXISTS)
            {
                CloseHandle(mutex);
                mutex = NULL;
            }
            //<<
            //!
#else
            /*!
                Linux Single Instance
            */
            //>>
            // Unix/Linux/macOS: Use a lock file
            lockPath = "/tmp/" + appName + ".lock";
            lockFile = open(lockPath.c_str(), O_CREAT | O_RDWR, 0666);
            
            if (lockFile == -1)
            {
                lockFile = -1;
                return;
            }
            
            // Try to acquire exclusive lock
            if (flock(lockFile, LOCK_EX | LOCK_NB) == -1)
            {
                close(lockFile);
                lockFile = -1;
            }
            //<<
            //!
#endif
        }
        //!

        /*!
            Single Instance Destructor
        */
        ~SingleInstance()
        {
#ifdef _WIN32
            /*!
                Windows Destructor
            */
            //>>
            if (mutex)
            {
                CloseHandle(mutex);
            }
            //<<
            //!
#else
            /*!
                Linux Destructor
            */
            //>>
            if (lockFile != -1)
            {
                flock(lockFile, LOCK_UN);
                close(lockFile);
                unlink(lockPath.c_str());
            }
            //<<
            //!
#endif
        }
        //!

        /*!
            Another Instance Check

            !_method
        */
        bool isAnotherInstanceRunning() const
        {
#ifdef _WIN32
            /*!
                Window Instance Check
            */
            //>>
            return (mutex == NULL);
            //<<
            //!
#else
            /*!
                Linux Instance Check
            */
            //>>
            return (lockFile == -1);
            //<<
            //!
#endif
        }
        //!
};
        //!

/*!
    String to Lower Helper
*/
//>>
std::string stringToLower(const std::string& str);
//<<
//!


/*!
    Ignore Bad Access Errors
*/
#ifdef __linux__
    /*!
        Linux Access Errors
    */
    //>>
inline int ignore_x11_errors(Display* d, XErrorEvent* e)
{
    (void)d; // Suppress unused parameter warning
    // 10 = BadAccess
    if (e->error_code == BadAccess)
    {
        return 0; // ignore the error
    }
    return 0; // ignore all other errors too
}
    //<<
    //!
#endif

#ifdef _WIN32
    /*!
        Windows Access Errors
    */
    //>>
    LRESULT CALLBACK MMRYWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    //<<
    //!
#endif
//!

/*!
    Global Variables
*/
    //>>
    inline std::atomic<bool> running;
    inline std::atomic<bool> visible;
    // Window properties
    inline int windowWidth { 800 };
    inline int windowHeight { 600 };
    const int WINDOW_X { 100 };
    const int WINDOW_Y { 100 };
    const int LINE_HEIGHT { 25 };
    const int WIN_SEL_RECT_HEIGHT { 26 };
    const int WIN_SEL_RECT_OFFSET_Y { 4 };
    const int FONT_SIZE { 16 };
    const std::string FONT_NAME { "Consolas" };

    // Minimum window size constraints
    const int MIN_WINDOW_WIDTH { 425 };
    const int MIN_WINDOW_HEIGHT { 525 };

    // Dynamic width adjustment for clip list
    inline int clipListWidth { 780 }; // Default width (windowWidth - 20 for margins)

    // Clipboard data
    inline std::vector<ClipboardItem> items;
    inline std::string lastClipboardContent;

    // Navigation
    inline size_t selectedItem { 0 };
    inline size_t consoleScrollOffset { 0 }; // For scrolling main clips list
    inline bool filterMode { false };
    inline std::string filterText;
    inline std::vector<size_t> filteredItems;
    inline bool regexSubmitted { false };
    inline int pendingRepeatCount { 0 };

    // Command mode
    inline bool commandMode { false };
    inline std::string commandText;

    // Theme selection mode
    inline bool cmd_themeSelectMode { false };
    inline std::vector<std::string> availableThemes;
    inline size_t selectedTheme { 0 };
    inline size_t themeSelectScrollOffset { 0 };

    // Config selection mode
    inline bool cmd_configSelectMode { false };
    inline std::vector<std::string> availableConfigs;
    inline size_t selectedConfig { 0 };
    inline size_t configSelectScrollOffset { 0 };



    // Bookmark dialog
    inline bool bookmarkDialogVisible { false };
    inline std::string bookmarkDialogInput;
    inline std::vector<std::string> bookmarkGroups;
    inline size_t selectedBookmarkGroup { 0 };
    inline size_t bookmarkMgmtScrollOffset { 0 }; // For scrolling long lists

    // Pinned dialog
    inline bool pinnedDialogVisible { false };
    inline size_t selectedViewPinnedItem { 0 };
    inline size_t viewPinnedScrollOffset { 0 }; // For scrolling long lists
    inline int m_maxVisiblePinnedItems { 1 }; // Stores the number of currently visible pinned items

    // Add to bookmark dialog state
    inline bool addToBookmarkDialogVisible { false };
    inline size_t selectedAddBookmarkGroup { 0 };
    inline size_t addBookmarkScrollOffset { 0 }; // For scrolling long lists

    // Help dialog state
    inline bool helpDialogVisible { false };
    inline int helpDialogScrollOffset { 0 };
    inline bool helpFilterMode { false };
    inline size_t helpFilterScrollOffset { 0 };

    // Edit dialog state
    inline bool editDialogVisible { false };
    inline std::string editDialogInput;
    inline int editDialogScrollOffset { 0 };
    inline size_t editDialogCursorPos { 0 };
    inline size_t editDialogCursorLine { 0 };

    // View bookmarks dialog state
    inline bool viewBookmarksDialogVisible { false };
    inline bool viewBookmarksShowingGroups { true }; // true = groups, false = clips
    inline size_t selectedViewBookmarkGroup { 0 };
    inline size_t selectedViewBookmarkItem { 0 };
    inline size_t viewBookmarksScrollOffset { 0 }; // For scrolling long lists

    // Bookmark dialog filtering
    inline bool filterBookmarksMode { false };
    inline std::string filterBookmarksText;

    inline bool filterAddBookmarksMode { false };
    inline std::string filterAddBookmarksText;

    // Bookmark clips filtering
    inline bool filterBookmarkClipsMode { false };
    inline std::string filterBookmarkClipsText;
    inline std::vector<std::string> filteredBookmarkClips;
    //<<
//!

#endif
