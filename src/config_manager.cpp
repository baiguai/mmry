#include "config_manager.h"
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <sys/stat.h>

ConfigManager::ConfigManager()
    : maxClips(500), encrypted(false), theme("console"), autoStart(false) {
}

void ConfigManager::setupConfigDir() {
    configDir = std::string(getenv("HOME")) + "/.config/mmry";
    bookmarksDir = configDir + "/bookmarks";
    dataFile = configDir + "/clipboard_history.dat";

    // Create config directory if it doesn't exist
    mkdir(configDir.c_str(), 0755);
    mkdir(bookmarksDir.c_str(), 0755);
}

void ConfigManager::loadConfig() {
    std::string configPath = configDir + "/config.ini";
    std::ifstream file(configPath);

    if (!file.is_open()) {
        createDefaultConfig(configPath);
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        size_t separatorPos = line.find('=');
        if (separatorPos != std::string::npos) {
            std::string key = line.substr(0, separatorPos);
            std::string value = line.substr(separatorPos + 1);

            if (key == "max_clips") {
                maxClips = std::stoul(value);
            } else if (key == "encrypted") {
                encrypted = (value == "true");
            } else if (key == "encryption_key") {
                encryptionKey = value;
            } else if (key == "theme") {
                theme = value;
            } else if (key == "auto_start") {
                autoStart = (value == "true");
            }
        }
    }
}

void ConfigManager::createDefaultConfig(const std::string& configPath) {
    std::ofstream file(configPath);
    if (file.is_open()) {
        file << "max_clips=500" << std::endl;
        file << "encrypted=false" << std::endl;
        file << "encryption_key=" << std::endl;
        file << "theme=console" << std::endl;
        file << "auto_start=false" << std::endl;
    }
}

std::string ConfigManager::getConfigDir() const {
    return configDir;
}

std::string ConfigManager::getBookmarksDir() const {
    return bookmarksDir;
}

std::string ConfigManager::getDataFile() const {
    return dataFile;
}

size_t ConfigManager::getMaxClips() const {
    return maxClips;
}

bool ConfigManager::isEncrypted() const {
    return encrypted;
}

std::string ConfigManager::getEncryptionKey() const {
    return encryptionKey;
}

std::string ConfigManager::getTheme() const {
    return theme;
}

bool ConfigManager::getAutoStart() const {
    return autoStart;
}
