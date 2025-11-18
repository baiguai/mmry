#include "clipboardmanager.h"
#include <QClipboard>
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>

ClipboardManager::ClipboardManager(QObject* parent) 
    : QObject(parent), nextId(1) {
    loadFromFile();
    setupClipboardMonitoring();
}

ClipboardManager::~ClipboardManager() {
    saveToFile();
}

void ClipboardManager::startMonitoring() {
    // Monitoring is set up in constructor
}

void ClipboardManager::setupClipboardMonitoring() {
    QClipboard* clipboard = QApplication::clipboard();
    if (clipboard) {
        connect(clipboard, &QClipboard::dataChanged, this, &ClipboardManager::onClipboardChanged);
    }
}

void ClipboardManager::onClipboardChanged() {
    QClipboard* clipboard = QApplication::clipboard();
    if (clipboard) {
        QString text = clipboard->text();
        if (!text.isEmpty()) {
            // Check if this is a duplicate
            bool isDuplicate = false;
            for (const auto& item : items) {
                if (item.content == text.toStdString()) {
                    isDuplicate = true;
                    break;
                }
            }
            
            if (!isDuplicate) {
                items.emplace_back(text.toStdString(), nextId++);
                saveToFile();
            }
        }
    }
}

void ClipboardManager::saveToFile() const {
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataPath);
    QString filePath = dataPath + "/clipboard.json";
    
    QJsonArray jsonArray;
    for (const auto& item : items) {
        QJsonObject jsonItem;
        jsonItem["id"] = static_cast<qint64>(item.id);
        jsonItem["content"] = QString::fromStdString(item.content);
        jsonItem["timestamp"] = QString::fromStdString(std::to_string(
            std::chrono::duration_cast<std::chrono::seconds>(
                item.timestamp.time_since_epoch()
            ).count()
        ));
        jsonArray.append(jsonItem);
    }
    
    QJsonDocument doc(jsonArray);
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
    }
}

void ClipboardManager::loadFromFile() {
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString filePath = dataPath + "/clipboard.json";
    
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonArray jsonArray = doc.array();
        
        items.clear();
        for (const auto& value : jsonArray) {
            QJsonObject jsonItem = value.toObject();
            std::string content = jsonItem["content"].toString().toStdString();
            size_t id = static_cast<size_t>(jsonItem["id"].toInt());
            
            ClipboardItem item(content, id);
            items.push_back(item);
            
            if (id >= nextId) {
                nextId = id + 1;
            }
        }
    }
}

#include "clipboardmanager.moc"