#pragma once

#include <string>
#include <vector>

bool RunMetadataMethodSearch(
    const std::string& needle,
    int maxResults,
    std::vector<std::string>& outLines,
    std::string* errorMessage = nullptr);
