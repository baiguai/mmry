#include "utils.h"
#include "config.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <fstream>
#include <regex>
#include <sstream>

// ============================================================
// PathDetector
// ============================================================
namespace
{
    class PathDetector
    {
    public:
        static bool isFilePath(const std::string& text)
        {
            if (text.empty() || text.length() > 4096) return false;

            std::string trimmed = trim(text);
            if (trimmed.empty()) return false;

            if (isAbsolutePath(trimmed)) return true;
            if (hasPathSeparators(trimmed)) return true;
            if (hasValidFileExtension(trimmed)) return true;
            if (matchesPathPattern(trimmed)) return true;

            return false;
        }

    private:
        static std::string trim(const std::string& str)
        {
            size_t start = str.find_first_not_of(" \t\n\r");
            if (start == std::string::npos) return "";
            size_t end = str.find_last_not_of(" \t\n\r");
            return str.substr(start, end - start + 1);
        }

        static bool isAbsolutePath(const std::string& text)
        {
            if (text[0] == '/') return true;

            if (text.length() >= 3 &&
                std::isalpha(text[0]) &&
                text[1] == ':' &&
                (text[2] == '\\' || text[2] == '/'))
            {
                return true;
            }

            if (text.length() >= 2 && text[0] == '\\' && text[1] == '\\')
            {
                return true;
            }

            return false;
        }

        static bool hasPathSeparators(const std::string& text)
        {
            size_t slashCount = std::count(text.begin(), text.end(), '/');
            size_t backslashCount = std::count(text.begin(), text.end(), '\\');

            if (backslashCount > 0) return true;

            if (slashCount > 0)
            {
                if (text.find("./") == 0 || text.find("../") == 0) return true;
                if (slashCount >= 1 && text.find(' ') == std::string::npos) return true;
            }

            return false;
        }

        static bool hasValidFileExtension(const std::string& text)
        {
            size_t lastDot = text.find_last_of('.');
            size_t lastSlash = text.find_last_of("/\\");

            if (lastDot == std::string::npos ||
                lastDot == 0 ||
                lastDot == text.length() - 1)
            {
                return false;
            }

            if (lastSlash != std::string::npos && lastDot < lastSlash)
            {
                return false;
            }

            std::string ext = text.substr(lastDot + 1);

            if (ext.length() < 1 || ext.length() > 10) return false;

            bool validExt = std::all_of(ext.begin(), ext.end(),
                [](char c) { return std::isalnum(c); });

            if (!validExt) return false;

            std::string basename = text.substr(0, lastDot);
            if (lastSlash != std::string::npos)
            {
                basename = basename.substr(lastSlash + 1);
            }

            if (basename.empty() || basename.find(' ') != std::string::npos)
            {
                return false;
            }

            std::string extLower = ext;
            std::transform(extLower.begin(), extLower.end(), extLower.begin(), ::tolower);

            static const std::vector<std::string> commonExts =
            {
                "c", "cpp", "cc", "cxx", "h", "hpp", "hxx", "cs", "java", "py", "rb", "go",
                "js", "ts", "jsx", "tsx", "php", "swift", "kt", "rs", "scala", "r", "m", "mm",
                "html", "htm", "css", "scss", "sass", "less", "xml", "json", "yaml", "yml",
                "txt", "md", "doc", "docx", "pdf", "rtf", "odt", "tex", "log",
                "xls", "xlsx", "csv", "ods",
                "ppt", "pptx", "odp",
                "jpg", "jpeg", "png", "gif", "bmp", "svg", "ico", "webp", "tiff", "tif",
                "mp3", "wav", "ogg", "flac", "aac", "m4a", "wma",
                "mp4", "avi", "mkv", "mov", "wmv", "flv", "webm",
                "zip", "rar", "7z", "tar", "gz", "bz2", "xz", "tgz",
                "exe", "dll", "so", "dylib", "app", "dmg", "deb", "rpm", "msi",
                "ini", "cfg", "conf", "config", "properties", "toml",
                "sh", "bash", "bat", "cmd", "ps1", "vbs",
                "db", "sqlite", "sql", "dat", "bin"
            };

            return std::find(commonExts.begin(), commonExts.end(), extLower) != commonExts.end();
        }

        static bool matchesPathPattern(const std::string& text)
        {
            static const std::regex patterns[] =
            {
                std::regex(R"(^/([a-zA-Z0-9_\-\.]+/?)+$)"),
                std::regex(R"(^\.{1,2}/([a-zA-Z0-9_\-\.]+/?)+$)"),
                std::regex(R"(^[a-zA-Z]:[/\\]([a-zA-Z0-9_\-\. ]+[/\\]?)+$)"),
                std::regex(R"(^\\\\[a-zA-Z0-9_\-\.]+\\([a-zA-Z0-9_\-\. ]+\\?)+$)"),
                std::regex(R"(^[a-zA-Z0-9_\-\.]+(/[a-zA-Z0-9_\-\.]+)+(\.[a-zA-Z0-9]{1,10})?$)"),
                std::regex(R"(^~(/[a-zA-Z0-9_\-\.]+)+$)")
            };

            for (const auto& pattern : patterns)
            {
                if (std::regex_match(text, pattern))
                {
                    return true;
                }
            }

            return false;
        }
    };

    // ============================================================
    // StringTrimmer
    // ============================================================

    class StringTrimmer
    {
    public:
        static std::string trimMiddle(const std::string& text, size_t maxLength)
        {
            if (text.length() <= maxLength)
            {
                return text;
            }

            if (maxLength < 4)
            {
                return text.substr(0, maxLength);
            }

            if (maxLength <= 10)
            {
                return text.substr(0, maxLength - 3) + "...";
            }

            const std::string ellipsis = "...";
            const size_t ellipsisLen = ellipsis.length();

            if (isUrl(text))
            {
                return trimUrlMiddle(text, maxLength, ellipsisLen);
            }
            else if (isFilePath(text))
            {
                return trimPathMiddle(text, maxLength, ellipsisLen);
            }

            return trimBalanced(text, maxLength, ellipsisLen);
        }

    private:
        static std::string trimUrlMiddle(const std::string& url, size_t maxLength, size_t ellipsisLen)
        {
            size_t protocolEnd = url.find("://");

            if (protocolEnd == std::string::npos)
            {
                return trimBalanced(url, maxLength, ellipsisLen);
            }

            protocolEnd += 3;

            if (protocolEnd >= url.length())
            {
                return trimBalanced(url, maxLength, ellipsisLen);
            }

            size_t domainEnd = url.find('/', protocolEnd);
            size_t queryStart = url.find('?');
            size_t fragmentStart = url.find('#');

            size_t lastSlash = url.find_last_of('/');

            std::string protocol = url.substr(0, protocolEnd);
            std::string domain = (domainEnd != std::string::npos)
                ? url.substr(protocolEnd, domainEnd - protocolEnd)
                : url.substr(protocolEnd);

            std::string lastSegment;
            if (lastSlash != std::string::npos && lastSlash + 1 < url.length())
            {
                size_t segmentEnd = std::min({
                    queryStart != std::string::npos ? queryStart : url.length(),
                    fragmentStart != std::string::npos ? fragmentStart : url.length(),
                    url.length()
                });

                if (segmentEnd > lastSlash)
                {
                    lastSegment = url.substr(lastSlash, segmentEnd - lastSlash);
                }
            }

            size_t prefixLen = protocol.length() + domain.length();
            size_t suffixLen = lastSegment.length();

            if (prefixLen + ellipsisLen + suffixLen <= maxLength &&
                !lastSegment.empty() &&
                domainEnd != std::string::npos &&
                lastSlash != std::string::npos &&
                lastSlash > domainEnd)
            {
                return protocol + domain + "..." + lastSegment;
            }

            if (prefixLen + ellipsisLen <= maxLength && domainEnd != std::string::npos)
            {
                size_t remainingSpace = maxLength - prefixLen - ellipsisLen;
                if (remainingSpace > 0 && remainingSpace <= url.length())
                {
                    std::string suffix = url.substr(url.length() - remainingSpace);
                    return protocol + domain + "..." + suffix;
                }
                return protocol + domain + "...";
            }

            if (protocol.length() + ellipsisLen < maxLength)
            {
                size_t domainSpace = maxLength - protocol.length() - ellipsisLen;

                if (domainSpace < domain.length() && domainSpace > 0)
                {
                    return protocol + domain.substr(0, domainSpace) + "...";
                }
            }

            return trimBalanced(url, maxLength, ellipsisLen);
        }

        static std::string trimPathMiddle(const std::string& path, size_t maxLength, size_t ellipsisLen)
        {
            std::string prefix;
            size_t prefixEnd = 0;

            if (path[0] == '/')
            {
                prefix = "/";
                prefixEnd = 1;
            }
            else if (path.length() >= 2 && path[1] == ':')
            {
                prefix = path.substr(0, std::min(size_t(3), path.length()));
                prefixEnd = prefix.length();
            }
            else if (path.length() >= 2 && path[0] == '\\' && path[1] == '\\')
            {
                size_t nextSep = path.find_first_of("\\/", 2);
                if (nextSep != std::string::npos)
                {
                    prefix = path.substr(0, nextSep);
                    prefixEnd = nextSep;
                }
                else
                {
                    prefix = path.substr(0, std::min(size_t(20), path.length()));
                    prefixEnd = prefix.length();
                }
            }
            else if (path.find("./") == 0 || path.find("../") == 0)
            {
                size_t firstSep = path.find_first_of("/\\");
                if (firstSep != std::string::npos)
                {
                    prefix = path.substr(0, firstSep + 1);
                    prefixEnd = firstSep + 1;
                }
                else
                {
                    prefix = path;
                    prefixEnd = path.length();
                }
            }
            else if (path.find("~/") == 0)
            {
                prefix = "~/";
                prefixEnd = 2;
            }

            std::vector<size_t> separators;
            for (size_t i = prefixEnd; i < path.length(); ++i)
            {
                if (path[i] == '/' || path[i] == '\\')
                {
                    separators.push_back(i);
                }
            }

            if (separators.empty())
            {
                if (path.length() <= maxLength)
                {
                    return path;
                }
                return trimBalanced(path, maxLength, ellipsisLen);
            }

            size_t firstComponentsEnd = separators[0];
            if (separators.size() > 1)
            {
                firstComponentsEnd = separators[1];
            }
            std::string firstPart = path.substr(0, firstComponentsEnd);

            size_t lastComponentsStart = separators[separators.size() - 1];
            if (separators.size() >= 2)
            {
                lastComponentsStart = separators[separators.size() - 2];
            }
            if (separators.size() >= 3)
            {
                lastComponentsStart = separators[separators.size() - 3];
            }
            std::string lastPart = path.substr(lastComponentsStart);

            if (firstPart.length() + ellipsisLen + lastPart.length() <= maxLength)
            {
                return firstPart + "..." + lastPart;
            }

            if (separators.size() >= 2)
            {
                firstComponentsEnd = separators[0];
                firstPart = path.substr(0, firstComponentsEnd);

                lastComponentsStart = separators[separators.size() - 2];
                lastPart = path.substr(lastComponentsStart);

                if (firstPart.length() + ellipsisLen + lastPart.length() <= maxLength)
                {
                    return firstPart + "..." + lastPart;
                }
            }

            size_t suffixStart = separators[separators.size() - 1];
            if (separators.size() >= 2)
            {
                suffixStart = separators[separators.size() - 2];
            }
            if (separators.size() >= 3)
            {
                suffixStart = separators[separators.size() - 3];
            }
            std::string suffix = path.substr(suffixStart);

            if (prefix.length() + ellipsisLen + suffix.length() <= maxLength)
            {
                return prefix + "..." + suffix;
            }

            suffix = path.substr(separators[separators.size() - 1]);
            if (prefix.length() + ellipsisLen + suffix.length() <= maxLength)
            {
                return prefix + "..." + suffix;
            }

            size_t remainingSpace = maxLength - prefix.length() - ellipsisLen;
            if (remainingSpace > 0 && remainingSpace <= suffix.length())
            {
                return prefix + "..." + suffix.substr(suffix.length() - remainingSpace);
            }

            return trimBalanced(path, maxLength, ellipsisLen);
        }

        static std::string trimBalanced(const std::string& text, size_t maxLength, size_t ellipsisLen)
        {
            size_t availableSpace = maxLength - ellipsisLen;
            size_t keepStart = availableSpace / 2;
            size_t keepEnd = availableSpace - keepStart;

            return text.substr(0, keepStart) + "..." + text.substr(text.length() - keepEnd);
        }
    };
} // anonymous namespace

// ============================================================
// Public API
// ============================================================

bool isUrl(const std::string& text)
{
    return (text.substr(0, 7) == "http://" ||
            text.substr(0, 8) == "https://" ||
            text.substr(0, 6) == "ftp://" ||
            text.substr(0, 7) == "sftp://" ||
            text.substr(0, 4) == "www.");
}

bool isFilePath(const std::string& text)
{
    return PathDetector::isFilePath(text);
}

bool isPath(const std::string& text)
{
    return isUrl(text) || isFilePath(text);
}

std::string smartTrim(const std::string& text, size_t maxLength)
{
    if (isPath(text))
    {
        return trimMiddle(text, maxLength);
    }
    return text.substr(0, maxLength - 3) + "...";
}

std::string trimMiddle(const std::string& text, size_t maxLength)
{
    return StringTrimmer::trimMiddle(text, maxLength);
}

std::string wildcardToRegex(const std::string& pattern)
{
    std::string regex_pattern;
    regex_pattern.reserve(pattern.size() * 2);
    for (char c : pattern)
    {
        switch (c)
        {
            case '*':
                regex_pattern += ".*";
                break;
            case '.': case '+': case '?': case '^': case '$': case '(': case ')':
            case '[': case ']': case '{': case '}': case '|': case '\\':
                regex_pattern += '\\';
                regex_pattern += c;
                break;
            default:
                regex_pattern += c;
        }
    }
    return regex_pattern;
}

bool isRegexPatternSafe(const std::string& pattern)
{
    if (pattern.empty()) return true;

    for (size_t i = 0; i + 1 < pattern.size(); ++i)
    {
        if (pattern[i] == '(' && pattern[i + 1] == '?')
        {
            if (i + 2 >= pattern.size()) return false;
            // Only allow (?: non-capturing groups.
            // Reject lookaheads (?=, (?!), lookbehinds (?<=, (?<!),
            // atomic groups (?>), recursion (?R, (?0, etc.), conditionals,
            // and any other (? extension, as libstdc++'s std::regex
            // can crash (segfault) on these constructs.
            if (pattern[i + 2] != ':')
                return false;
        }
    }
    return true;
}

int countLines(const std::string& content)
{
    if (content.empty()) return 0;
    int lines = 1;
    for (char c : content)
    {
        if (c == '\n') lines++;
    }
    return lines;
}

int calculateDialogContentLength(const DialogDimensions& dims)
{
    int availableWidth = dims.contentWidth;
    int maxChars = availableWidth / 8;
    if (maxChars < 20) maxChars = 20;
    if (maxChars > 100) maxChars = 100;
    return maxChars;
}

int calculateMaxContentLength(int clipListWidth, bool verboseMode)
{
    int availableWidth = clipListWidth - 30;

    if (verboseMode) {
        availableWidth -= 20;
    }

    int maxChars = availableWidth / 8;

    if (maxChars < 20) maxChars = 20;
    if (maxChars > 200) maxChars = 200;

    return maxChars;
}

DialogDimensions calculateDialogDimensions(int windowWidth, int windowHeight, int preferredWidth, int preferredHeight)
{
    DialogDimensions dims;

    int maxHeight = windowHeight - 40;
    int maxWidth = windowWidth - 40;

    int minDialogWidth = 200;
    int minDialogHeight = 100;

    dims.height = std::min(preferredHeight, std::max(minDialogHeight, maxHeight));
    dims.width = std::min(preferredWidth, std::max(minDialogWidth, maxWidth));

    dims.contentHeight = dims.height - 80;
    dims.contentWidth = dims.width - 40;

    if (dims.contentWidth < 200) dims.contentWidth = 200;

    dims.x = (windowWidth - dims.width) / 2;
    dims.y = (windowHeight - dims.height) / 2;

    return dims;
}

std::string encrypt(const std::string& data, const ConfigManager& config)
{
    if (!config.encrypted || config.encryptionKey.empty())
    {
        return data;
    }

    std::string encrypted;
    encrypted.resize(data.length());

    for (size_t i = 0; i < data.length(); ++i)
    {
        encrypted[i] = data[i] ^ config.encryptionKey[i % config.encryptionKey.length()];
    }

    const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string result;
    int val = 0, valb = -6;

    for (char c : encrypted)
    {
        val = (val << 8) + (c & 0xff);
        valb += 8;
        while (valb >= 0)
        {
            result.push_back(chars[(val >> valb) & 0x3f]);
            valb -= 6;
        }
    }

    if (valb > -6)
    {
        result.push_back(chars[((val << 8) >> (valb + 8)) & 0x3f]);
    }

    while (result.length() % 4)
    {
        result.push_back('=');
    }

    return result;
}

std::string decrypt(const std::string& data, const ConfigManager& config)
{
    if (!config.encrypted || config.encryptionKey.empty())
    {
        return data;
    }

    const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string decoded;
    int val = 0, valb = -8;

    for (char c : data)
    {
        if (c == '=') break;

        size_t pos = chars.find(c);
        if (pos == std::string::npos) continue;

        val = (val << 6) + pos;
        valb += 6;

        if (valb >= 0)
        {
            decoded.push_back((val >> valb) & 0xff);
            valb -= 8;
        }
    }

    std::string decrypted;
    decrypted.resize(decoded.length());

    for (size_t i = 0; i < decoded.length(); ++i)
    {
        decrypted[i] = decoded[i] ^ config.encryptionKey[i % config.encryptionKey.length()];
    }

    return decrypted;
}

std::vector<std::string> getSortedPinnedItems(const std::string& pinnedFile)
{
    std::vector<std::pair<long long, std::string>> pinnedItems;
    std::string line;
    std::ifstream inFile(pinnedFile);

    while (std::getline(inFile, line))
    {
        size_t pos = line.find('|');
        if (pos != std::string::npos && pos > 0)
        {
            try
            {
                long long timestamp = std::stoll(line.substr(0, pos));
                pinnedItems.push_back({timestamp, line});
            }
            catch (...)
            {
                long long timestamp = std::chrono::system_clock::now().time_since_epoch().count();
                pinnedItems.push_back({timestamp, line});
            }
        }
    }
    inFile.close();

    std::sort(pinnedItems.begin(), pinnedItems.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    std::vector<std::string> sortedLines;
    for (const auto& item : pinnedItems)
    {
        sortedLines.push_back(item.second);
    }

    return sortedLines;
}
