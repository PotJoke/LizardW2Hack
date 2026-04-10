#include "pch.h"
#include "hook_config.h"
#include "metadata_explorer.h"
#include "banner_logo.inc"

#include <MinHook.h>
#include <algorithm>
#include <conio.h>
#include <cstdio>
#include <cwchar>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <wincon.h>

static GameRuntimeMode g_runtimeMode = GameRuntimeMode::Unknown;

WORD g_colorOk = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
WORD g_colorWarn = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
WORD g_colorError = FOREGROUND_RED | FOREGROUND_INTENSITY;
WORD g_colorOffset = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
std::unordered_set<uintptr_t> g_hookedTargets;

enum class MainTab { Cheats = 0, Logs = 1, Explorer = 2 };

static void TrySetBrailleConsoleFont(HANDLE hOut) {
    if (!hOut || hOut == INVALID_HANDLE_VALUE) return;

    CONSOLE_FONT_INFOEX base{};
    base.cbSize = sizeof(base);
    if (!GetCurrentConsoleFontEx(hOut, FALSE, &base)) {
        base.dwFontSize.X = 0;
        base.dwFontSize.Y = 16;
        base.FontFamily = FF_DONTCARE;
        base.FontWeight = 400;
    }

    static const wchar_t* kFaces[] = {
        L"Cascadia Mono",
        L"Cascadia Code",
        L"Segoe UI Symbol",
        L"Segoe UI",
    };
    for (const wchar_t* face : kFaces) {
        CONSOLE_FONT_INFOEX cfi = base;
        if (wcscpy_s(cfi.FaceName, LF_FACESIZE, face) != 0) continue;
        if (SetCurrentConsoleFontEx(hOut, FALSE, &cfi)) return;
    }
}

static void CreateConsole() {
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    g_console = GetStdHandle(STD_OUTPUT_HANDLE);
    if (g_console && g_console != INVALID_HANDLE_VALUE) {
        TrySetBrailleConsoleFont(g_console);
        SetConsoleTextAttribute(g_console, g_colorInfo);
    }
}

static void WriteConsoleUtf8(WORD color, const char* utf8) {
    if (!utf8) return;
    if (g_console && g_console != INVALID_HANDLE_VALUE) {
        SetConsoleTextAttribute(g_console, color);
        int n = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, nullptr, 0);
        if (n > 0) {
            std::vector<wchar_t> buf((size_t)n);
            MultiByteToWideChar(CP_UTF8, 0, utf8, -1, buf.data(), n);
            DWORD written = 0;
            WriteConsoleW(g_console, buf.data(), (DWORD)(n - 1), &written, nullptr);
        }
        DWORD w2 = 0;
        WriteConsoleW(g_console, L"\n", 1, &w2, nullptr);
        SetConsoleTextAttribute(g_console, g_colorInfo);
        return;
    }
    LizardLogf(color, "%s\n", utf8);
}

static void PrintBannerLogo() {
    WriteConsoleUtf8(g_colorOffset, GetLizardBannerLogoUtf8());
}

static void PrintBanner() {
    PrintBannerLogo();
    LizardLogf(g_colorOk, "=== LizardW2Hack ===\n");
    LizardLogf(g_colorInfo, "Author: PotJoke\n");
    LizardLogf(g_colorWarn, "Starting in 5 seconds...\n\n");
}

static void Init() {
    MH_STATUS mh = MH_Initialize();
    CreateConsole();
    if (mh != MH_OK && mh != MH_ERROR_ALREADY_INITIALIZED) {
        LizardSessionLogf(g_colorError, "[Hook] MH_Initialize failed: %d\n", (int)mh);
    }
}

static void ClearConsoleScreen() {
    if (!g_console) return;
    CONSOLE_SCREEN_BUFFER_INFO csbi{};
    if (!GetConsoleScreenBufferInfo(g_console, &csbi)) return;
    DWORD cellCount = (DWORD)csbi.dwSize.X * (DWORD)csbi.dwSize.Y;
    DWORD written = 0;
    COORD home = { 0, 0 };
    FillConsoleOutputCharacterA(g_console, ' ', cellCount, home, &written);
    FillConsoleOutputAttribute(g_console, csbi.wAttributes, cellCount, home, &written);
    SetConsoleCursorPosition(g_console, home);
}

static void GetConsoleSize(short& rows, short& cols) {
    CONSOLE_SCREEN_BUFFER_INFO csbi{};
    if (g_console && GetConsoleScreenBufferInfo(g_console, &csbi)) {
        cols = csbi.dwSize.X;
        rows = csbi.dwSize.Y;
    } else {
        cols = 80;
        rows = 25;
    }
}

static void PrintTabBar(MainTab current) {
    const char* t1 = (current == MainTab::Cheats) ? "[1] Cheats *" : "[1] Cheats";
    const char* t2 = (current == MainTab::Logs) ? "[2] Logs *" : "[2] Logs";
    const char* t3 = (current == MainTab::Explorer) ? "[3] Explorer *" : "[3] Explorer";
    LizardLogf(g_colorOk, "%s  %s  %s\n\n", t1, t2, t3);
}

static void DrawCheatsPanel(const std::vector<HookDefinition>& defs, size_t itemCount, int selected) {
    ClearConsoleScreen();
    PrintBannerLogo();
    LizardLogf(g_colorOk, "LizardW2Hack by PotJoke\n");
    PrintTabBar(MainTab::Cheats);
    LizardLogf(g_colorInfo, "Cheats: Up/Down, Enter toggle runtime, R reload config, E edit selected, Esc exit. Keys 1-3 switch tabs.\n");
    if (g_runtimeMode == GameRuntimeMode::Mono) {
        LizardLogf(g_colorInfo, "Mono mode: bool methods are hookable (supported arg shapes: total native args 0..5).\n");
    }
    LizardLogf(g_colorInfo, "\n");
    if (itemCount == 0) {
        LizardLogf(g_colorWarn, "No hooks loaded (check LizardHooks.txt).\n");
        return;
    }
    for (size_t i = 0; i < itemCount; ++i) {
        const HookSlotRuntime& s = g_hook_slots[(int)i];
        const char* mark = ((int)i == selected) ? "*" : " ";
        const char* state = s.enabled ? "ON " : "OFF";
        WORD stateColor = s.enabled ? g_colorOk : g_colorError;
        LizardLogf(g_colorInfo, "%s [%02d] ", mark, (int)i + 1);
        LizardLogf(stateColor, "%s ", state);
        LizardLogf(g_colorInfo, "- %s\n", s.display_name.c_str());
    }
}

static void DrawCheatsEditPanel(const HookDefinition& def, int selectedField, const std::string& status) {
    ClearConsoleScreen();
    PrintBannerLogo();
    LizardLogf(g_colorOk, "LizardW2Hack by PotJoke\n");
    PrintTabBar(MainTab::Cheats);
    LizardLogf(g_colorWarn, "Config Editor: Up/Down select field, Enter edit/toggle, S save, Esc cancel.\n\n");
    if (!status.empty()) LizardLogf(g_colorOk, "%s\n\n", status.c_str());

    auto printField = [&](int idx, const char* label, const std::string& value) {
        LizardLogf(idx == selectedField ? g_colorOffset : g_colorInfo, "%s %-16s: %s\n",
            idx == selectedField ? ">" : " ", label, value.c_str());
    };

    printField(0, "display_name", def.display_name);
    printField(1, "enabled_on_start", def.enabled_on_start ? "true" : "false");
    printField(2, "return_bool", def.return_bool ? "true" : "false");
    printField(3, "param_count", std::to_string(def.param_count));
    printField(4, "log_line", def.log_line.empty() ? "(empty)" : def.log_line);
}

static bool ReadAsciiLineInline(const std::string& title, std::string& inOut) {
    std::string value = inOut;
    for (;;) {
        ClearConsoleScreen();
        PrintBannerLogo();
        LizardLogf(g_colorOk, "LizardW2Hack by PotJoke\n");
        PrintTabBar(MainTab::Cheats);
        LizardLogf(g_colorInfo, "%s\n", title.c_str());
        LizardLogf(g_colorWarn, "Enter apply, Esc cancel, Backspace delete.\n\n");
        LizardLogf(g_colorOffset, "> %s", value.c_str());

        int ch = _getch();
        if (ch == 27) return false;
        if (ch == 13) {
            inOut = value;
            return true;
        }
        if (ch == 8) {
            if (!value.empty()) value.pop_back();
            continue;
        }
        if (ch >= 32 && ch < 127) value += (char)ch;
    }
}

static void DrawLogsPanel(int scroll) {
    ClearConsoleScreen();
    LizardLogf(g_colorOk, "LizardW2Hack by PotJoke\n");
    PrintTabBar(MainTab::Logs);
    LizardLogf(g_colorInfo, "Logs: PgUp/PgDn scroll, 1-3 tabs, Esc exit.\n\n");

    std::vector<std::string> lines;
    LizardGetLogSnapshot(lines);
    short rows = 25, cols = 80;
    GetConsoleSize(rows, cols);
    (void)cols;
    int visible = (int)rows - 10;
    if (visible < 5) visible = 5;
    int maxScroll = (int)lines.size() > visible ? (int)lines.size() - visible : 0;
    if (scroll > maxScroll) scroll = maxScroll;
    if (scroll < 0) scroll = 0;

    size_t end = lines.empty() ? 0 : (size_t)std::min<int>((int)lines.size(), scroll + visible);
    LizardLogf(g_colorOffset, "--- %zu lines, view %d-%d ---\n", lines.size(), scroll + 1, (int)end);
    for (int i = scroll; i < (int)lines.size() && i < scroll + visible; ++i) {
        LizardLogf(g_colorInfo, "%s\n", lines[(size_t)i].c_str());
    }
}

static std::string ExplorerEllipsizeAscii(const std::string& s, size_t maxChars) {
    if (s.size() <= maxChars) return s;
    if (maxChars <= 3) return s.substr(0, maxChars);
    return s.substr(0, maxChars - 3) + "...";
}

static std::string ExplorerQualifiedToHookFullName(const std::string& qualifiedMethod) {
    std::string out = qualifiedMethod;
    size_t p = out.rfind("::");
    if (p != std::string::npos) out.replace(p, 2, "$$");
    return out;
}

static int ExplorerParamCountFromParamList(const std::string& paramList) {
    size_t l = paramList.find('(');
    size_t r = paramList.rfind(')');
    if (l == std::string::npos || r == std::string::npos || r <= l + 1) return 0;
    std::string inside = paramList.substr(l + 1, r - l - 1);
    while (!inside.empty() && (inside.front() == ' ' || inside.front() == '\t')) inside.erase(inside.begin());
    while (!inside.empty() && (inside.back() == ' ' || inside.back() == '\t')) inside.pop_back();
    if (inside.empty() || inside == "void") return 0;
    int n = 1;
    for (char c : inside) if (c == ',') n++;
    return n;
}

static void DrawExplorerPanel(
    const std::string& query,
    const std::vector<ExplorerMethodHit>& results,
    int selectedResult,
    const std::unordered_set<std::string>& configuredMethods,
    const std::string& status
) {
    ClearConsoleScreen();
    LizardLogf(g_colorOk, "LizardW2Hack by PotJoke\n");
    PrintTabBar(MainTab::Explorer);
    LizardLogf(g_colorInfo, "Explorer: Enter search, Up/Down select. '+' add to config, '-' delete from config.\n");
    LizardLogf(g_colorWarn, "Note: keys 1-3 switch tabs (avoid leading digits in query).\n");
    if (g_runtimeMode == GameRuntimeMode::Mono) {
        LizardLogf(g_colorInfo, "Mono runtime detected: searching loaded assemblies/methods in runtime.\n\n");
    } else {
        LizardLogf(g_colorInfo, "If search says path empty, wait until GameAssembly is loaded (e.g. main menu) and try again.\n\n");
    }
    LizardLogf(g_colorOffset, "Query: %s\n", query.c_str());
    if (!status.empty()) LizardLogf(g_colorOk, "%s\n", status.c_str());

    short rows, cols;
    GetConsoleSize(rows, cols);
    // Two console lines per hit (name row + params/RVA row).
    int visible = ((int)rows - 14) / 2;
    if (visible < 3) visible = 3;
    int selected = selectedResult;
    if (selected < 0) selected = 0;
    if (selected >= (int)results.size()) selected = (int)results.size() - 1;
    int rscroll = selected - (visible / 2);
    if (rscroll < 0) rscroll = 0;
    int maxScroll = (int)results.size() > visible ? (int)results.size() - visible : 0;
    if (rscroll > maxScroll) rscroll = maxScroll;

    LizardLogf(g_colorOffset, "--- %zu matches (offset %d) ---\n", results.size(), rscroll);

    const WORD colorRet = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    const WORD colorName = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
    const WORD colorParams = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    const WORD colorSelected = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
    const WORD colorConfigured = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    const WORD colorMissing = FOREGROUND_RED | FOREGROUND_INTENSITY;

    for (int i = rscroll; i < (int)results.size() && i < rscroll + visible; ++i) {
        const ExplorerMethodHit& h = results[(size_t)i];
        const bool isSelected = (i == selected);
        const bool isConfigured = configuredMethods.find(ExplorerQualifiedToHookFullName(h.qualifiedMethod)) != configuredMethods.end();
        std::string retCol = ExplorerEllipsizeAscii(h.returnType, 28);
        std::string nameCol = ExplorerEllipsizeAscii(h.qualifiedMethod, 72);
        LizardLogf(isSelected ? colorSelected : g_colorInfo, "%s ", isSelected ? ">" : " ");
        LizardLogf(colorRet, "%s  ", retCol.c_str());
        LizardLogf(isConfigured ? colorConfigured : colorName, "%s\n", nameCol.c_str());
        LizardLogf(g_colorInfo, "  ");
        LizardLogf(colorParams, "%s", h.paramList.c_str());
        if (!h.rvaText.empty()) {
            LizardLogf(g_colorInfo, "  ");
            LizardLogf(g_colorOffset, "%s", h.rvaText.c_str());
        }
        LizardLogf(g_colorInfo, "  ");
        LizardLogf(isConfigured ? colorConfigured : colorMissing, "%s", isConfigured ? "[IN CONFIG]" : "[NOT IN CONFIG]");
        LizardLogf(g_colorInfo, "\n");
    }
}

bool CreateHookWithResolvedOffset(const char* hookName, uintptr_t offset, LPVOID detour, LPVOID* original) {
    if (offset == 0) {
        LizardSessionLogf(g_colorWarn, "[Hook] Skip %s: resolved address is 0x0\n", hookName);
        return false;
    }

    uintptr_t targetAddr = offset;
    if (g_hookedTargets.find(targetAddr) != g_hookedTargets.end()) {
        LizardSessionLogf(g_colorWarn, "[Hook] Skip %s: duplicate target 0x%p already hooked\n", hookName, (void*)targetAddr);
        return false;
    }

    LPVOID target = reinterpret_cast<LPVOID>(targetAddr);
    MH_STATUS status = MH_CreateHook(target, detour, original);
    if (status != MH_OK) {
        LizardSessionLogf(g_colorError, "[Hook] Failed %s: address=0x%p, MH_STATUS=%d\n", hookName, (void*)targetAddr, (int)status);
        return false;
    }

    g_hookedTargets.insert(targetAddr);
    LizardSessionLogf(g_colorOk, "[Hook] Ready %s: address=0x%p\n", hookName, (void*)targetAddr);
    return true;
}

static DWORD WINAPI ThreadMain(LPVOID /*param*/) {
    Init();
    PrintBanner();
    LizardSessionLogf(g_colorInfo, "Ready to serve master!\n");
    LizardSessionLogf(g_colorWarn, "Please do NOT close console\n");
    for (int i = 0; i < 120 && g_runtimeMode == GameRuntimeMode::Unknown; ++i) {
        g_runtimeMode = DetectGameRuntimeMode();
        if (g_runtimeMode != GameRuntimeMode::Unknown) break;
        Sleep(250);
    }
    LizardSessionLogf(g_colorInfo, "[Runtime] detected mode: %s\n", RuntimeModeToString(g_runtimeMode));
    if (g_runtimeMode == GameRuntimeMode::Il2Cpp) {
        ResolvePathsRelativeToGameAssembly();
        LizardSessionLogf(g_colorInfo, "[Resolver] global-metadata path: %s\n", g_globalMetadataPath.c_str());
        LizardSessionLogf(g_colorInfo, "[Resolver] GameAssembly path: %s\n", g_gameAssemblyPath.c_str());
        LizardSessionLogf(g_colorInfo, "[Resolver] methodPointers RVA cache/current: 0x%p\n", (void*)g_methodPointersRva);
    } else if (g_runtimeMode == GameRuntimeMode::Mono) {
        HMODULE monoMod = GetMonoModuleHandle();
        LizardSessionLogf(g_colorInfo, "[MonoResolver] module: 0x%p\n", monoMod);
    } else {
        LizardSessionLogf(g_colorWarn, "[Runtime] failed to detect runtime. Hooks will not be resolved.\n");
    }

    std::vector<HookDefinition> defs;
    std::wstring cfgPath;
    for (int i = 0; i < kMaxHookSlots; ++i) g_hook_originals[i] = nullptr;
    auto rehookFromCurrentDefs = [&]() {
        MH_DisableHook(MH_ALL_HOOKS);
        std::vector<uintptr_t> targets(g_hookedTargets.begin(), g_hookedTargets.end());
        for (uintptr_t targetAddr : targets) {
            MH_RemoveHook(reinterpret_cast<LPVOID>(targetAddr));
        }
        g_hookedTargets.clear();
        for (int i = 0; i < kMaxHookSlots; ++i) g_hook_originals[i] = nullptr;

        for (size_t i = 0; i < defs.size() && i < (size_t)kMaxHookSlots; ++i) {
            const HookDefinition& d = defs[i];
            uintptr_t methodAddr = 0;
            void* det = nullptr;

            if (g_runtimeMode == GameRuntimeMode::Mono) {
                MonoResolvedMethodInfo mi = ResolveMonoMethodForHook(
                    d.namespaze.c_str(), d.class_name.c_str(), d.method_name.c_str(), d.param_count);
                if (!mi.found || mi.address == 0) {
                    LizardSessionLogf(g_colorError, "[MonoHook] %s -> not found\n", d.full_name.c_str());
                    continue;
                }
                if (!mi.returnsBool) {
                    LizardSessionLogf(g_colorWarn, "[MonoHook] %s skipped: return type is not bool.\n", d.full_name.c_str());
                    continue;
                }
                int totalArgs = (mi.isStatic ? 0 : 1) + mi.managedParamCount;
                det = GetMonoDetourForTotalArgs(totalArgs, (int)i);
                if (!det) {
                    LizardSessionLogf(g_colorWarn, "[MonoHook] %s skipped: unsupported arg shape (total native args=%d).\n", d.full_name.c_str(), totalArgs);
                    continue;
                }
                methodAddr = mi.address;
                LizardSessionLogf(g_colorOffset, "[MonoHook] %s -> addr=0x%p static=%s managedParams=%d totalArgs=%d\n",
                    d.full_name.c_str(), (void*)methodAddr, mi.isStatic ? "true" : "false", mi.managedParamCount, totalArgs);
            } else {
                methodAddr = ResolveMethodAddressByRuntime(
                    g_runtimeMode,
                    d.namespaze.c_str(), d.class_name.c_str(), d.method_name.c_str(), d.param_count);
                LizardSessionLogf(methodAddr == 0 ? g_colorError : g_colorOffset, "[Resolver] %s -> 0x%p\n", d.full_name.c_str(), (void*)methodAddr);
                det = GetDetourForSignature(d.signature, (int)i);
            }

            CreateHookWithResolvedOffset(d.full_name.c_str(), methodAddr, det, &g_hook_originals[(int)i]);
        }

        MH_EnableHook(MH_ALL_HOOKS);
    };

    if (!LoadGameHookDefinitions(defs, cfgPath)) {
        LizardSessionLogf(g_colorWarn, "[Hooks] Cannot read config: %ls\n", cfgPath.c_str());
    } else {
        LizardSessionLogf(g_colorInfo, "[Hooks] Config: %ls (%zu hook(s))\n", cfgPath.c_str(), defs.size());
        for (size_t i = 0; i < defs.size() && i < (size_t)kMaxHookSlots; ++i) {
            const HookDefinition& d = defs[i];
            g_hook_slots[(int)i].display_name = d.display_name;
            g_hook_slots[(int)i].full_name = d.full_name;
            g_hook_slots[(int)i].enabled = d.enabled_on_start;
            g_hook_slots[(int)i].log_line = d.log_line;
            g_hook_slots[(int)i].return_bool = d.return_bool;
        }
        rehookFromCurrentDefs();
    }
    size_t menuCount = (defs.size() < (size_t)kMaxHookSlots) ? defs.size() : (size_t)kMaxHookSlots;

    Sleep(2000);

    MainTab tab = MainTab::Cheats;
    int selected = 0;
    int logScroll = 0;
    bool cheatsEditMode = false;
    int cheatsEditField = 0;
    int cheatsEditIndex = -1;
    HookDefinition cheatsEditBuffer;
    HookDefinition cheatsEditOriginal;
    std::string cheatsEditStatus;
    std::string explorerQuery;
    std::vector<ExplorerMethodHit> explorerResults;
    int explorerSelected = 0;
    std::string explorerStatus;
    std::unordered_set<std::string> configuredMethods;
    for (const HookDefinition& d : defs) configuredMethods.insert(d.full_name);

    auto reloadConfigFromFile = [&]() -> bool {
        std::unordered_map<std::string, bool> runtimeEnabledByName;
        runtimeEnabledByName.reserve((size_t)kMaxHookSlots);
        for (int i = 0; i < kMaxHookSlots; ++i) {
            if (!g_hook_slots[i].full_name.empty()) {
                runtimeEnabledByName[g_hook_slots[i].full_name] = g_hook_slots[i].enabled;
            }
        }

        std::vector<HookDefinition> reloaded;
        std::wstring reloadedPath;
        if (!LoadGameHookDefinitions(reloaded, reloadedPath)) {
            LizardSessionLogf(g_colorError, "[Hooks] Reload failed: %ls\n", reloadedPath.c_str());
            return false;
        }
        defs = std::move(reloaded);
        cfgPath = std::move(reloadedPath);
        configuredMethods.clear();
        for (const HookDefinition& d : defs) configuredMethods.insert(d.full_name);
        for (int i = 0; i < kMaxHookSlots; ++i) {
            if (i < (int)defs.size()) {
                g_hook_slots[i].display_name = defs[(size_t)i].display_name;
                g_hook_slots[i].full_name = defs[(size_t)i].full_name;
                auto itRuntimeState = runtimeEnabledByName.find(defs[(size_t)i].full_name);
                g_hook_slots[i].enabled = (itRuntimeState != runtimeEnabledByName.end())
                    ? itRuntimeState->second
                    : defs[(size_t)i].enabled_on_start;
                g_hook_slots[i].log_line = defs[(size_t)i].log_line;
                g_hook_slots[i].return_bool = defs[(size_t)i].return_bool;
            } else {
                g_hook_slots[i].display_name.clear();
                g_hook_slots[i].full_name.clear();
                g_hook_slots[i].enabled = false;
                g_hook_slots[i].log_line.clear();
                g_hook_slots[i].return_bool = false;
            }
        }
        menuCount = (defs.size() < (size_t)kMaxHookSlots) ? defs.size() : (size_t)kMaxHookSlots;
        if (selected >= (int)menuCount) selected = menuCount > 0 ? (int)menuCount - 1 : 0;
        rehookFromCurrentDefs();
        LizardSessionLogf(g_colorOk, "[Hooks] Reloaded config: %ls (%zu hook(s))\n", cfgPath.c_str(), defs.size());
        return true;
    };

    auto redraw = [&]() {
        switch (tab) {
        case MainTab::Cheats:
            if (cheatsEditMode) DrawCheatsEditPanel(cheatsEditBuffer, cheatsEditField, cheatsEditStatus);
            else DrawCheatsPanel(defs, menuCount, selected);
            break;
        case MainTab::Logs:
            DrawLogsPanel(logScroll);
            break;
        case MainTab::Explorer:
            DrawExplorerPanel(explorerQuery, explorerResults, explorerSelected, configuredMethods, explorerStatus);
            break;
        }
    };

    redraw();

    for (;;) {
        int ch = _getch();
        if (ch == '1') {
            tab = MainTab::Cheats;
            redraw();
            continue;
        }
        if (ch == '2') {
            tab = MainTab::Logs;
            redraw();
            continue;
        }
        if (ch == '3') {
            tab = MainTab::Explorer;
            redraw();
            continue;
        }

        if (tab == MainTab::Cheats) {
            if (cheatsEditMode) {
                if (ch == 224 || ch == 0) {
                    int k = _getch();
                    if (k == 72) {
                        cheatsEditField--;
                        if (cheatsEditField < 0) cheatsEditField = 4;
                        redraw();
                    } else if (k == 80) {
                        cheatsEditField++;
                        if (cheatsEditField > 4) cheatsEditField = 0;
                        redraw();
                    }
                    continue;
                }
                if (ch == 13) {
                    if (cheatsEditField == 0) {
                        std::string v = cheatsEditBuffer.display_name;
                        if (ReadAsciiLineInline("Edit display_name", v)) {
                            cheatsEditBuffer.display_name = v;
                            cheatsEditStatus = "display_name updated.";
                        }
                    } else if (cheatsEditField == 1) {
                        cheatsEditBuffer.enabled_on_start = !cheatsEditBuffer.enabled_on_start;
                        cheatsEditStatus = "enabled_on_start toggled.";
                    } else if (cheatsEditField == 2) {
                        cheatsEditBuffer.return_bool = !cheatsEditBuffer.return_bool;
                        cheatsEditStatus = "return_bool toggled.";
                    } else if (cheatsEditField == 3) {
                        cheatsEditBuffer.param_count++;
                        cheatsEditStatus = "param_count increased.";
                    } else if (cheatsEditField == 4) {
                        std::string v = cheatsEditBuffer.log_line;
                        if (ReadAsciiLineInline("Edit log_line", v)) {
                            cheatsEditBuffer.log_line = v;
                            cheatsEditStatus = "log_line updated.";
                        }
                    }
                    redraw();
                    continue;
                }
                if (ch == 's' || ch == 'S') {
                    if (cheatsEditIndex >= 0 && cheatsEditIndex < (int)defs.size()) {
                        defs[(size_t)cheatsEditIndex] = cheatsEditBuffer;
                        if (SaveGameHookDefinitions(defs, cfgPath)) {
                            if (reloadConfigFromFile()) {
                                cheatsEditMode = false;
                            } else {
                                cheatsEditStatus = "Saved, but auto-reload failed.";
                            }
                        } else {
                            defs[(size_t)cheatsEditIndex] = cheatsEditOriginal;
                            cheatsEditStatus = "Failed to write config file.";
                        }
                    } else {
                        cheatsEditMode = false;
                    }
                    redraw();
                    continue;
                }
                if (ch == '-') {
                    if (cheatsEditField == 3 && cheatsEditBuffer.param_count > 0) {
                        cheatsEditBuffer.param_count--;
                        cheatsEditStatus = "param_count decreased.";
                        redraw();
                    }
                    continue;
                }
                if (ch == 27) {
                    cheatsEditMode = false;
                    redraw();
                    continue;
                }
                continue;
            }
            if (ch == 27) break;
            if (ch == 13) {
                if (menuCount > 0) {
                    g_hook_slots[selected].enabled = !g_hook_slots[selected].enabled;
                    redraw();
                }
                continue;
            }
            if (ch == 'r' || ch == 'R') {
                reloadConfigFromFile();
                redraw();
                continue;
            }
            if (ch == 'e' || ch == 'E') {
                if (menuCount > 0 && selected >= 0 && selected < (int)menuCount) {
                    cheatsEditIndex = selected;
                    cheatsEditOriginal = defs[(size_t)selected];
                    cheatsEditBuffer = defs[(size_t)selected];
                    cheatsEditField = 0;
                    cheatsEditStatus = "Editing selected config entry.";
                    cheatsEditMode = true;
                    redraw();
                }
                continue;
            }
            if (ch == 224 || ch == 0) {
                int k = _getch();
                if (menuCount == 0) continue;
                if (k == 72) {
                    selected = (selected - 1 + (int)menuCount) % (int)menuCount;
                    redraw();
                } else if (k == 80) {
                    selected = (selected + 1) % (int)menuCount;
                    redraw();
                }
            }
            continue;
        }

        if (tab == MainTab::Logs) {
            if (ch == 27) break;
            if (ch == 224 || ch == 0) {
                int k = _getch();
                std::vector<std::string> lines;
                LizardGetLogSnapshot(lines);
                short rows = 25, cols = 80;
                GetConsoleSize(rows, cols);
                (void)cols;
                int visible = (int)rows - 10;
                if (visible < 5) visible = 5;
                int maxScroll = (int)lines.size() > visible ? (int)lines.size() - visible : 0;
                if (k == 73) {
                    logScroll -= visible;
                    if (logScroll < 0) logScroll = 0;
                    redraw();
                } else if (k == 81) {
                    logScroll += visible;
                    if (logScroll > maxScroll) logScroll = maxScroll;
                    redraw();
                }
            }
            continue;
        }

        if (tab == MainTab::Explorer) {
            if (ch == 27) break;
            if (ch == 224 || ch == 0) {
                int k = _getch();
                if (k == 72) {
                    if (!explorerResults.empty()) {
                        explorerSelected--;
                        if (explorerSelected < 0) explorerSelected = 0;
                    }
                    redraw();
                } else if (k == 80) {
                    if (!explorerResults.empty()) {
                        explorerSelected++;
                        if (explorerSelected >= (int)explorerResults.size()) explorerSelected = (int)explorerResults.size() - 1;
                    }
                    redraw();
                }
                continue;
            }
            if (ch == 8) {
                if (!explorerQuery.empty()) explorerQuery.pop_back();
                explorerStatus.clear();
                redraw();
                continue;
            }
            if (ch == 13) {
                if (explorerQuery.empty()) {
                    explorerStatus = "Type a non-empty substring first.";
                    redraw();
                    continue;
                }
                explorerStatus = "Searching...";
                redraw();
                std::string searchErr;
                if (RunMetadataMethodSearch(explorerQuery, 300, explorerResults, &searchErr)) {
                    explorerStatus = explorerResults.empty() ? "No matches." : "Done.";
                } else {
                    explorerStatus = searchErr.empty() ? "Search failed." : searchErr;
                    explorerResults.clear();
                }
                explorerSelected = 0;
                redraw();
                continue;
            }
            if (ch == '+') {
                if (explorerResults.empty()) {
                    explorerStatus = "Nothing to add: result list is empty.";
                    redraw();
                    continue;
                }
                const ExplorerMethodHit& h = explorerResults[(size_t)explorerSelected];
                std::string fullName = ExplorerQualifiedToHookFullName(h.qualifiedMethod);
                if (configuredMethods.find(fullName) != configuredMethods.end()) {
                    explorerStatus = "Already in config.";
                    redraw();
                    continue;
                }
                HookDefinition d;
                d.full_name = fullName;
                d.display_name = fullName;
                SplitIl2CppFullName(d.full_name, d.namespaze, d.class_name, d.method_name);
                d.signature = InferSignatureFromParamList(h.paramList);
                d.param_count = ExplorerParamCountFromParamList(h.paramList);
                d.enabled_on_start = true;
                d.return_bool = false;
                defs.push_back(d);
                configuredMethods.insert(fullName);
                if (SaveGameHookDefinitions(defs, cfgPath)) {
                    if (reloadConfigFromFile()) explorerStatus = "Added and auto-reloaded config.";
                    else explorerStatus = "Added, but auto-reload failed.";
                } else {
                    defs.pop_back();
                    configuredMethods.erase(fullName);
                    explorerStatus = "Failed to write config file.";
                }
                redraw();
                continue;
            }
            if (ch == '-') {
                if (explorerResults.empty()) {
                    explorerStatus = "Nothing to delete: result list is empty.";
                    redraw();
                    continue;
                }
                const ExplorerMethodHit& h = explorerResults[(size_t)explorerSelected];
                std::string fullName = ExplorerQualifiedToHookFullName(h.qualifiedMethod);
                auto it = std::find_if(defs.begin(), defs.end(), [&](const HookDefinition& d) {
                    return d.full_name == fullName;
                });
                if (it == defs.end()) {
                    explorerStatus = "Method is not in config.";
                    redraw();
                    continue;
                }
                HookDefinition backup = *it;
                defs.erase(it);
                configuredMethods.erase(fullName);
                if (SaveGameHookDefinitions(defs, cfgPath)) {
                    if (reloadConfigFromFile()) explorerStatus = "Removed and auto-reloaded config.";
                    else explorerStatus = "Removed, but auto-reload failed.";
                } else {
                    defs.push_back(std::move(backup));
                    configuredMethods.insert(fullName);
                    explorerStatus = "Failed to write config file.";
                }
                redraw();
                continue;
            }
            if (ch >= 32 && ch < 127) {
                explorerQuery += (char)ch;
                explorerStatus.clear();
                redraw();
            }
        }
    }
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID /*lpReserved*/) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, ThreadMain, nullptr, 0, nullptr);
        break;
    default:
        break;
    }
    return TRUE;
}
