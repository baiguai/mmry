#ifndef CLIPBOARD_HISTORY_H
#define CLIPBOARD_HISTORY_H

#include <string>
#include <vector>
#include <chrono>

struct ClipboardItem {
    std::string content;
    std::chrono::system_clock::time_point timestamp;

    ClipboardItem(const std::string& content)
        : content(content), timestamp(std::chrono::system_clock::now()) {}
};

class ClipboardHistory {
public:
    void add(const std::string& content);
    void remove(size_t index);
    void moveToTop(size_t index);
    const std::vector<ClipboardItem>& getItems() const;
    void loadFromFile(const std::string& filename, bool encrypted, const std::string& key);
    void saveToFile(const std::string& filename, bool encrypted, const std::string& key) const;

private:
    std::vector<ClipboardItem> items;
    std::string lastClipboardContent;

    std::string encrypt(const std::string& data, const std::string& key) const;
    std::string decrypt(const std::string& data, const std::string& key) const;
};

#endif // CLIPBOARD_HISTORY_H