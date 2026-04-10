#include "pch.h"
#include "metadata_explorer.h"

typedef uint32_t(*mono_image_get_table_rows_t)(void* image, int table_id);
typedef void* (*mono_class_get_t)(void* image, uint32_t type_token);
typedef const char* (*mono_class_get_name_t)(void* klass);
typedef const char* (*mono_class_get_namespace_t)(void* klass);
typedef void* (*mono_signature_get_params_t)(void* signature, void** iter);
typedef void* (*mono_signature_get_return_type_t)(void* signature);
typedef char* (*mono_type_get_name_t)(void* type);
typedef void (*mono_free_t)(void* ptr);

static bool SearchMonoMethodsForExplorerRuntime(
    const std::string& needle,
    int maxResults,
    std::vector<ExplorerMethodHit>& outLines,
    std::string* errorMessage
) {
    outLines.clear();
    if (needle.empty() || maxResults <= 0) {
        if (errorMessage) *errorMessage = "Search query is empty.";
        return false;
    }

    MonoApi api{};
    if (!ResolveMonoApi(api)) {
        if (errorMessage) *errorMessage = "Mono API exports are unavailable.";
        return false;
    }
    if (!EnsureMonoThreadAttached(api)) {
        if (errorMessage) *errorMessage = "mono_thread_attach failed.";
        return false;
    }

    HMODULE mono = GetMonoModuleHandle();
    if (!mono) {
        if (errorMessage) *errorMessage = "mono module is not loaded.";
        return false;
    }

    mono_image_get_table_rows_t mono_image_get_table_rows =
        (mono_image_get_table_rows_t)GetProcAddress(mono, "mono_image_get_table_rows");
    mono_class_get_t mono_class_get =
        (mono_class_get_t)GetProcAddress(mono, "mono_class_get");
    mono_class_get_name_t mono_class_get_name =
        (mono_class_get_name_t)GetProcAddress(mono, "mono_class_get_name");
    mono_class_get_namespace_t mono_class_get_namespace =
        (mono_class_get_namespace_t)GetProcAddress(mono, "mono_class_get_namespace");
    mono_signature_get_params_t mono_signature_get_params =
        (mono_signature_get_params_t)GetProcAddress(mono, "mono_signature_get_params");
    mono_signature_get_return_type_t mono_signature_get_return_type =
        (mono_signature_get_return_type_t)GetProcAddress(mono, "mono_signature_get_return_type");
    mono_type_get_name_t mono_type_get_name =
        (mono_type_get_name_t)GetProcAddress(mono, "mono_type_get_name");
    mono_free_t mono_free =
        (mono_free_t)GetProcAddress(mono, "mono_free");

    if (!mono_image_get_table_rows || !mono_class_get || !mono_class_get_name || !mono_class_get_namespace ||
        !mono_signature_get_params || !mono_signature_get_return_type || !mono_type_get_name) {
        if (errorMessage) *errorMessage = "Required Mono Explorer exports are unavailable.";
        return false;
    }

    MonoAssemblyListCtx ctx;
    api.assembly_foreach(&MonoCollectAssembly, &ctx);
    if (ctx.assemblies.empty()) {
        if (errorMessage) *errorMessage = "No loaded mono assemblies found.";
        return false;
    }

    const int MONO_TABLE_TYPEDEF = 2;
    for (void* assembly : ctx.assemblies) {
        if ((int)outLines.size() >= maxResults) break;
        void* image = api.assembly_get_image(assembly);
        if (!image) continue;

        uint32_t typeRows = mono_image_get_table_rows(image, MONO_TABLE_TYPEDEF);
        for (uint32_t i = 1; i <= typeRows && (int)outLines.size() < maxResults; ++i) {
            uint32_t typeToken = 0x02000000u | i;
            void* klass = mono_class_get(image, typeToken);
            if (!klass) continue;

            const char* cn = mono_class_get_name(klass);
            const char* ns = mono_class_get_namespace(klass);
            if (!cn) cn = "";
            if (!ns) ns = "";
            std::string classFull = ns[0] ? (std::string(ns) + "." + std::string(cn)) : std::string(cn);

            void* iter = nullptr;
            while ((int)outLines.size() < maxResults) {
                void* method = api.class_get_methods(klass, &iter);
                if (!method) break;
                const char* mn = api.method_get_name(method);
                if (!mn || !AsciiSubstringCi(mn, needle.c_str())) continue;

                void* sig = api.method_signature(method);
                if (!sig) continue;

                std::string retType = "void";
                if (void* rt = mono_signature_get_return_type(sig)) {
                    char* rname = mono_type_get_name(rt);
                    if (rname && rname[0]) retType = rname;
                    if (rname && mono_free) mono_free(rname);
                }

                std::string params = "(";
                void* piter = nullptr;
                int pidx = 0;
                while (void* pt = mono_signature_get_params(sig, &piter)) {
                    if (pidx > 0) params += ", ";
                    char* pname = mono_type_get_name(pt);
                    if (pname && pname[0]) params += pname;
                    else params += "System.Object";
                    if (pname && mono_free) mono_free(pname);
                    params += " arg" + std::to_string(pidx);
                    ++pidx;
                }
                params += ")";

                ExplorerMethodHit hit;
                hit.returnType = std::move(retType);
                hit.qualifiedMethod = classFull + "::" + mn;
                hit.paramList = std::move(params);
                void* compiled = api.compile_method(method);
                if (compiled) {
                    char buf[64];
                    snprintf(buf, sizeof(buf), "ADDR=0x%p", compiled);
                    hit.rvaText = buf;
                } else {
                    hit.rvaText = "ADDR=?";
                }
                outLines.push_back(std::move(hit));
            }
        }
    }

    if (errorMessage) errorMessage->clear();
    return true;
}

bool RunMetadataMethodSearch(
    const std::string& needle,
    int maxResults,
    std::vector<ExplorerMethodHit>& outLines,
    std::string* errorMessage
) {
    GameRuntimeMode mode = DetectGameRuntimeMode();
    if (mode == GameRuntimeMode::Mono) {
        return SearchMonoMethodsForExplorerRuntime(needle, maxResults, outLines, errorMessage);
    }
    if (mode != GameRuntimeMode::Il2Cpp) {
        if (errorMessage) {
            *errorMessage = "Runtime is not detected yet. Wait for game initialization and try again.";
        }
        outLines.clear();
        return false;
    }

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
