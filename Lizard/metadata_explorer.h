#pragma once

#include "il2cpp_resolver.h"

#include <string>
#include <vector>

bool RunMetadataMethodSearch(
    const std::string& needle,
    int maxResults,
    std::vector<ExplorerMethodHit>& outLines,
    std::string* errorMessage = nullptr);
