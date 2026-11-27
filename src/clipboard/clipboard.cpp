#include "../clipboard_manager.h"

/*[./Introduction to Clipboard.cpp]
    The clipboard.cpp handles the system clipboard work.
*/
//!

#ifdef __linux__
    /*!
        Linux Specific Methods
    */

    /*!
        Request Clipboard Content

        !_method
    */
    void ClipboardManager::requestClipboardContent()
    {
        // Request clipboard content as UTF8_STRING
        XConvertSelection(display, clipboardAtom, utf8Atom, clipboardAtom, window, CurrentTime);
    }
    //!

    /*!
        Handle Selection Notify

        !_method
    */
    void ClipboardManager::handleSelectionNotify(XEvent* event)
    {
        if (event->xselection.property == None)
        {
            // If UTF8_STRING is not available, try plain TEXT
            XConvertSelection(display, clipboardAtom, XA_STRING, clipboardAtom, window, CurrentTime);
            return;
        }

        Atom target = event->xselection.target;
        if (target != utf8Atom && target != XA_STRING)
        {
            // We are not interested in other formats
            return;
        }

        Atom type;
        int format;
        unsigned long nitems, bytes_after;
        unsigned char* data = nullptr;

        XGetWindowProperty(display, window, clipboardAtom, 0, LONG_MAX, False, AnyPropertyType,
                           &type, &format, &nitems, &bytes_after, &data);

        if (data)
        {
            std::string content(reinterpret_cast<char*>(data), nitems);
            XFree(data);
            processClipboardContent(content);
        }
    }
    //!
    //!
#endif

/*!
    Process Clipboard Content

    !_method
*/
void ClipboardManager::processClipboardContent(const std::string& content)
{
    // Trim trailing newlines
    std::string trimmed_content = content;
    while (!trimmed_content.empty() && (trimmed_content.back() == '\n' || trimmed_content.back() == '\r'))
    {
        trimmed_content.pop_back();
    }

    if (trimmed_content.empty() || trimmed_content == lastClipboardContent)
    {
        return;
    }

    lastClipboardContent = trimmed_content;

    // The rest of the logic from checkClipboard
    size_t duplicateIndex = 0;
    
    // Check for duplicates and move to top if found
    bool isDuplicate = false;
    for (size_t i = 0; i < items.size(); i++)
    {
        if (items[i].content == trimmed_content)
        {
            isDuplicate = true;
            duplicateIndex = i;
            break;
        }
    }
  
    if (isDuplicate)
    {
        // Move existing clip to top
        std::string clipContent = items[duplicateIndex].content;
        items.erase(items.begin() + duplicateIndex);
        items.emplace(items.begin(), clipContent);

        // Reset selection to top when item is moved
        selectedItem = 0;

        // Update filtered items if in filter mode
        if (filterMode)
        {
            updateFilteredItems();
        }

        saveToFile();

        std::cout << "Existing clip moved to top\n";

        // Refresh display if window is visible
        if (visible)
        {
            drawConsole();
        }

        return;
    }

    items.emplace(items.begin(), trimmed_content);
    while (items.size() > config.maxClips)
    {
        items.pop_back();
    }
    
    // Reset selection to top when new item is added
    selectedItem = 0;
    
    // Update filtered items if in filter mode
    if (filterMode)
    {
        updateFilteredItems();
    }
    
    saveToFile();
    
    std::cout << "New clipboard item added\n";
    
    // Refresh display if window is visible
    if (visible)
    {
        drawConsole();
    }
}
//!

/*!
    Copy To Clipboard

    !_method
*/
void ClipboardManager::copyToClipboard(const std::string& content)
{
#ifdef __linux__
    /*!
        Linux Copy To
    */
    //>>
    // Use xclip to copy to clipboard
    FILE* pipe = popen("xclip -selection clipboard", "w");
    if (pipe)
    {
        fwrite(content.c_str(), 1, content.length(), pipe);
        pclose(pipe);
    }
    //<<
    //!
#endif

#ifdef _WIN32
    /*!
        Windows Copy To
    */
    //>>
    // Windows clipboard
    if (OpenClipboard(nullptr))
    {
        EmptyClipboard();
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, content.length() + 1);
        if (hMem)
        {
            memcpy(GlobalLock(hMem), content.c_str(), content.length() + 1);
            GlobalUnlock(hMem);
            SetClipboardData(CF_TEXT, hMem);
        }
        CloseClipboard();
    }
    //<<
    //!
#endif

    /*!
        Apple Copy
    */
#ifdef __APPLE__
    //>>
    // macOS clipboard using pbcopy
    FILE* pipe = popen("pbcopy", "w");
    if (pipe)
    {
        fwrite(content.c_str(), 1, content.length(), pipe);
        pclose(pipe);
    }
    //<<
    //!
#endif
}
//!
