#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <memory>

struct ClipboardItem {
    std::string content;
    std::chrono::system_clock::time_point timestamp;
    size_t id;
    
    ClipboardItem(const std::string& content, size_t id) 
        : content(content), id(id), timestamp(std::chrono::system_clock::now()) {}
};

class ClipboardManager : public QObject {
    Q_OBJECT

public:
    explicit ClipboardManager(QObject* parent = nullptr);
    ~ClipboardManager();
    
    void startMonitoring();
    std::vector<ClipboardItem> getItems() const { return items; }
    void clearItems() { items.clear(); }

private slots:
    void onClipboardChanged();

private:
    std::vector<ClipboardItem> items;
    size_t nextId;
    
    void setupClipboardMonitoring();
    void saveToFile() const;
    void loadFromFile();
};