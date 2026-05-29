#include "config.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <sys/stat.h>
#include <dirent.h>
#include <cstring>
#include <cerrno>

#ifdef __linux__
#include <unistd.h>
#endif
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

// Config keys by type
// If we later add more config keys - be sure to add those here
// Eventually, these will be used instead of the hard coded keys in the code below
// For now - search for: !@!
// to get all the places keys are hard coded
static const std::vector<std::string> booleanKeys = {"verbose", "debugging", "encrypted", "autostart"};
static const std::vector<std::string> numberKeys  = {"max_clips"};
static const std::vector<std::string> stringKeys = {"encryption_key", "theme"};

unsigned long ConfigManager::hexToRgb(const std::string& hex)
{
    if (hex.length() != 7 || hex[0] != '#')
    {
        return 0;
    }
    
    try
    {
        unsigned long r = std::stoul(hex.substr(1, 2), nullptr, 16);
        unsigned long g = std::stoul(hex.substr(3, 2), nullptr, 16);
        unsigned long b = std::stoul(hex.substr(5, 2), nullptr, 16);
        
#ifdef _WIN32
        return b * 256 * 256 + g * 256 + r;
#else
        return r * 256 * 256 + g * 256 + b;
#endif
    }
    catch (...)
    {
        return 0;
    }
}

void ConfigManager::loadTheme()
{
#ifdef _WIN32
    const char pathSep = '\\';
#else
    const char pathSep = '/';
#endif
    backgroundColor = 0x000000;
    textColor = 0xFFFFFF;
    selectionColor = 0x333333;
    borderColor = 0x888888;
    std::string themePath = configDir + pathSep + "themes" + pathSep + theme + ".json";
    std::ifstream file(themePath);
    
    if (!file.is_open())
    {
        themePath = std::string("themes") + pathSep + theme + ".json";
        file.open(themePath);
    }
    
    if (!file.is_open())
    {
        std::cout << "Theme file not found for theme: " << theme << ", using default colors\n";
        return;
    }
    
    std::cout << "Loading theme from: " << themePath << "\n";
    
    std::string line;
    bool inColorsSection = false;
    while (std::getline(file, line))
    {
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        
        if (line.empty() || line[0] == '/' || line[0] == '#')
        {
            continue;
        }
        
        if (line.find("\"colors\"") != std::string::npos)
        {
            inColorsSection = true;
            continue;
        }
        
        if (inColorsSection && line.find("}") != std::string::npos)
        {
            break;
        }
        
        if (inColorsSection)
        {
            if (line.find("\"background\"") != std::string::npos)
            {
                size_t start = line.find('"', line.find(':'));
                size_t end = line.find('"', start + 1);
                if (start != std::string::npos && end != std::string::npos)
                {
                    backgroundColor = hexToRgb(line.substr(start + 1, end - start - 1));
                }
            }
            else if (line.find("\"text\"") != std::string::npos)
            {
                size_t start = line.find('"', line.find(':'));
                size_t end = line.find('"', start + 1);
                if (start != std::string::npos && end != std::string::npos)
                {
                    textColor = hexToRgb(line.substr(start + 1, end - start - 1));
                }
            }
            else if (line.find("\"selection\"") != std::string::npos)
            {
                size_t start = line.find('"', line.find(':'));
                size_t end = line.find('"', start + 1);
                if (start != std::string::npos && end != std::string::npos)
                {
                    selectionColor = hexToRgb(line.substr(start + 1, end - start - 1));
                }
            }
            else if (line.find("\"border\"") != std::string::npos)
            {
                size_t start = line.find('"', line.find(':'));
                size_t end = line.find('"', start + 1);
                if (start != std::string::npos && end != std::string::npos)
                {
                    borderColor = hexToRgb(line.substr(start + 1, end - start - 1));
                }
            }
        }
    }
    file.close();
}

void ConfigManager::createDefaultThemeFile()
{
    std::string themesDir = configDir + "/themes";
    struct stat st = {};
    if (stat(themesDir.c_str(), &st) == -1)
    {
#ifdef _WIN32
        mkdir(themesDir.c_str());
#else
        mkdir(themesDir.c_str(), 0755);
#endif
    }
    
    std::string themeFile = themesDir + "/console.json";
    std::ofstream outFile(themeFile);
    if (outFile.is_open())
    {
        outFile << "{\n";
        outFile << "  \"name\": \"Console\",\n";
        outFile << "  \"description\": \"Default console theme with black background and green text\",\n";
        outFile << "  \"colors\": {\n";
        outFile << "    \"background\": \"#000000\",\n";
        outFile << "    \"text\": \"#FFFFFF\",\n";
        outFile << "    \"selection\": \"#333333\",\n";
        outFile << "    \"border\": \"#444444\"\n";
        outFile << "  }\n";
        outFile << "}\n";
        outFile.close();
        std::cout << "Created default theme file: " << themeFile << "\n";
    }
}

void ConfigManager::switchTheme(const std::string& themeName)
{
    theme = themeName;
    loadTheme();
    std::cout << "Switched to theme: " << themeName << "\n";
}

std::vector<std::string> ConfigManager::discoverThemes()
{
    std::vector<std::string> result;
    std::string userThemesDir = configDir + "/themes";
    std::string localThemesDir = "themes";
    
    auto scanThemesDir = [&](const std::string& themesDir)
    {
        DIR* dir = opendir(themesDir.c_str());
        if (dir)
        {
            struct dirent* entry;
            while ((entry = readdir(dir)) != nullptr)
            {
                std::string filename = entry->d_name;
                if (filename.length() > 5 && filename.substr(filename.length() - 5) == ".json" &&
                    filename != "." && filename != "..")
                {
                    std::string themeName = filename.substr(0, filename.length() - 5);
                    result.push_back(themeName);
                }
            }
            closedir(dir);
        }
    };
    
    scanThemesDir(userThemesDir);
    scanThemesDir(localThemesDir);
    
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    
    return result;
}

std::vector<std::string> ConfigManager::discoverConfigs()
{
    std::vector<std::string> result;
    std::string configFile = configDir + "/config.json";
    std::ifstream file(configFile);

    if (file.is_open())
    {
        std::string line;
        while (std::getline(file, line))
        {
            size_t start = line.find('"');
            if (start != std::string::npos && start != line.rfind('"'))
            {
                size_t end = line.find('"', start + 1);
                if (end != std::string::npos)
                {
                    std::string configKey = line.substr(start + 1, end - start - 1);
                    if (!configKey.empty())
                    {
                        result.push_back(configKey);
                    }
                }
            }
        }
        file.close();
    }
    
    return result;
}

void ConfigManager::setupConfigDir()
{
    std::string exePath;
    
#ifdef _WIN32
    char buf[MAX_PATH];
    if (GetModuleFileNameA(NULL, buf, MAX_PATH))
    {
        exePath = buf;
    }
#elif __linux__
    char buf[4096];
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len != -1)
    {
        buf[len] = '\0';
        exePath = buf;
    }
#elif __APPLE__
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);
    std::string path(size, '\0');
    if (_NSGetExecutablePath(&path[0], &size) == 0)
    {
        path.resize(strlen(path.c_str()));
        exePath = path;
    }
#endif
    
    if (exePath.empty())
    {
        configDir = "./data";
    }
    else
    {
#ifdef _WIN32
        size_t pos = exePath.find_last_of("\\");
#else
        size_t pos = exePath.find_last_of("/");
#endif
        if (pos != std::string::npos)
        {
            configDir = exePath.substr(0, pos) + "/data";
        }
        else
        {
            configDir = "./data";
        }
    }
    
    auto createDirectory = [](const std::string& path)
    {
#ifdef _WIN32
        return mkdir(path.c_str()) == 0 || errno == EEXIST;
#else
        return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;
#endif
    };
    
    struct stat st = {};
    if (stat(configDir.c_str(), &st) == -1)
    {
        if (createDirectory(configDir))
        {
            std::cout << "Created config directory: " << configDir << "\n";
        }
        else
        {
            std::cerr << "Failed to create config directory: " << configDir << "\n";
        }
    }
    
#ifdef _WIN32
    const char pathSep = '\\';
#else
    const char pathSep = '/';
#endif
    
    std::string themesDir = configDir + pathSep + "themes";
    if (stat(themesDir.c_str(), &st) == -1)
    {
        if (createDirectory(themesDir))
        {
            std::cout << "Created themes directory: " << themesDir << "\n";
        }
        else
        {
            std::cerr << "Failed to create themes directory: " << themesDir << "\n";
        }
    }
    
    bookmarksDir = configDir + pathSep + "bookmarks";
    if (stat(bookmarksDir.c_str(), &st) == -1)
    {
        if (createDirectory(bookmarksDir))
        {
            std::cout << "Created bookmarks directory: " << bookmarksDir << "\n";
        }
        else
        {
            std::cerr << "Failed to create bookmarks directory: " << bookmarksDir << "\n";
        }
    }
    
    dataFile = configDir + pathSep + "clips.txt";
    pinnedFile = configDir + pathSep + "pinned.txt";
    
    ensureRequiredFiles();
}

void ConfigManager::ensureRequiredFiles()
{
    std::string configFile = configDir + "/config.json";
    struct stat st = {};
    if (stat(configFile.c_str(), &st) == -1)
    {
        createDefaultConfig();
    }
    
    std::string themeFile = configDir + "/themes/" + theme + ".json";
    if (stat(themeFile.c_str(), &st) == -1)
    {
        createDefaultThemeFile();
    }
    
    std::string bookmarksFile = bookmarksDir + "/bookmarks.txt";
    if (stat(bookmarksFile.c_str(), &st) == -1)
    {
        std::ofstream outFile(bookmarksFile);
        if (outFile.is_open())
        {
            outFile << "default|0\n";
            outFile.close();
            std::cout << "Created bookmarks file: " << bookmarksFile << "\n";
        }
    }
    
    if (stat(dataFile.c_str(), &st) == -1)
    {
        std::ofstream outFile(dataFile);
        if (outFile.is_open())
        {
            outFile.close();
            std::cout << "Created clips file: " << dataFile << "\n";
        }
    }
    if (stat(pinnedFile.c_str(), &st) == -1)
    {
        std::ofstream outFile(pinnedFile);
        if (outFile.is_open())
        {
            outFile.close();
            std::cout << "Created pinned clips file: " << pinnedFile << "\n";
        }
    }
}

// !@!
// When new configuration items are created, they'll need to be added to this method
void ConfigManager::loadConfig()
{
#ifdef _WIN32
    const char pathSep = '\\';
#else
    const char pathSep = '/';
#endif
    
    std::string configFile = configDir + pathSep + "config.json";
    std::ifstream file(configFile);

    if (file.is_open())
    {
        std::string line;
        while (std::getline(file, line))
        {
            if (line.find("\"verbose\"") != std::string::npos)
            {
                verboseMode = line.find("true") != std::string::npos;
            }
            else if (line.find("\"max_clips\"") != std::string::npos)
            {
                size_t colon = line.find(':');
                if (colon != std::string::npos)
                {
                    std::string value = line.substr(colon + 1);
                    value.erase(0, value.find_first_not_of(" \t"));
                    value.erase(value.find_last_not_of(" \t,") + 1);
                    maxClips = std::stoull(value);
                }
            }
            else if (line.find("\"encrypted\"") != std::string::npos)
            {
                encrypted = line.find("true") != std::string::npos;
            }
            else if (line.find("\"autostart\"") != std::string::npos)
            {
                autoStart = line.find("true") != std::string::npos;
            }
            else if (line.find("\"encryption_key\"") != std::string::npos)
            {
                size_t start = line.find('"', line.find(':'));
                size_t end = line.find('"', start + 1);
                if (start != std::string::npos && end != std::string::npos)
                {
                    encryptionKey = line.substr(start + 1, end - start - 1);
                }
            }
            else if (line.find("\"theme\"") != std::string::npos)
            {
                size_t start = line.find('"', line.find(':'));
                size_t end = line.find('"', start + 1);
                if (start != std::string::npos && end != std::string::npos)
                {
                    theme = line.substr(start + 1, end - start - 1);
                }
            }
            else if (line.find("\"debugging\"") != std::string::npos)
            {
                m_debugging = line.find("true") != std::string::npos;
            }
        }
        file.close();
    }
    else
    {
        createDefaultConfig();
    }
}

// !@!
// When new configuration items are created, they'll need to be added to this method
void ConfigManager::saveConfig()
{
    if (maxClips > 1000)
    {
        maxClips = 1000;
    }
    
    std::string configFile = configDir + "/config.json";
    std::cout << "DEBUG: Saving to " << configFile << "\n";
    std::cout << "DEBUG: maxClips before save = " << maxClips << "\n";
    
    std::ofstream outFile(configFile, std::ios::trunc);
    
    if (!outFile.is_open())
    {
        std::cout << "DEBUG: Failed to open file for writing\n";
        return;
    }
    
    std::cout << "DEBUG: File opened successfully\n";
    
    std::map<std::string, std::string> configValues;
    configValues["verbose"] = verboseMode ? "true" : "false";
    configValues["debugging"] = m_debugging ? "true" : "false";
    configValues["max_clips"] = std::to_string(maxClips);
    configValues["encrypted"] = encrypted ? "true" : "false";
    configValues["encryption_key"] = encryptionKey;
    configValues["autostart"] = autoStart ? "true" : "false";
    configValues["theme"] = theme;
    
    std::cout << "DEBUG: About to write max_clips = " << configValues["max_clips"] << "\n";
    
    outFile << "{\n";
    bool first = true;
    int writeCount = 0;
    for (const auto& pair : configValues)
    {
        if (!first)
        {
            outFile << ",\n";
        }
        first = false;
        
        if (pair.second == "true" || pair.second == "false")
        {
            outFile << "    \"" << pair.first << "\": " << pair.second;
        }
        else if (pair.second.find_first_not_of("0123456789") == std::string::npos)
        {
            outFile << "    \"" << pair.first << "\": " << pair.second;
        }
        else
        {
            outFile << "    \"" << pair.first << "\": \"" << pair.second << "\"";
        }
        writeCount++;
        std::cout << "DEBUG: Wrote config entry " << writeCount << ": " << pair.first << " = " << pair.second << "\n";
    }
    outFile << "\n}\n";
    outFile.flush();
    outFile.close();
    
    std::cout << "DEBUG: Save completed, wrote " << writeCount << " entries\n";
}

// !@!
// When new configuration items are created, they'll need to be added to this method
void ConfigManager::createDefaultConfig()
{
    std::string configFile = configDir + "/config.json";
    std::ofstream outFile(configFile);
    outFile << "{\n";
    outFile << "    \"debugging\": false,\n";
    outFile << "    \"verbose\": false,\n";
    outFile << "    \"max_clips\": 500,\n";
    outFile << "    \"encrypted\": true,\n";
    outFile << "    \"encryption_key\": \"mmry_default_key_2026\",\n";
    outFile << "    \"autostart\": false,\n";
    outFile << "    \"theme\": \"console\"\n";
    outFile << "}\n";
    outFile.close();
    
    std::cout << "Created default config at: " << configFile << "\n";
}

// !@!
// When new configuration items are created, they'll need to be added to this method
std::string ConfigManager::getConfigValue(const std::string& configKey)
{
    if (configKey == "verbose") return verboseMode ? "true" : "false";
    if (configKey == "debugging") return m_debugging ? "true" : "false";
    if (configKey == "max_clips") return std::to_string(maxClips);
    if (configKey == "encrypted") return encrypted ? "true" : "false";
    if (configKey == "encryption_key") return encryptionKey;
    if (configKey == "autostart") return autoStart ? "true" : "false";
    if (configKey == "theme") return theme;
    return "";
}

std::string ConfigManager::getConfigType(const std::string& configKey)
{
    if (std::find(booleanKeys.begin(), booleanKeys.end(), configKey) != booleanKeys.end())
        return "boolean (true/false)";
    if (std::find(numberKeys.begin(), numberKeys.end(), configKey) != numberKeys.end())
        return "number (positive integer)";
    if (std::find(stringKeys.begin(), stringKeys.end(), configKey) != stringKeys.end())
        return "string";

    return "unknown";
}

// !@!
// When new configuration items are created, they'll need to be added to this method
bool ConfigManager::updateConfigValue(const std::string& configKey, const std::string& newValue)
{
    try
    {
        std::string currentValue = getConfigValue(configKey);
        
        if (currentValue == "true" || currentValue == "false")
        {
            if (newValue == "true" || newValue == "false")
            {
                if (configKey == "verbose") verboseMode = newValue == "true";
                else if (configKey == "debugging") m_debugging = newValue == "true";
                else if (configKey == "encrypted") encrypted = newValue == "true";
                else if (configKey == "autostart") autoStart = newValue == "true";
                return true;
            }
            return false;
        }
        
        try
        {
            std::stoull(currentValue);
            size_t newNumValue = std::stoull(newValue);
            if (newNumValue > 0)
            {
                if (configKey == "max_clips")
                {
                    std::cout << "DEBUG: Updating maxClips from " << maxClips << " to " << newNumValue << "\n";
                    maxClips = newNumValue;
                    std::cout << "DEBUG: maxClips is now " << maxClips << "\n";
                }
                return true;
            }
            return false;
        }
        catch (...)
        {
            if (configKey == "encryption_key")
            {
                encryptionKey = newValue;
                return true;
            }
            else if (configKey == "theme")
            {
                theme = newValue;
                loadTheme();
                return true;
            }
            return true;
        }
    }
    catch (const std::exception& e)
    {
        return false;
    }
}
