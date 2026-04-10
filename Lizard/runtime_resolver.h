#pragma once

#include <Windows.h>
#include <cstdint>

#include "il2cpp_resolver.h"
#include "mono_resolver.h"

enum class GameRuntimeMode {
    Unknown = 0,
    Il2Cpp,
    Mono,
};

static GameRuntimeMode DetectGameRuntimeMode() {
    if (GetModuleHandleA("GameAssembly.dll")) return GameRuntimeMode::Il2Cpp;
    if (GetMonoModuleHandle()) return GameRuntimeMode::Mono;
    return GameRuntimeMode::Unknown;
}

static const char* RuntimeModeToString(GameRuntimeMode mode) {
    switch (mode) {
    case GameRuntimeMode::Il2Cpp: return "Il2CPP";
    case GameRuntimeMode::Mono: return "Mono";
    default: return "Unknown";
    }
}

static uintptr_t ResolveMethodAddressByRuntime(
    GameRuntimeMode mode,
    const char* namespaze,
    const char* className,
    const char* methodName,
    int paramCount
) {
    if (mode == GameRuntimeMode::Il2Cpp) {
        HMODULE gameAssembly = GetModuleHandleA("GameAssembly.dll");
        if (!gameAssembly) return 0;
        uintptr_t rva = FindMethodOffsetByClassAndMethod(namespaze, className, methodName, paramCount);
        if (!rva) return 0;
        return (uintptr_t)gameAssembly + rva;
    }

    if (mode == GameRuntimeMode::Mono) {
        return FindMonoMethodAddressByClassAndMethod(namespaze, className, methodName, paramCount);
    }

    return 0;
}
