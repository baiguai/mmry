#include "help.h"

#ifdef __linux__
#include <X11/Xlib.h>
extern Display* display;
extern Window window;
extern GC gc;
#endif


std::vector<HelpTopic> helpTopicsCache;
std::string helpFilterText;


#ifdef __linux__
void drawHelpTopic(HDC /*hdc*/, int x, int y, int contentTop, int contentBottom, const std::string& topic)
{
    if (y >= contentTop && y < contentBottom)
    {
        XDrawString(display, window, gc, x, y, topic.c_str(), topic.length());
    }
}
#endif

#ifdef _WIN32
void drawHelpTopic(HDC hdc, int x, int y, int contentTop, int contentBottom, const std::string& topic)
{
    if (hdc && y >= contentTop && y < contentBottom)
    {
        RECT rc;
        rc.left   = x;
        rc.top    = y - 12;     // baseline to bounding box adjustment
        rc.right  = x + 2000;   // wide; parent clipping restricts it
        rc.bottom = y + 4;

        DrawTextA(
            hdc,
            topic.c_str(),
            (int)topic.length(),
            &rc,
            DT_LEFT | DT_NOPREFIX | DT_SINGLELINE
        );
    }
}
#endif

#ifdef __APPLE__
        // (If we eventually implement macOS)
#endif

void drawAllHelpTopics(HDC hdc, int titleLeft, int topicLeft, int lineHeight, int gap, int y, int contentTop, int contentBottom)
{
    if (helpTopicsCache.empty())
    {
        buildHelpTopicsCache();
    }

    std::string filterQuery = helpFilterText;

    // Show REGEX examples when filter starts with '!'
    if (!filterQuery.empty() && filterQuery[0] == '!')
    {
        const std::vector<std::pair<std::string, std::string>> regexExamples = {
            {"REGEX Examples (type after /! in main window):", ""},
            {R"(!hello)", "Clips containing \"hello\""},
            {R"(!^hello)", "Clips starting with \"hello\""},
            {R"(!hello$)", "Clips ending with \"hello\""},
            {R"(!^hello$)", "Clips exactly equal to \"hello\""},
            {R"(![Hh]ello)", "Clips with \"Hello\" or \"hello\""},
            {R"(!\bhello\b)", "Clips where \"hello\" is a whole word"},
            {R"(!hello.*world)", "Clips with \"hello\" then \"world\""},
            {R"(!hello|world)", "Clips with \"hello\" or \"world\""},
            {R"(!(?=.*hello)(?=.*world))", "Clips with both \"hello\" and \"world\""},
            {R"(!^(?=.*hello)(?=.*world).*$)", "Clips with both (any order)"},
            {R"(!^.{100,}$)", "Clips with 100+ characters"},
            {R"(!^.{0,10}$)", "Clips with 10 or fewer characters"},
            {R"(!\d+)", "Clips containing digits"},
            {R"(!\d{3}-\d{3}-\d{4})", "Clips matching phone number pattern"},
            {R"(!\w+@\w+\.\w+)", "Clips with email-like patterns"},
            {R"(!http\S+)", "Clips containing URLs"},
            {R"(!\s+)", "Clips with whitespace"},
            {R"(!^[ \t]*$)", "Clips that are blank/whitespace-only"},
            {R"(!^\- )", "Clips starting with \"- \" (list items)"},
        };

        for (size_t i = 0; i < regexExamples.size(); ++i)
        {
            const auto& [text, desc] = regexExamples[i];
            if (i == 0)
            {
                drawHelpTopic(hdc, titleLeft, y, contentTop, contentBottom, text);
                y += lineHeight;
                y += gap;
            }
            else
            {
                drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, text + "  -  " + desc);
                y += lineHeight;
            }
        }
        return;
    }

    bool keysOnly = false;
    if (!filterQuery.empty() && filterQuery[0] == ':')
    {
        keysOnly = true;
        filterQuery = filterQuery.substr(1);
    }
    std::string lowerQuery = stringToLower(filterQuery);

    std::vector<HelpTopic> filtered;
    for (const auto& topic : helpTopicsCache)
    {
        if (topic.isHeader)
        {
            filtered.push_back(topic);
        }
        else
        {
            std::string keyLower = stringToLower(topic.key);
            std::string descLower = stringToLower(topic.description);
            if (keysOnly)
            {
                if (keyLower.find(lowerQuery) != std::string::npos)
                {
                    filtered.push_back(topic);
                }
            }
            else
            {
                if (keyLower.find(lowerQuery) != std::string::npos || descLower.find(lowerQuery) != std::string::npos)
                {
                    filtered.push_back(topic);
                }
            }
        }
    }

    for (const auto& topic : filtered)
    {
        std::string displayText;
        if (topic.isHeader)
        {
            displayText = topic.key;
            drawHelpTopic(hdc, titleLeft, y, contentTop, contentBottom, displayText);
        }
        else
        {
            displayText = topic.key + "  -  " + topic.description;
            drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, displayText);
        }
        y += lineHeight;
        if (topic.isHeader)
        {
            y += gap;
        }
    }

    y += lineHeight;
    drawHelpTopic(hdc, titleLeft, y, contentTop, contentBottom, "Global Hotkey:");
    y += lineHeight;
    drawHelpTopic(hdc, topicLeft, y, contentTop, contentBottom, "Ctrl+Alt+C     - Show/hide window");
}

void buildHelpTopicsCache()
{
    helpTopicsCache.clear();

    helpTopicsCache.push_back({"Main Window:", "", true});
    helpTopicsCache.push_back({"j/k", "Navigate items", false});
    helpTopicsCache.push_back({"g/G", "Top/bottom", false});
    helpTopicsCache.push_back({"/", "Filter mode (! for REGEX)", false});
    helpTopicsCache.push_back({"Shift+m", "Manage bookmark groups", false});
    helpTopicsCache.push_back({"m", "Add clip to group", false});
    helpTopicsCache.push_back({"`", "View bookmarks", false});
    helpTopicsCache.push_back({"p", "Pin clip", false});
    helpTopicsCache.push_back({"'", "View pinned clips", false});
    helpTopicsCache.push_back({"i", "Edit current clip", false});
    helpTopicsCache.push_back({"?", "This help", false});
    helpTopicsCache.push_back({"Shift+d", "Delete item", false});
    helpTopicsCache.push_back({"Shift+q", "Quit", false});
    helpTopicsCache.push_back({"Enter", "Copy item", false});
    helpTopicsCache.push_back({"Escape", "Hide window", false});

    helpTopicsCache.push_back({"Filter Mode:", "", true});
    helpTopicsCache.push_back({"Type text", "Filter items", false});
    helpTopicsCache.push_back({"Backspace", "Delete char", false});
    helpTopicsCache.push_back({"Up/down arrow", "Navigate items", false});
    helpTopicsCache.push_back({"Delete", "Delete item", false});
    helpTopicsCache.push_back({"Enter", "Copy item", false});
    helpTopicsCache.push_back({"Escape", "Exit filter", false});

    helpTopicsCache.push_back({"Pinned Clips:", "", true});
    helpTopicsCache.push_back({"j/k", "Navigate items", false});
    helpTopicsCache.push_back({"g/G", "Top/bottom", false});
    helpTopicsCache.push_back({"Shift+d", "Delete item", false});
    helpTopicsCache.push_back({"Enter", "Copy item", false});
    helpTopicsCache.push_back({"Escape", "Exit pinned clips", false});

    helpTopicsCache.push_back({"Add Bookmark Group Dialog:", "", true});
    helpTopicsCache.push_back({"Type text", "Define Group Name / Filter Existing", false});
    helpTopicsCache.push_back({"Backspace", "Delete char", false});
    helpTopicsCache.push_back({"Enter", "Create group", false});
    helpTopicsCache.push_back({"Escape", "Exit dialog", false});

    helpTopicsCache.push_back({"Add Clip to Group Dialog:", "", true});
    helpTopicsCache.push_back({"j/k", "Navigate group", false});
    helpTopicsCache.push_back({"g/G", "Top/bottom", false});
    helpTopicsCache.push_back({"/", "Begin filtering groups", false});
    helpTopicsCache.push_back({"Enter", "Add clip to group", false});
    helpTopicsCache.push_back({"Escape", "Exit filtering / Exit dialog", false});

    helpTopicsCache.push_back({"View/Delete/Use Bookmarks Dialog", "", true});
    helpTopicsCache.push_back({"j/k", "Navigate groups/clips", false});
    helpTopicsCache.push_back({"g/G", "Top/bottom", false});
    helpTopicsCache.push_back({"h", "Back to groups list", false});
    helpTopicsCache.push_back({"Shift+d", "Delete item", false});
    helpTopicsCache.push_back({"/", "Begin filtering groups", false});
    helpTopicsCache.push_back({"Enter", "View group clips/copy clip", false});
    helpTopicsCache.push_back({"Escape", "Exit filtering / Exit dialog", false});

    helpTopicsCache.push_back({"Commands:", "", true});
    helpTopicsCache.push_back({":", "Activate commands", false});
    helpTopicsCache.push_back({"theme", "Select theme to apply", false});
    helpTopicsCache.push_back({"config", "Select config option or modify with: config key value", false});
    helpTopicsCache.push_back({"Enter", "Select config or apply change", false});
    helpTopicsCache.push_back({"Example: config max_clips 1000", "", false});
    helpTopicsCache.push_back({"Escape", "Cancel command", false});

    helpTopicsCache.push_back({"Help Window:", "", true});
    helpTopicsCache.push_back({"/", "Filter help topics (prefix ':' to search keys only)", false});
    helpTopicsCache.push_back({"j/k or arrows", "Scroll down/up", false});
    helpTopicsCache.push_back({"g/G", "Go to top/bottom", false});
    helpTopicsCache.push_back({"Escape", "Close help", false});
}
