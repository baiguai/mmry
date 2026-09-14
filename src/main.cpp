#include "main.h"
#include "key_translation.h"
#include "help.h"
#include "ui.h"
#include "config.h"
#include "utils.h"
#include "clipboard_manager.h"

/*

    ============================================================================
    MMRY
    Clipboard Manager
    ============================================================================


    -- SOURCE CODE -------------------------------------------------------------

    https://github.com/baiguai/mmry


    -- PROJECT STRUCTURE -------------------------------------------------------

    key_translations
        This file handles translating key codes from different platforms into
        one common set of key values.

    help
        This file handles the drawing of the help dialog as well as defining
        all of the help entries themselves.

    ui
        For the UI the code is broken out into each platform's logic.
        This file handles the drawing of all the various dialogs, including
        the main window.

    config
        This file handles all of the configuration logic, including handling
        themes and ensuring the required configuration files are present.

    utils
        This file contains various helper methods that are used throughout
        the project.

*/






void ClipboardManager::moveCursorWordLeft()
{
    std::string currentLine = "";
    std::istringstream iss(editDialogInput);
    for (size_t i = 0; i <= editDialogCursorLine; ++i)
    {
        std::getline(iss, currentLine);
    }

    if (editDialogCursorPos > 0)
    {
        size_t pos = editDialogCursorPos;
        while (pos > 0 && isspace(currentLine[pos - 1]))
        {
            pos--;
        }
        while (pos > 0 && !isspace(currentLine[pos - 1]))
        {
            pos--;
        }
        editDialogCursorPos = pos;
    }
}

void ClipboardManager::moveCursorWordRight()
{
    std::string currentLine = "";
    std::istringstream iss(editDialogInput);
    for (size_t i = 0; i <= editDialogCursorLine; ++i)
    {
        std::getline(iss, currentLine);
    }

    if (editDialogCursorPos < currentLine.length())
    {
        size_t pos = editDialogCursorPos;
        while (pos < currentLine.length() && !isspace(currentLine[pos]))
        {
            pos++;
        }
        while (pos < currentLine.length() && isspace(currentLine[pos]))
        {
            pos++;
        }
        editDialogCursorPos = pos;
    }
}

void ClipboardManager::updateFilteredBookmarkClips()
{
    filteredBookmarkClips.clear();
    std::string selectedGroup = bookmarkGroups[selectedViewBookmarkGroup];
    std::string bookmarkFile = config.bookmarksDir + "/bookmarks_" + selectedGroup + ".txt";
    std::ifstream file(bookmarkFile);

    if (file.is_open())
    {
        std::string line;

        while (std::getline(file, line))
        {
            size_t pos = line.find('|');

            if (pos != std::string::npos && pos > 0)
            {
                std::string content = line.substr(pos + 1);

                try
                {
                    std::string decryptedContent = decrypt(content, config);
                    // Perform case-insensitive search
                    std::string lower_decrypted_content = decryptedContent;
                    std::transform(lower_decrypted_content.begin(), lower_decrypted_content.end(), lower_decrypted_content.begin(),
                                   [](unsigned char c){ return std::tolower(c); });

                    std::string lower_filter_text = filterBookmarkClipsText;
                    std::transform(lower_filter_text.begin(), lower_filter_text.end(), lower_filter_text.begin(),
                                   [](unsigned char c){ return std::tolower(c); });

                    if (lower_decrypted_content.find(lower_filter_text) != std::string::npos)
                    {
                        filteredBookmarkClips.push_back(decryptedContent);
                    }
                }
                catch (...)
                {
                    // Fallback to non-decrypted content if decryption fails
                    std::string lower_content = content;
                    std::transform(lower_content.begin(), lower_content.end(), lower_content.begin(),
                                   [](unsigned char c){ return std::tolower(c); });

                    std::string lower_filter_text = filterBookmarkClipsText;
                    std::transform(lower_filter_text.begin(), lower_filter_text.end(), lower_filter_text.begin(),
                                   [](unsigned char c){ return std::tolower(c); });

                    if (lower_content.find(lower_filter_text) != std::string::npos)
                    {
                        filteredBookmarkClips.push_back(content);
                    }
                }
            }
        }

        file.close();
    }

    // Reset selection if no items match

    if (filteredBookmarkClips.empty())
    {
        selectedViewBookmarkItem = 0;
    }
    else if (selectedViewBookmarkItem >= filteredBookmarkClips.size())
    {
        selectedViewBookmarkItem = filteredBookmarkClips.size() - 1;
    }
}

size_t ClipboardManager::getBookmarkItemCount()
{
    if (selectedViewBookmarkGroup >= bookmarkGroups.size())
    {
        return 0;
    }

    std::string selectedGroup = bookmarkGroups[selectedViewBookmarkGroup];
    std::string bookmarkFile = config.bookmarksDir + "/bookmarks_" + selectedGroup + ".txt";
    std::ifstream file(bookmarkFile);
        
    size_t itemCount = 0;

    if (file.is_open())
    {
        std::string line;

        while (std::getline(file, line))
        {
            size_t pos = line.find('|');

            if (pos != std::string::npos && pos > 0)
            {
                itemCount++;
            }
        }

        file.close();
    }

    return itemCount;
}

std::string stringToLower(const std::string& str)
{
    std::string lower_str;

    lower_str.reserve(str.length());

    std::transform(str.begin(), str.end(), std::back_inserter(lower_str),
                   [](unsigned char c){ return std::tolower(c); });

    return lower_str;
}

// Global pointer for signal handling
ClipboardManager* g_manager = nullptr;

#include <signal.h>

void signal_handler(int signal)
{
    std::cout << "\nReceived signal " << signal << ", cleaning up...\n";

    if (g_manager)
    {
        // Just set running to false, don't join in signal handler
        g_manager->setRunning(false);
    }
    exit(0);
}





#ifdef _WIN32
LRESULT CALLBACK MMRYWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Get the ClipboardManager instance
    ClipboardManager* manager = nullptr;

    if (msg == WM_NCCREATE)
    {
        // Store instance pointer
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
        manager = (ClipboardManager*)pCreate->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)manager);
    }
    else
    {
        manager = (ClipboardManager*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }
    
    switch (msg)
    {
        case WM_SIZE:
            if (manager)
            {
                int newWidth = LOWORD(lParam);
                int newHeight = HIWORD(lParam);
                manager->updateWindowDimensions(newWidth, newHeight);
                manager->drawConsole();
            }
            return 0;

        case WM_KEYDOWN:
            // Process WM_KEYDOWN only - this prevents double processing
            if (manager)
            {
                MSG winMsg = {hwnd, msg, wParam, lParam, 0, 0, 0};
                manager->handleKeyPressCommon(&winMsg);
            }
            return 0;

        case WM_CHAR:
            // Skip WM_CHAR to prevent double processing
            // All key handling is done via WM_KEYDOWN
            return 0;

        case WM_PAINT:
            if (manager)
            {
                PAINTSTRUCT ps;
                BeginPaint(hwnd, &ps);
                manager->drawConsole();
                EndPaint(hwnd, &ps);
            }
            return 0;

        case WM_ERASEBKGND:
            // Return 1 to indicate we handled background erasing
            // This prevents Windows from erasing to white
            return 1;

        case WM_CLIPBOARDUPDATE:
            if (manager)
            {
                if (OpenClipboard(hwnd))
                {
                    if (IsClipboardFormatAvailable(CF_TEXT))
                    {
                        HANDLE hData = GetClipboardData(CF_TEXT);
                        if (hData)
                        {
                            char* pszText = static_cast<char*>(GlobalLock(hData));
                            if (pszText)
                            {
                                manager->processClipboardContent(pszText);
                                GlobalUnlock(hData);
                            }
                        }
                    }
                    CloseClipboard();
                }
            }
            return 0;

        case WM_DESTROY:
            if (manager)
            {
                RemoveClipboardFormatListener(hwnd);
            }
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

#endif



int main_linux()
{
#ifdef __linux__
        // Install signal handlers for graceful shutdown
        signal(SIGTERM, signal_handler);
        signal(SIGINT, signal_handler);

        SingleInstance guard("Mmry");
        if (guard.isAnotherInstanceRunning())
        {
            std::cerr << "Another instance is already running. Exiting.\n";
            return 1;
        }
        
        // Install temporary error handler
        XErrorHandler oldHandler = XSetErrorHandler(ignore_x11_errors);
        (void)oldHandler; // Suppress unused variable warning

        ClipboardManager manager;
        g_manager = &manager;
        manager.run();
        g_manager = nullptr;
#endif

    return 0;
}

int main_windows()
{
#ifdef _WIN32
        // Check for another instance using a named mutex
        HANDLE hMutex = CreateMutexA(NULL, TRUE, "Global\\MmryClipboardManager");
        if (GetLastError() == ERROR_ALREADY_EXISTS)
        {
            std::cerr << "Another instance is already running. Exiting.\n";
            if (hMutex)
            {
                CloseHandle(hMutex);
            }
            MessageBoxA(NULL, "MMRY is already running.", "MMRY", MB_OK | MB_ICONINFORMATION);
            return 1;
        }

        // Install signal handlers for graceful shutdown
        signal(SIGTERM, signal_handler);
        signal(SIGINT, signal_handler);

        ClipboardManager manager;
        g_manager = &manager;
        manager.run();
        g_manager = nullptr;

        // Clean up mutex
        if (hMutex)
        {
            ReleaseMutex(hMutex);
            CloseHandle(hMutex);
        }
#endif

    return 0;
}



int main()
{
    if (main_linux())
    {
        return 1;
    }

    if (main_windows())
    {
        return 1;
    }

    return 0;
}
