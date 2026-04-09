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
#include <unordered_set>
#include <vector>
#include <wincon.h>

uintptr_t GameAssembly = (uintptr_t)GetModuleHandle("GameAssembly.dll");

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
    MH_Initialize();
    CreateConsole();
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
    LizardLogf(g_colorInfo, "Cheats: Up/Down, Enter toggle, Esc exit. Keys 1-3 switch tabs.\n\n");
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

static void DrawExplorerPanel(const std::string& query, const std::vector<std::string>& results, int resultScroll, const std::string& status) {
    ClearConsoleScreen();
    LizardLogf(g_colorOk, "LizardW2Hack by PotJoke\n");
    PrintTabBar(MainTab::Explorer);
    LizardLogf(g_colorInfo, "Explorer: ASCII substring in method name, Enter to search. Up/Down scroll results.\n");
    LizardLogf(g_colorWarn, "Note: keys 1-3 switch tabs (avoid leading digits in query).\n");
    LizardLogf(g_colorInfo, "If search says path empty, wait until GameAssembly is loaded (e.g. main menu) and try again.\n\n");
    LizardLogf(g_colorOffset, "Query: %s\n", query.c_str());
    if (!status.empty()) LizardLogf(g_colorOk, "%s\n", status.c_str());

    short rows, cols;
    GetConsoleSize(rows, cols);
    int visible = (int)rows - 14;
    if (visible < 4) visible = 4;
    int maxScroll = (int)results.size() > visible ? (int)results.size() - visible : 0;
    int rscroll = resultScroll;
    if (rscroll > maxScroll) rscroll = maxScroll;
    if (rscroll < 0) rscroll = 0;

    LizardLogf(g_colorOffset, "--- %zu matches (offset %d) ---\n", results.size(), rscroll);
    for (int i = rscroll; i < (int)results.size() && i < rscroll + visible; ++i) {
        LizardLogf(g_colorInfo, "%s\n", results[(size_t)i].c_str());
    }
}

bool CreateHookWithResolvedOffset(const char* hookName, uintptr_t offset, LPVOID detour, LPVOID* original) {
    if (offset == 0) {
        LizardSessionLogf(g_colorWarn, "[Hook] Skip %s: resolved offset is 0x0\n", hookName);
        return false;
    }

    uintptr_t targetAddr = GameAssembly + offset;
    if (g_hookedTargets.find(targetAddr) != g_hookedTargets.end()) {
        LizardSessionLogf(g_colorWarn, "[Hook] Skip %s: duplicate target 0x%llX already hooked\n", hookName, (unsigned long long)offset);
        return false;
    }

    LPVOID target = reinterpret_cast<LPVOID>(targetAddr);
    MH_STATUS status = MH_CreateHook(target, detour, original);
    if (status != MH_OK) {
        LizardSessionLogf(g_colorError, "[Hook] Failed %s: offset=0x%llX, MH_STATUS=%d\n", hookName, (unsigned long long)offset, (int)status);
        return false;
    }

    g_hookedTargets.insert(targetAddr);
    LizardSessionLogf(g_colorOk, "[Hook] Ready %s: offset=0x%llX\n", hookName, (unsigned long long)offset);
    return true;
}

static DWORD WINAPI ThreadMain(LPVOID /*param*/) {
    Init();
    PrintBanner();
    LizardSessionLogf(g_colorInfo, "Ready to serve master!\n");
    LizardSessionLogf(g_colorWarn, "Please do NOT close console\n");
    ResolvePathsRelativeToGameAssembly();
    LizardSessionLogf(g_colorInfo, "[Resolver] global-metadata path: %s\n", g_globalMetadataPath.c_str());
    LizardSessionLogf(g_colorInfo, "[Resolver] GameAssembly path: %s\n", g_gameAssemblyPath.c_str());
    LizardSessionLogf(g_colorInfo, "[Resolver] methodPointers RVA cache/current: 0x%p\n", (void*)g_methodPointersRva);

    std::vector<HookDefinition> defs;
    std::wstring cfgPath;
    for (int i = 0; i < kMaxHookSlots; ++i) g_hook_originals[i] = nullptr;
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
            uintptr_t off = FindMethodOffsetByClassAndMethod(
                d.namespaze.c_str(), d.class_name.c_str(), d.method_name.c_str(), d.param_count);
            LizardSessionLogf(off == 0 ? g_colorError : g_colorOffset, "[Offsets] %s -> 0x%p\n", d.full_name.c_str(), (void*)off);
            void* det = GetDetourForSignature(d.signature, (int)i);
            CreateHookWithResolvedOffset(d.full_name.c_str(), off, det, &g_hook_originals[(int)i]);
        }
    }

    MH_EnableHook(MH_ALL_HOOKS);
    const size_t menuCount = (defs.size() < (size_t)kMaxHookSlots) ? defs.size() : (size_t)kMaxHookSlots;

    Sleep(2000);

    MainTab tab = MainTab::Cheats;
    int selected = 0;
    int logScroll = 0;
    std::string explorerQuery;
    std::vector<std::string> explorerResults;
    int explorerScroll = 0;
    std::string explorerStatus;

    auto redraw = [&]() {
        switch (tab) {
        case MainTab::Cheats:
            DrawCheatsPanel(defs, menuCount, selected);
            break;
        case MainTab::Logs:
            DrawLogsPanel(logScroll);
            break;
        case MainTab::Explorer:
            DrawExplorerPanel(explorerQuery, explorerResults, explorerScroll, explorerStatus);
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
            if (ch == 27) break;
            if (ch == 13) {
                if (menuCount > 0) {
                    g_hook_slots[selected].enabled = !g_hook_slots[selected].enabled;
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
                short rows, cols;
                GetConsoleSize(rows, cols);
                int visible = (int)rows - 14;
                if (visible < 4) visible = 4;
                int maxScroll = (int)explorerResults.size() > visible ? (int)explorerResults.size() - visible : 0;
                if (k == 72) {
                    explorerScroll--;
                    if (explorerScroll < 0) explorerScroll = 0;
                    redraw();
                } else if (k == 80) {
                    explorerScroll++;
                    if (explorerScroll > maxScroll) explorerScroll = maxScroll;
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
                explorerScroll = 0;
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
