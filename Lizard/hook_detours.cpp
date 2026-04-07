#include "pch.h"
#include "hook_config.h"

HookSlotRuntime g_hook_slots[kMaxHookSlots];
void* g_hook_originals[kMaxHookSlots];

static void PrintLogLine(const HookSlotRuntime& c) {
    if (c.log_line.empty()) return;
    printf("%s", c.log_line.c_str());
    if (c.log_line.back() != '\n') printf("\n");
}

#define DEF_METHOD(N) \
    static bool __stdcall Detour_Method_##N(DWORD* method) { \
        const auto& c = g_hook_slots[N]; \
        if (!c.enabled && g_hook_originals[N]) { \
            return ((bool(__fastcall*)(DWORD*))g_hook_originals[N])(method); \
        } \
        PrintLogLine(c); \
        return c.return_bool; \
    }

#define DEF_THIS_METHOD(N) \
    static bool __stdcall Detour_ThisMethod_##N(DWORD* __this, DWORD* method) { \
        const auto& c = g_hook_slots[N]; \
        if (!c.enabled && g_hook_originals[N]) { \
            return ((bool(__fastcall*)(DWORD*, DWORD*))g_hook_originals[N])(__this, method); \
        } \
        PrintLogLine(c); \
        return c.return_bool; \
    }

#define DEF_THIS_SCENES(N) \
    static bool __stdcall Detour_ThisScenes_##N(DWORD* __this, DWORD* scenes, DWORD* method) { \
        const auto& c = g_hook_slots[N]; \
        if (!c.enabled && g_hook_originals[N]) { \
            return ((bool(__fastcall*)(DWORD*, DWORD*, DWORD*))g_hook_originals[N])(__this, scenes, method); \
        } \
        PrintLogLine(c); \
        return c.return_bool; \
    }

#define DEF_THIS_4(N) \
    static bool __stdcall Detour_This4_##N(DWORD* a, DWORD* b, DWORD* c, DWORD* method) { \
        const auto& slot = g_hook_slots[N]; \
        if (!slot.enabled && g_hook_originals[N]) { \
            return ((bool(__fastcall*)(DWORD*, DWORD*, DWORD*, DWORD*))g_hook_originals[N])(a, b, c, method); \
        } \
        PrintLogLine(slot); \
        return slot.return_bool; \
    }

#define DEF_THIS_5(N) \
    static bool __stdcall Detour_This5_##N(DWORD* __this, DWORD* price, DWORD* showAlert, DWORD* callback, DWORD* method) { \
        const auto& c = g_hook_slots[N]; \
        if (!c.enabled && g_hook_originals[N]) { \
            return ((bool(__fastcall*)(DWORD*, DWORD*, DWORD*, DWORD*, DWORD*))g_hook_originals[N])(__this, price, showAlert, callback, method); \
        } \
        PrintLogLine(c); \
        return c.return_bool; \
    }

#define DEF_ALL(N) \
    DEF_METHOD(N) \
    DEF_THIS_METHOD(N) \
    DEF_THIS_SCENES(N) \
    DEF_THIS_4(N) \
    DEF_THIS_5(N)

DEF_ALL(0)
DEF_ALL(1)
DEF_ALL(2)
DEF_ALL(3)
DEF_ALL(4)
DEF_ALL(5)
DEF_ALL(6)
DEF_ALL(7)
DEF_ALL(8)
DEF_ALL(9)
DEF_ALL(10)
DEF_ALL(11)
DEF_ALL(12)
DEF_ALL(13)
DEF_ALL(14)
DEF_ALL(15)
DEF_ALL(16)
DEF_ALL(17)
DEF_ALL(18)
DEF_ALL(19)
DEF_ALL(20)
DEF_ALL(21)
DEF_ALL(22)
DEF_ALL(23)
DEF_ALL(24)
DEF_ALL(25)
DEF_ALL(26)
DEF_ALL(27)
DEF_ALL(28)
DEF_ALL(29)
DEF_ALL(30)
DEF_ALL(31)

#define PTR_M(n) (void*)&Detour_Method_##n
#define PTR_TM(n) (void*)&Detour_ThisMethod_##n
#define PTR_TS(n) (void*)&Detour_ThisScenes_##n
#define PTR_T4(n) (void*)&Detour_This4_##n
#define PTR_T5(n) (void*)&Detour_This5_##n

static void* const kTable_Method[32] = {
    PTR_M(0),  PTR_M(1),  PTR_M(2),  PTR_M(3),  PTR_M(4),  PTR_M(5),  PTR_M(6),  PTR_M(7),
    PTR_M(8),  PTR_M(9),  PTR_M(10), PTR_M(11), PTR_M(12), PTR_M(13), PTR_M(14), PTR_M(15),
    PTR_M(16), PTR_M(17), PTR_M(18), PTR_M(19), PTR_M(20), PTR_M(21), PTR_M(22), PTR_M(23),
    PTR_M(24), PTR_M(25), PTR_M(26), PTR_M(27), PTR_M(28), PTR_M(29), PTR_M(30), PTR_M(31),
};

static void* const kTable_ThisMethod[32] = {
    PTR_TM(0),  PTR_TM(1),  PTR_TM(2),  PTR_TM(3),  PTR_TM(4),  PTR_TM(5),  PTR_TM(6),  PTR_TM(7),
    PTR_TM(8),  PTR_TM(9),  PTR_TM(10), PTR_TM(11), PTR_TM(12), PTR_TM(13), PTR_TM(14), PTR_TM(15),
    PTR_TM(16), PTR_TM(17), PTR_TM(18), PTR_TM(19), PTR_TM(20), PTR_TM(21), PTR_TM(22), PTR_TM(23),
    PTR_TM(24), PTR_TM(25), PTR_TM(26), PTR_TM(27), PTR_TM(28), PTR_TM(29), PTR_TM(30), PTR_TM(31),
};

static void* const kTable_ThisScenes[32] = {
    PTR_TS(0),  PTR_TS(1),  PTR_TS(2),  PTR_TS(3),  PTR_TS(4),  PTR_TS(5),  PTR_TS(6),  PTR_TS(7),
    PTR_TS(8),  PTR_TS(9),  PTR_TS(10), PTR_TS(11), PTR_TS(12), PTR_TS(13), PTR_TS(14), PTR_TS(15),
    PTR_TS(16), PTR_TS(17), PTR_TS(18), PTR_TS(19), PTR_TS(20), PTR_TS(21), PTR_TS(22), PTR_TS(23),
    PTR_TS(24), PTR_TS(25), PTR_TS(26), PTR_TS(27), PTR_TS(28), PTR_TS(29), PTR_TS(30), PTR_TS(31),
};

static void* const kTable_This4[32] = {
    PTR_T4(0),  PTR_T4(1),  PTR_T4(2),  PTR_T4(3),  PTR_T4(4),  PTR_T4(5),  PTR_T4(6),  PTR_T4(7),
    PTR_T4(8),  PTR_T4(9),  PTR_T4(10), PTR_T4(11), PTR_T4(12), PTR_T4(13), PTR_T4(14), PTR_T4(15),
    PTR_T4(16), PTR_T4(17), PTR_T4(18), PTR_T4(19), PTR_T4(20), PTR_T4(21), PTR_T4(22), PTR_T4(23),
    PTR_T4(24), PTR_T4(25), PTR_T4(26), PTR_T4(27), PTR_T4(28), PTR_T4(29), PTR_T4(30), PTR_T4(31),
};

static void* const kTable_This5[32] = {
    PTR_T5(0),  PTR_T5(1),  PTR_T5(2),  PTR_T5(3),  PTR_T5(4),  PTR_T5(5),  PTR_T5(6),  PTR_T5(7),
    PTR_T5(8),  PTR_T5(9),  PTR_T5(10), PTR_T5(11), PTR_T5(12), PTR_T5(13), PTR_T5(14), PTR_T5(15),
    PTR_T5(16), PTR_T5(17), PTR_T5(18), PTR_T5(19), PTR_T5(20), PTR_T5(21), PTR_T5(22), PTR_T5(23),
    PTR_T5(24), PTR_T5(25), PTR_T5(26), PTR_T5(27), PTR_T5(28), PTR_T5(29), PTR_T5(30), PTR_T5(31),
};

void* GetDetourForSignature(HookSignature sig, int slot) {
    if (slot < 0 || slot >= kMaxHookSlots) return nullptr;
    switch (sig) {
    case HookSignature::BoolMethod: return kTable_Method[slot];
    case HookSignature::BoolThisMethod: return kTable_ThisMethod[slot];
    case HookSignature::BoolThisScenes: return kTable_ThisScenes[slot];
    case HookSignature::BoolThis4: return kTable_This4[slot];
    case HookSignature::BoolThis5: return kTable_This5[slot];
    default: return kTable_ThisMethod[slot];
    }
}
