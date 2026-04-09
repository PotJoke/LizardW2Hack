#include "pch.h"
#include "metadata_explorer.h"

bool RunMetadataMethodSearch(
    const std::string& needle,
    int maxResults,
    std::vector<ExplorerMethodHit>& outLines,
    std::string* errorMessage
) {
    auto fail = [&](const char* msg) -> bool {
        if (errorMessage) *errorMessage = msg;
        return false;
    };

    ResolvePathsRelativeToGameAssembly();

    if (g_globalMetadataPath.empty()) {
        return fail("global-metadata path is empty (GameAssembly.dll not loaded yet, or no *_Data folder with il2cpp_data/Metadata). Try search again after the game finishes loading.");
    }

    std::vector<uint8_t> metadata;
    if (!ReadAllBytes(g_globalMetadataPath, metadata)) {
        if (errorMessage) {
            *errorMessage = "Failed to open or read file: ";
            *errorMessage += g_globalMetadataPath;
        }
        return false;
    }

    if (metadata.size() < 0x80) {
        return fail("global-metadata.dat is empty or too small.");
    }

    std::vector<uint8_t> gameAssembly;
    if (!g_gameAssemblyPath.empty())
        ReadAllBytes(g_gameAssemblyPath, gameAssembly);

    if (!SearchMetadataMethodsForExplorer(metadata, gameAssembly, needle.c_str(), maxResults, outLines)) {
        return fail("Metadata parse failed (bad sanity/version or size mismatch). File may be encrypted or from an unsupported Unity layout.");
    }

    if (errorMessage) errorMessage->clear();
    return true;
}
