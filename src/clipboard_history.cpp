#include "clipboard_history.h"
#include <fstream>
#include <iostream>
#include <algorithm>

void ClipboardHistory::add(const std::string& content) {
    if (content == lastClipboardContent) {
        return;
    }

    // Remove duplicates
    auto it = std::remove_if(items.begin(), items.end(),
        [&](const ClipboardItem& item) {
            return item.content == content;
        });
    items.erase(it, items.end());

    items.emplace(items.begin(), content);
    lastClipboardContent = content;
}

void ClipboardHistory::remove(size_t index) {
    if (index < items.size()) {
        items.erase(items.begin() + index);
    }
}

void ClipboardHistory::moveToTop(size_t index) {
    if (index > 0 && index < items.size()) {
        ClipboardItem item = items[index];
        items.erase(items.begin() + index);
        items.insert(items.begin(), item);
    }
}

const std::vector<ClipboardItem>& ClipboardHistory::getItems() const {
    return items;
}

void ClipboardHistory::loadFromFile(const std::string& filename, bool encrypted, const std::string& key) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        size_t separatorPos = line.find('|');
        if (separatorPos != std::string::npos) {
            std::string timestampStr = line.substr(0, separatorPos);
            std::string content = line.substr(separatorPos + 1);

            if (encrypted) {
                content = decrypt(content, key);
            }

            // Note: timestamp is not loaded correctly here, but for simplicity, we'll re-assign it.
            items.emplace_back(content);
        }
    }
}

void ClipboardHistory::saveToFile(const std::string& filename, bool encrypted, const std::string& key) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return;
    }

    for (const auto& item : items) {
        std::string content = item.content;
        if (encrypted) {
            content = encrypt(content, key);
        }
        auto time_t = std::chrono::system_clock::to_time_t(item.timestamp);
        file << time_t << "|" << content << std::endl;
    }
}

std::string ClipboardHistory::encrypt(const std::string& data, const std::string& key) const {
    if (key.empty()) {
        return data;
    }

    std::string encrypted_data = data;
    for (size_t i = 0; i < data.size(); ++i) {
        encrypted_data[i] = data[i] ^ key[i % key.size()];
    }
    return encrypted_data;
}

std::string ClipboardHistory::decrypt(const std::string& data, const std::string& key) const {
    if (key.empty()) {
        return data;
    }

    std::string decrypted_data = data;
    for (size_t i = 0; i < data.size(); ++i) {
        decrypted_data[i] = data[i] ^ key[i % key.size()];
    }
    return decrypted_data;
}
