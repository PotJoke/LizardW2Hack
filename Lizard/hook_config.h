#pragma once

#include <string>
#include <vector>

enum class HookSignature {
    BoolMethod,
    BoolThisMethod,
    BoolThisScenes,
    BoolThis4,
    BoolThis5,
};

struct HookDefinition {
    std::string display_name;
    std::string full_name;
    std::string namespaze;
    std::string class_name;
    std::string method_name;
    int param_count = 0;
    HookSignature signature = HookSignature::BoolThisMethod;
    bool enabled_on_start = true;
    bool return_bool = false;
    std::string log_line;
};

struct HookSlotRuntime {
    std::string display_name;
    std::string full_name;
    bool enabled = true;
    std::string log_line;
    bool return_bool = false;
};

constexpr int kMaxHookSlots = 32;

extern HookSlotRuntime g_hook_slots[kMaxHookSlots];
extern void* g_hook_originals[kMaxHookSlots];

// Loads UTF-8 hooks from a .txt next to the game executable (GetModuleHandle(NULL)).
// Default file name: LizardHooks.txt in the same folder as the .exe.
// Override full path with env var LIZARD_HOOKS_FILE (wide path).
bool LoadGameHookDefinitions(std::vector<HookDefinition>& out_defs, std::wstring& out_config_path);

void SplitIl2CppFullName(const std::string& full_name, std::string& namespaze, std::string& class_name, std::string& method_name);

HookSignature InferSignatureFromParamList(const std::string& params_inside_parens);

void* GetDetourForSignature(HookSignature sig, int slot);
