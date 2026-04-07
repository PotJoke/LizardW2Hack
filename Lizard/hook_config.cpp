#include "pch.h"
#include "hook_config.h"

#include <cstdio>
#include <cstdlib>
#include <regex>

static void StripLineComments(std::string& s) {
    size_t i = 0;
    while (i < s.size()) {
        if (i + 1 < s.size() && s[i] == '/' && s[i + 1] == '/') {
            size_t end = s.find('\n', i);
            if (end == std::string::npos) {
                s.erase(i);
                break;
            }
            s.erase(i, end - i);
            continue;
        }
        ++i;
    }
}

static void SkipUtf8Bom(std::string& s) {
    if (s.size() >= 3 && (unsigned char)s[0] == 0xEF && (unsigned char)s[1] == 0xBB && (unsigned char)s[2] == 0xBF) {
        s.erase(0, 3);
    }
}

static bool ReadWholeFile(const std::wstring& path, std::string& out) {
    FILE* f = nullptr;
    if (_wfopen_s(&f, path.c_str(), L"rb") != 0 || !f) return false;
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return false;
    }
    long sz = ftell(f);
    if (sz < 0) {
        fclose(f);
        return false;
    }
    if (fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return false;
    }
    out.resize((size_t)sz);
    size_t rd = fread(out.data(), 1, (size_t)sz, f);
    fclose(f);
    if (rd != (size_t)sz) return false;
    return true;
}

static std::string UnescapeCString(const std::string& in) {
    std::string o;
    o.reserve(in.size());
    for (size_t i = 0; i < in.size(); ++i) {
        if (in[i] != '\\' || i + 1 >= in.size()) {
            o += in[i];
            continue;
        }
        char c = in[++i];
        switch (c) {
        case 'n': o += '\n'; break;
        case 'r': o += '\r'; break;
        case 't': o += '\t'; break;
        case '\\': o += '\\'; break;
        case '"': o += '"'; break;
        default: o += '\\'; o += c; break;
        }
    }
    return o;
}

static int CountDWORDPointers(const std::string& p) {
    int n = 0;
    for (size_t pos = 0; pos < p.size();) {
        pos = p.find("DWORD", pos);
        if (pos == std::string::npos) break;
        size_t after = pos + 5;
        while (after < p.size() && (p[after] == ' ' || p[after] == '\t')) after++;
        if (after < p.size() && p[after] == '*') n++;
        pos += 5;
    }
    return n;
}

static int DefaultParamCountFromSignature(HookSignature sig) {
    switch (sig) {
    case HookSignature::BoolMethod: return 0;
    case HookSignature::BoolThisMethod: return 0;
    case HookSignature::BoolThisScenes: return 1;
    case HookSignature::BoolThis4: return 2;
    case HookSignature::BoolThis5: return 3;
    default: return 0;
    }
}

HookSignature InferSignatureFromParamList(const std::string& params_inside_parens) {
    int count = CountDWORDPointers(params_inside_parens);
    switch (count) {
    case 1: return HookSignature::BoolMethod;
    case 2: return HookSignature::BoolThisMethod;
    case 3: return HookSignature::BoolThisScenes;
    case 4: return HookSignature::BoolThis4;
    case 5: return HookSignature::BoolThis5;
    default: return HookSignature::BoolThisMethod;
    }
}

void SplitIl2CppFullName(const std::string& full_name, std::string& namespaze, std::string& class_name, std::string& method_name) {
    namespaze.clear();
    class_name.clear();
    method_name.clear();
    const size_t p = full_name.rfind("$$");
    if (p == std::string::npos) {
        class_name = full_name;
        return;
    }
    std::string left = full_name.substr(0, p);
    method_name = full_name.substr(p + 2);
    const size_t dot = left.rfind('.');
    if (dot == std::string::npos) {
        class_name = left;
    } else {
        namespaze = left.substr(0, dot);
        class_name = left.substr(dot + 1);
    }
}

static void ParseHookBody(const std::string& body, HookDefinition& def) {
    def.param_count = DefaultParamCountFromSignature(def.signature);
    def.enabled_on_start = true;
    def.return_bool = false;
    def.log_line.clear();
    def.display_name = def.full_name;

    std::regex paramRe(R"(\bparam(?:_count)?\s+(\d+)\s*[;])");
    std::smatch pm;
    if (std::regex_search(body, pm, paramRe)) {
        def.param_count = (int)strtol(pm[1].str().c_str(), nullptr, 10);
    }

    std::regex retRe(R"(\breturn\s+(true|false)\s*;)");
    std::smatch rm;
    if (std::regex_search(body, rm, retRe)) {
        def.return_bool = (rm[1] == "true");
    }

    std::regex printfRe(R"rx(printf\s*\(\s*"((?:\\.|[^"\\])*)"\s*\))rx");
    std::smatch fm;
    if (std::regex_search(body, fm, printfRe)) {
        def.log_line = UnescapeCString(fm[1]);
    }

    std::regex enabledRe(R"(\b(enabled|enabled_on_start|start_enabled)\s+(true|false)\s*[;])");
    std::smatch em;
    if (std::regex_search(body, em, enabledRe)) {
        def.enabled_on_start = (em[2] == "true");
    }

    std::regex nameRe(R"rx(\b(name|title)\s*"((?:\\.|[^"\\])*)"\s*[;])rx");
    std::smatch nm;
    if (std::regex_search(body, nm, nameRe)) {
        def.display_name = UnescapeCString(nm[2]);
    }
}

static void ParseHookDefinitionsFromText(const std::string& s, std::vector<HookDefinition>& out) {
    std::string content = s;
    StripLineComments(content);
    std::regex headerRe(R"(([^\s(]+)\s*\(([^)]*)\)\s*\{)");
    size_t searchStart = 0;
    while (searchStart < content.size()) {
        std::smatch m;
        std::string sub = content.substr(searchStart);
        if (!std::regex_search(sub, m, headerRe)) break;
        size_t relPos = (size_t)m.position(0);
        std::string fullName = m[1].str();
        if (fullName.find("$$") == std::string::npos) {
            searchStart += relPos + 1;
            continue;
        }
        size_t matchLen = (size_t)m.length(0);
        size_t braceOpen = searchStart + relPos + matchLen - 1;
        if (braceOpen >= content.size() || content[braceOpen] != '{') {
            searchStart += relPos + 1;
            continue;
        }
        int depth = 0;
        size_t braceClose = content.size();
        bool found = false;
        for (size_t i = braceOpen; i < content.size(); ++i) {
            if (content[i] == '{') depth++;
            else if (content[i] == '}') {
                depth--;
                if (depth == 0) {
                    braceClose = i;
                    found = true;
                    break;
                }
            }
        }
        if (!found) break;

        std::string body = content.substr(braceOpen + 1, braceClose - braceOpen - 1);
        HookDefinition def;
        def.full_name = std::move(fullName);
        std::string params = m[2].str();
        SplitIl2CppFullName(def.full_name, def.namespaze, def.class_name, def.method_name);
        def.signature = InferSignatureFromParamList(params);
        ParseHookBody(body, def);

        out.push_back(std::move(def));
        searchStart = braceClose + 1;
    }
}

bool LoadGameHookDefinitions(std::vector<HookDefinition>& out_defs, std::wstring& out_config_path) {
    out_defs.clear();
    wchar_t envPath[4096];
    DWORD envLen = GetEnvironmentVariableW(L"LIZARD_HOOKS_FILE", envPath, (DWORD)(sizeof(envPath) / sizeof(envPath[0])));
    if (envLen > 0 && envLen < sizeof(envPath) / sizeof(envPath[0])) {
        out_config_path.assign(envPath);
    } else {
        wchar_t exePath[MAX_PATH];
        if (!GetModuleFileNameW(NULL, exePath, MAX_PATH)) return false;
        std::wstring dir(exePath);
        size_t slash = dir.find_last_of(L"\\/");
        if (slash != std::wstring::npos) dir.resize(slash + 1);
        else dir.clear();
        out_config_path = dir + L"LizardHooks.txt";
    }

    std::string utf8;
    if (!ReadWholeFile(out_config_path, utf8)) return false;
    SkipUtf8Bom(utf8);
    ParseHookDefinitionsFromText(utf8, out_defs);
    return true;
}
