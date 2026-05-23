#ifndef HELP_H
#define HELP_H

#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
typedef void* HDC;
#endif

struct HelpTopic {
    std::string key;
    std::string description;
    bool isHeader;
};
extern std::vector<HelpTopic> helpTopicsCache;
extern std::string helpFilterText;
void buildHelpTopicsCache();                      // defined in main.cpp
std::string stringToLower(const std::string& str); // declared in main.h too


#ifdef __linux__
void drawHelpTopic(HDC /*hdc*/, int x, int y, int contentTop, int contentBottom, const std::string& topic);
#endif

#ifdef _WIN32
void drawHelpTopic(HDC hdc, int x, int y, int contentTop, int contentBottom, const std::string& topic);
#endif

#ifdef __APPLE__
    // (If you eventually implement macOS)
#endif

void drawAllHelpTopics(HDC hdc, int titleLeft, int topicLeft, int lineHeight, int gap, int y, int contentTop, int contentBottom);

void buildHelpTopicsCache();

#endif
