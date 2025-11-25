#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <string>
#include <vector>

class ConfigManager {
public:
    ConfigManager();
    void loadConfig();
    void setupConfigDir();

    // Getters for configuration properties
    std::string getConfigDir() const;
    std::string getBookmarksDir() const;
    std::string getDataFile() const;
    size_t getMaxClips() const;
    bool isEncrypted() const;
    std::string getEncryptionKey() const;
    std::string getTheme() const;
    bool getAutoStart() const;

private:
    std::string configDir;
    std::string bookmarksDir;
    std::string dataFile;
    size_t maxClips;
    bool encrypted;
    std::string encryptionKey;
    std::string theme;
    bool autoStart;

    void createDefaultConfig(const std::string& configPath);
};

#endif // CONFIG_MANAGER_H