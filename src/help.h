#ifndef HELP_H
#define HELP_H

#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
typedef void* HDC;
#endif

struct HelpTopic
{
    std::string key;
    std::string description;
    bool isHeader;
};

struct HelpDrawLine
{
    bool atTitleLeft;
    int leadGap;
    std::string text;
};

constexpr int HELP_LINE_HEIGHT = 15;
constexpr int HELP_GAP = 15;
constexpr int HELP_CONTENT_TOP_OFFSET = 20 + 25 + HELP_GAP;

extern std::vector<HelpTopic> helpTopicsCache;
extern std::string helpFilterText;
void buildHelpTopicsCache();
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

std::vector<HelpDrawLine> buildHelpDrawLines(int lineHeight, int gap);
int measureHelpContentHeight(int lineHeight, int gap);

#endif
