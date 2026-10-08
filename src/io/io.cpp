#include "../clipboard_manager.h"

void ClipboardManager::saveToFile()
{
    std::ofstream file(config.dataFile);
    if (file.is_open())
    {
        for (const auto& item : items)
        {
            // Store timestamp and content (encrypted if enabled)
            auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                item.timestamp.time_since_epoch()).count();
            std::string contentToSave = encrypt(item.content, config);
            file << timestamp << "|" << contentToSave << "\n";
        }
        file.close();
    }
}

void ClipboardManager::loadFromFile()
{
    std::ifstream file(config.dataFile);
    if (file.is_open())
    {
        std::string line;
        while (std::getline(file, line))
        {
            size_t pos = line.find('|');
            if (pos != std::string::npos && pos > 0)
            {
                std::string timestampStr = line.substr(0, pos);
                std::string content = line.substr(pos + 1);
                
                try
                {
                    std::string decryptedContent;
                    
                    // Try to decrypt first
                    try
                    {
                        decryptedContent = decrypt(content, config);
                        // Check if decryption produced reasonable results (no control characters)
                        bool hasControlChars = false;
                        for (char c : decryptedContent)
                        {
                            if (c < 32 && c != '\n' && c != '\r' && c != '\t')
                            {
                                hasControlChars = true;
                                break;
                            }
                        }
                        
                        // If decryption produced garbage, assume the content was never encrypted
                        if (hasControlChars || decryptedContent.empty())
                        {
                            decryptedContent = content;
                        }
                    }
                    catch (...)
                    {
                        // If decryption fails, assume content was never encrypted
                        decryptedContent = content;
                    }
                    
                    ClipboardItem item(decryptedContent);
                    auto timestamp = std::chrono::seconds(std::stoll(timestampStr));
                    item.timestamp = std::chrono::system_clock::time_point(timestamp);
                    
                    items.push_back(item);
                }
                catch (const std::exception& e)
                {
                    // Skip invalid entries
                    continue;
                }
            }
        }
        file.close();
    }
    else
    {
        // Create empty clips.txt file if it doesn't exist
        std::ofstream outFile(config.dataFile);
        if (outFile.is_open())
        {
            outFile.close();
            std::cout << "Created empty clips file: " << config.dataFile << "\n";
        }
        else
        {
            std::cerr << "Failed to create clips file: " << config.dataFile << "\n";
        }
    }
}
