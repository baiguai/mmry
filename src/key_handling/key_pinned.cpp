#include "../clipboard_manager.h"

bool ClipboardManager::key_pin_down()
{
    auto sortedItems = getSortedPinnedItems(config.pinnedFile);
    if (selectedViewPinnedItem < sortedItems.size() - 1)
    {
        selectedViewPinnedItem++;
        updatePinnedScrollOffset();
        drawConsole();
    }
    return true;
}

bool ClipboardManager::key_pin_up()
{
    if (selectedViewPinnedItem > 0)
    {
        selectedViewPinnedItem--;
        updatePinnedScrollOffset();
        drawConsole();
    }
    return true;
}

bool ClipboardManager::key_pin_top()
{
    selectedViewPinnedItem = 0;
    viewPinnedScrollOffset = 0;
    drawConsole();
    return true;
}

bool ClipboardManager::key_pin_bottom()
{
    auto sortedItems = getSortedPinnedItems(config.pinnedFile);
    if (!sortedItems.empty())
    {
        selectedViewPinnedItem = sortedItems.size() - 1;
        updatePinnedScrollOffset();
        drawConsole();
    }
    return true;
}

bool ClipboardManager::key_pin_delete()
{
    auto sortedItems = getSortedPinnedItems(config.pinnedFile);
    
    // Remove the selected item if valid
    if (selectedViewPinnedItem < sortedItems.size())
    {
        sortedItems.erase(sortedItems.begin() + selectedViewPinnedItem);

        // Write back remaining lines
        std::ofstream outFile(config.pinnedFile);
        if (outFile.is_open())
        {
            for (const auto& item : sortedItems)
            {
                outFile << item << "\n";
            }
            outFile.close();
            
            std::cout << "Deleted pinned clip\n";
            
            // Adjust selection
            if (selectedViewPinnedItem > 0 && selectedViewPinnedItem >= sortedItems.size())
            {
                selectedViewPinnedItem = sortedItems.size() - 1;
            }
            
            // Close dialog if no pinned clips left
            if (sortedItems.empty())
            {
                pinnedDialogVisible = false;
            }
            
            drawConsole();
        }
    }
    return true;
}

bool ClipboardManager::key_pin_copy()
{
    auto sortedItems = getSortedPinnedItems(config.pinnedFile);

    if (selectedViewPinnedItem < sortedItems.size())
    {
        std::string selectedLine = sortedItems[selectedViewPinnedItem];
        size_t pos = selectedLine.find('|');
        if (pos != std::string::npos)
        {
            std::string contentToCopy;
            std::string contentToSave = selectedLine.substr(pos + 1);
            try
            {
                contentToCopy = decrypt(contentToSave, config);
            }
            catch (...)
            {
                contentToCopy = contentToSave;
            }

            copyToClipboard(contentToCopy);

            int lineCount = countLines(contentToCopy);
            if (lineCount > 1)
            {
                std::cout << "Copied " << lineCount << " lines from pinned clips\n";
            }
            else
            {
                std::cout << "Copied from pinned clips: " << contentToCopy.substr(0, 50) << "...\n";
            }

            // Remove the old line from sorted items
            sortedItems.erase(sortedItems.begin() + selectedViewPinnedItem);

            // Add the new line at the beginning
            auto newTimestamp = std::chrono::system_clock::now().time_since_epoch().count();
            std::string newLine = std::to_string(newTimestamp) + "|" + contentToSave;
            sortedItems.insert(sortedItems.begin(), newLine);
            
            // Write the updated lines back to the file
            std::ofstream outFile(config.pinnedFile);
            for (const auto& l : sortedItems)
            {
                outFile << l << std::endl;
            }
            outFile.close();
        }

        pinnedDialogVisible = false;
        hideWindow();
    }
    return true;
}
