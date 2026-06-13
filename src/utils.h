#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include "ui.h"

class ConfigManager;

bool isUrl(const std::string& text);
bool isFilePath(const std::string& text);
bool isPath(const std::string& text);

std::string smartTrim(const std::string& text, size_t maxLength);
std::string trimMiddle(const std::string& text, size_t maxLength);

std::string wildcardToRegex(const std::string& pattern);
bool isRegexPatternSafe(const std::string& pattern);
bool extractLookaheadTerms(const std::string& pattern,
                           std::vector<std::string>& required,
                           std::vector<std::string>& forbidden);

int countLines(const std::string& content);

int calculateDialogContentLength(const DialogDimensions& dims);
int calculateMaxContentLength(int clipListWidth, bool verboseMode);
DialogDimensions calculateDialogDimensions(int windowWidth, int windowHeight, int preferredWidth, int preferredHeight);

std::string encrypt(const std::string& data, const ConfigManager& config);
std::string decrypt(const std::string& data, const ConfigManager& config);

std::vector<std::string> getSortedPinnedItems(const std::string& pinnedFile);

#endif
