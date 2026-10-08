#include "../clipboard_manager.h"

void ClipboardManager::updateWindowDimensions(int newWidth, int newHeight)
{

#ifdef __linux__
    // Enforce minimum window size constraints
    if (newWidth < MIN_WINDOW_WIDTH)
    {
        newWidth = MIN_WINDOW_WIDTH;
    }
    if (newHeight < MIN_WINDOW_HEIGHT)
    {
        newHeight = MIN_WINDOW_HEIGHT;
    }
#endif
    
    
    // if (debugging) logfile << "updateWindowDimensions (internal): newWidth=" << newWidth << ", newHeight=" << newHeight << " -> windowWidth=" << windowWidth << ", windowHeight=" << windowHeight << std::endl;
    windowWidth = newWidth;
    windowHeight = newHeight;
    updateClipListWidth();
    updateConsoleScrollOffset();
}

void ClipboardManager::updateClipListWidth()
{
    // Calculate clip list width with margins (10px on each side)
    clipListWidth = windowWidth - 20;
    
    // Ensure minimum width for usability
    if (clipListWidth < 200)
    {
        clipListWidth = 200;
    }
}
