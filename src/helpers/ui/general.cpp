#include "../../clipboard_manager.h"

size_t ClipboardManager::getDisplayItemCount()
{
    if (filterMode)
    {
        return filteredItems.size();
    }
    return items.size();
}

size_t ClipboardManager::getActualItemIndex(size_t displayIndex)
{
    if (filterMode && displayIndex < filteredItems.size())
    {
        return filteredItems[displayIndex];
    }
    return displayIndex;
}

void ClipboardManager::updateFilteredItems()
{
    selectedItem = 0;
    consoleScrollOffset = 0;
    filteredItems.clear();

    if (filterText.empty())
    {
        for (size_t i = 0; i < items.size(); ++i)
        {
            filteredItems.push_back(i);
        }
    }
    else if (filterText[0] == '!')
    {
        // Explicit regex search (after '!' prefix)
        std::string regex_pattern = filterText.substr(1);
        if (!regex_pattern.empty())
        {
            if (isRegexPatternSafe(regex_pattern))
            {
                try
                {
                    std::regex rgx(regex_pattern, std::regex_constants::icase | std::regex_constants::multiline);

                    for (size_t i = 0; i < items.size(); ++i)
                    {
                        if (!items[i].lowercase_content.empty() && 
                            std::regex_search(items[i].lowercase_content, rgx))
                        {
                            filteredItems.push_back(i);
                        }
                    }
                }
                catch (const std::exception& e)
                {
                    writeLog(std::string(__FUNCTION__) + " Regex error: " + std::string(e.what()));
                }
            }
            else
            {
                // Fallback: try to extract terms from common lookahead patterns
                std::vector<std::string> required, forbidden;
                if (extractLookaheadTerms(regex_pattern, required, forbidden))
                {
                    for (size_t i = 0; i < items.size(); ++i)
                    {
                        if (items[i].lowercase_content.empty())
                            continue;

                        bool match = true;
                        for (const auto& term : required)
                        {
                            if (items[i].lowercase_content.find(term) == std::string::npos)
                            {
                                match = false;
                                break;
                            }
                        }
                        if (match)
                        {
                            for (const auto& term : forbidden)
                            {
                                if (items[i].lowercase_content.find(term) != std::string::npos)
                                {
                                    match = false;
                                    break;
                                }
                            }
                        }
                        if (match)
                            filteredItems.push_back(i);
                    }
                }
                else
                {
                    writeLog(std::string(__FUNCTION__) + " Regex pattern contains unsafe constructs");
                }
            }
        }
    }
    else
    {
        // Fast path: simple substring search (most common case)
        if (filterText.find('*') == std::string::npos && 
            filterText.find('?') == std::string::npos &&
            filterText.find('.') == std::string::npos &&
            filterText.find('+') == std::string::npos)
        {
            
            std::string lower_filter = filterText;
            std::transform(lower_filter.begin(), lower_filter.end(), lower_filter.begin(),
                           [](unsigned char c){ return std::tolower(c); });

            for (size_t i = 0; i < items.size(); ++i)
            {
                if (items[i].lowercase_content.find(lower_filter) != std::string::npos)
                {
                    filteredItems.push_back(i);
                }
            }
        }
        else
        {
            // Slow path: regex search for wildcard patterns
            std::string regex_str = wildcardToRegex(filterText);
            if (!isRegexPatternSafe(regex_str))
            {
                writeLog("Regex error: generated pattern contains unsafe constructs");
            }
            else
            {
                try
                {
                    std::regex rgx(regex_str, std::regex_constants::icase);
                    
                    for (size_t i = 0; i < items.size(); ++i)
                    {
                        if (std::regex_search(items[i].content, rgx))
                        {
                            filteredItems.push_back(i);
                        }
                    }
                }
                catch (const std::exception& e)
                {
                    writeLog("Regex error: " + std::string(e.what()));
                }
            }
        }
    }
    
    // Reset selection if no items match
    if (filteredItems.empty())
    {
        selectedItem = 0;
    }
    else if (selectedItem >= filteredItems.size())
    {
        selectedItem = filteredItems.size() - 1;
    }
}
