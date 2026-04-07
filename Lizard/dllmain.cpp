#include "pch.h"
#include "hook_config.h"
#include "banner_logo.inc"

#include <MinHook.h>
#include <conio.h>
#include <cstdarg>
#include <cstdio>
#include <cwchar>
#include <unordered_set>
#include <vector>
#include <wincon.h>

uintptr_t GameAssembly = (uintptr_t)GetModuleHandle("GameAssembly.dll");

HANDLE g_console = nullptr;
WORD g_colorInfo = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY;
WORD g_colorOk = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
WORD g_colorWarn = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
WORD g_colorError = FOREGROUND_RED | FOREGROUND_INTENSITY;
WORD g_colorOffset = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
std::unordered_set<uintptr_t> g_hookedTargets;

// Default conhost fonts often lack Braille (U+2800..) — tofu squares. Copy/paste still works because text is correct UTF-16.
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

void PrintColor(WORD color, const char* fmt, ...) {
    if (g_console) SetConsoleTextAttribute(g_console, color);
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    if (g_console) SetConsoleTextAttribute(g_console, g_colorInfo);
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

// printf("%s", utf8) often breaks for non-ASCII: CRT may not emit UTF-8 the way conhost expects (mojibake like "вЂ").
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
    PrintColor(color, "%s\n", utf8);
}

static void PrintBannerLogo() {
    WriteConsoleUtf8(g_colorOffset, GetLizardBannerLogoUtf8());
}

static void PrintBanner() {
    PrintBannerLogo();
    PrintColor(g_colorOk, "=== LizardW2Hack ===\n");
    PrintColor(g_colorInfo, "Author: PotJoke\n");
    PrintColor(g_colorWarn, "Starting in 5 seconds...\n\n");
    Sleep(5000);
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

static void DrawHooksMenu(const std::vector<HookDefinition>& defs, size_t itemCount, int selected) {
    ClearConsoleScreen();
    PrintBannerLogo();
    PrintColor(g_colorOk, "LizardW2Hack by PotJoke\n");
    PrintColor(g_colorOk, "=== Main menu ===\n");
    PrintColor(g_colorInfo, "Use Up/Down arrows, Enter to toggle, Esc to exit menu loop.\n\n");
    for (size_t i = 0; i < itemCount; ++i) {
        const HookSlotRuntime& s = g_hook_slots[(int)i];
        const char* mark = ((int)i == selected) ? "*" : " ";
        const char* state = s.enabled ? "ON " : "OFF";
        WORD stateColor = s.enabled ? g_colorOk : g_colorError;
        PrintColor(g_colorInfo, "%s [%02d] ", mark, (int)i + 1);
        PrintColor(stateColor, "%s ", state);
        PrintColor(g_colorInfo, "- %s\n", s.display_name.c_str());
    }
}

bool CreateHookWithResolvedOffset(const char* hookName, uintptr_t offset, LPVOID detour, LPVOID* original) {
    if (offset == 0) {
        PrintColor(g_colorWarn, "[Hook] Skip %s: resolved offset is 0x0\n", hookName);
        return false;
    }

    uintptr_t targetAddr = GameAssembly + offset;
    if (g_hookedTargets.find(targetAddr) != g_hookedTargets.end()) {
        PrintColor(g_colorWarn, "[Hook] Skip %s: duplicate target 0x%llX already hooked\n", hookName, (unsigned long long)offset);
        return false;
    }

    LPVOID target = reinterpret_cast<LPVOID>(targetAddr);
    MH_STATUS status = MH_CreateHook(target, detour, original);
    if (status != MH_OK) {
        PrintColor(g_colorError, "[Hook] Failed %s: offset=0x%llX, MH_STATUS=%d\n", hookName, (unsigned long long)offset, (int)status);
        return false;
    }

    g_hookedTargets.insert(targetAddr);
    PrintColor(g_colorOk, "[Hook] Ready %s: offset=0x%llX\n", hookName, (unsigned long long)offset);
    return true;
}

static DWORD WINAPI ThreadMain(LPVOID /*param*/) {
    Init();
    PrintBanner();
    PrintColor(g_colorInfo, "Ready to serve master!\n");
    PrintColor(g_colorWarn, "Please do NOT close console\n");
    ResolvePathsRelativeToGameAssembly();
    PrintColor(g_colorInfo, "[Resolver] global-metadata path: %s\n", g_globalMetadataPath.c_str());
    PrintColor(g_colorInfo, "[Resolver] GameAssembly path: %s\n", g_gameAssemblyPath.c_str());
    PrintColor(g_colorInfo, "[Resolver] methodPointers RVA cache/current: 0x%p\n", (void*)g_methodPointersRva);

    std::vector<HookDefinition> defs;
    std::wstring cfgPath;
    for (int i = 0; i < kMaxHookSlots; ++i) g_hook_originals[i] = nullptr;
    if (!LoadGameHookDefinitions(defs, cfgPath)) {
        PrintColor(g_colorWarn, "[Hooks] Cannot read config: %ls\n", cfgPath.c_str());
    } else {
        PrintColor(g_colorInfo, "[Hooks] Config: %ls (%zu hook(s))\n", cfgPath.c_str(), defs.size());
        for (size_t i = 0; i < defs.size() && i < (size_t)kMaxHookSlots; ++i) {
            const HookDefinition& d = defs[i];
            g_hook_slots[(int)i].display_name = d.display_name;
            g_hook_slots[(int)i].full_name = d.full_name;
            g_hook_slots[(int)i].enabled = d.enabled_on_start;
            g_hook_slots[(int)i].log_line = d.log_line;
            g_hook_slots[(int)i].return_bool = d.return_bool;
            uintptr_t off = FindMethodOffsetByClassAndMethod(
                d.namespaze.c_str(), d.class_name.c_str(), d.method_name.c_str(), d.param_count);
            PrintColor(off == 0 ? g_colorError : g_colorOffset, "[Offsets] %s -> 0x%p\n", d.full_name.c_str(), (void*)off);
            void* det = GetDetourForSignature(d.signature, (int)i);
            CreateHookWithResolvedOffset(d.full_name.c_str(), off, det, &g_hook_originals[(int)i]);
        }
    }

    MH_EnableHook(MH_ALL_HOOKS);
    const size_t menuCount = (defs.size() < (size_t)kMaxHookSlots) ? defs.size() : (size_t)kMaxHookSlots;
    if (menuCount > 0) {
        Sleep(2000);
        int selected = 0;
        DrawHooksMenu(defs, menuCount, selected);
        for (;;) {
            int ch = _getch();
            if (ch == 27) break;
            if (ch == 13) {
                g_hook_slots[selected].enabled = !g_hook_slots[selected].enabled;
                DrawHooksMenu(defs, menuCount, selected);
                continue;
            }
            if (ch == 224 || ch == 0) {
                int k = _getch();
                if (k == 72) {
                    selected = (selected - 1 + (int)menuCount) % (int)menuCount;
                    DrawHooksMenu(defs, menuCount, selected);
                } else if (k == 80) {
                    selected = (selected + 1) % (int)menuCount;
                    DrawHooksMenu(defs, menuCount, selected);
                }
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
