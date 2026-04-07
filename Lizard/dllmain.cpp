#include "pch.h"

#include <MinHook.h>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string>
#include <unordered_set>

uintptr_t GameAssembly = (uintptr_t)GetModuleHandle("GameAssembly.dll");
uintptr_t GetStoryMode = 0x0;
uintptr_t StoryMode = 0x0;
uintptr_t OneScene = 0x0;
uintptr_t SetScene = 0x0;
uintptr_t SceneOpen = 0x0;
uintptr_t LevelCleared = 0x0;
uintptr_t CheckPrice = 0x0;

HANDLE g_console = nullptr;
WORD g_colorInfo = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY;
WORD g_colorOk = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
WORD g_colorWarn = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
WORD g_colorError = FOREGROUND_RED | FOREGROUND_INTENSITY;
WORD g_colorOffset = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
std::unordered_set<uintptr_t> g_hookedTargets;

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
    g_console = GetStdHandle(STD_OUTPUT_HANDLE);
    if (g_console) SetConsoleTextAttribute(g_console, g_colorInfo);
}

static void Init() {
    MH_Initialize();
    CreateConsole();
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

bool(__fastcall* getStoryModeEnabled_o)(DWORD*, DWORD*);
bool __stdcall getStoryModeEnabled(DWORD* __this, DWORD* method) {
    printf("getStoryModeEnabled called\n");
    return false;
}

bool(__fastcall* StoryModeEnabled_o)(DWORD*);
bool __stdcall StoryModeEnabled(DWORD* method) {
    return false;
}

bool(__fastcall* CheckOneOfScenariosUnlocked_o)(DWORD*, DWORD*, DWORD*);
bool(__fastcall* CheckSetOfScenariosUnlocked_o)(DWORD*, DWORD*, DWORD*);
bool __stdcall CheckScenariosUnlocked(DWORD* __this, DWORD* scenes, DWORD* method) {
    printf("CheckScenariosUnlocked called\n");
    return true;
}

bool(__fastcall* isSceneOpened_o)(DWORD*, DWORD*, DWORD*, DWORD*);
bool __stdcall isSceneOpened(DWORD* __this, DWORD* novelId, DWORD* scenario, DWORD* method) {
    printf("isSceneOpened called\n");
    return true;
}

bool(__fastcall* isLevelCleared_o)(DWORD*, DWORD*);
bool __stdcall isLevelCleared(DWORD* __this, DWORD* method) {
    printf("isLevelCompleted called\n");
    return true;
}

bool(__fastcall* isEnoughMoney_o)(DWORD*, DWORD*, DWORD*, DWORD*, DWORD*);
bool __stdcall isEnoughMoney(DWORD* __this, DWORD* price, DWORD* showAlert, DWORD* callback, DWORD* method) {
    printf("isEnoughMoney called\n");
    return true;
}

static DWORD WINAPI ThreadMain(LPVOID /*param*/) {
    Init();
    PrintColor(g_colorOk, "Greetings from PotJoke\n");
    PrintColor(g_colorInfo, "Ready to serve master!\n");
    PrintColor(g_colorWarn, "Please do NOT close console\n");
    ResolvePathsRelativeToGameAssembly();
    PrintColor(g_colorInfo, "[Resolver] global-metadata path: %s\n", g_globalMetadataPath.c_str());
    PrintColor(g_colorInfo, "[Resolver] GameAssembly path: %s\n", g_gameAssemblyPath.c_str());
    PrintColor(g_colorInfo, "[Resolver] methodPointers RVA cache/current: 0x%p\n", (void*)g_methodPointersRva);

    auto resolveAndApply = [](const char* fullName, const char* namespase, const char* className, const char* methodName, int paramCount, uintptr_t& target) {
        const char* effectiveNs = namespase;
        const char* effectiveClass = className;
        std::string classBuffer;
        std::string nsBuffer;
        if (namespase[0] == '\0') {
            const char* dot = strrchr(className, '.');
            if (dot != nullptr) {
                nsBuffer.assign(className, (size_t)(dot - className));
                classBuffer.assign(dot + 1);
                effectiveNs = nsBuffer.c_str();
                effectiveClass = classBuffer.c_str();
            }
        }

        uintptr_t resolved = FindMethodOffsetByClassAndMethod(effectiveNs, effectiveClass, methodName, paramCount);
        if (resolved != 0) target = resolved;
        PrintColor(target == 0 ? g_colorError : g_colorOffset, "[Offsets] %s -> 0x%p\n", fullName, (void*)target);
    };

    resolveAndApply("GameController$$GetStoryModeEnabled", "", "GameController", "GetStoryModeEnabled", 0, GetStoryMode);
    resolveAndApply("CustomStoryFunctions$$StoryModeEnabled", "", "CustomStoryFunctions", "StoryModeEnabled", 0, StoryMode);
    resolveAndApply("Controllers.StoryStateUtilities$$CheckOneOfScenariosUnlocked", "Controllers", "StoryStateUtilities", "CheckOneOfScenariosUnlocked", 1, OneScene);
    resolveAndApply("FStoryModel$$CheckSetOfScenariosUnlocked", "", "FStoryModel", "CheckSetOfScenariosUnlocked", 1, SetScene);
    resolveAndApply("Player$$isSceneOpened", "", "Player", "isSceneOpened", 2, SceneOpen);
    resolveAndApply("YMatchThree.Core.LevelGameplay$$IsLevelComplete", "YMatchThree.Core", "LevelGameplay", "IsLevelComplete", 0, LevelCleared);
    resolveAndApply("GameController$$checkCoins", "", "GameController", "checkCoins", 3, CheckPrice);

    CreateHookWithResolvedOffset("GameController$$GetStoryModeEnabled", GetStoryMode, (LPVOID)&getStoryModeEnabled, (LPVOID*)&getStoryModeEnabled_o);
    CreateHookWithResolvedOffset("CustomStoryFunctions$$StoryModeEnabled", StoryMode, (LPVOID)&StoryModeEnabled, (LPVOID*)&StoryModeEnabled_o);
    CreateHookWithResolvedOffset("Controllers.StoryStateUtilities$$CheckOneOfScenariosUnlocked", OneScene, (LPVOID)&CheckScenariosUnlocked, (LPVOID*)&CheckOneOfScenariosUnlocked_o);
    CreateHookWithResolvedOffset("FStoryModel$$CheckSetOfScenariosUnlocked", SetScene, (LPVOID)&CheckScenariosUnlocked, (LPVOID*)&CheckSetOfScenariosUnlocked_o);
    CreateHookWithResolvedOffset("Player$$isSceneOpened", SceneOpen, (LPVOID)&isSceneOpened, (LPVOID*)&isSceneOpened_o);
    CreateHookWithResolvedOffset("YMatchThree.Core.LevelGameplay$$IsLevelComplete", LevelCleared, (LPVOID)&isLevelCleared, (LPVOID*)&isLevelCleared_o);
    CreateHookWithResolvedOffset("GameController$$checkCoins", CheckPrice, (LPVOID)&isEnoughMoney, (LPVOID*)&isEnoughMoney_o);

    MH_EnableHook(MH_ALL_HOOKS);
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
