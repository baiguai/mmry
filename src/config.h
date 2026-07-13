#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>
#include <map>

class ConfigManager
{
public:
    std::string configDir;
    std::string bookmarksDir;
    std::string dataFile;
    std::string pinnedFile;

    size_t maxClips { 500 };
    bool encrypted { false };
    std::string encryptionKey;
    std::string theme { "console" };
    std::string originalTheme;
    bool autoStart { false };
    bool verboseMode { false };
    bool m_debugging { true };

    unsigned long backgroundColor { 0 };
    unsigned long textColor { 0 };
    unsigned long selectionColor { 0 };
    unsigned long borderColor { 0 };

    void setupConfigDir();
    void ensureRequiredFiles();

    void loadConfig();
    void saveConfig();
    void createDefaultConfig();

    std::string getConfigValue(const std::string& key);
    std::string getConfigType(const std::string& key);
    bool updateConfigValue(const std::string& key, const std::string& value);

    void loadTheme();
    void createDefaultThemeFile();
    void switchTheme(const std::string& themeName);

    std::vector<std::string> discoverThemes();
    std::vector<std::string> discoverConfigs();

    unsigned long hexToRgb(const std::string& hex);
};

#endif
