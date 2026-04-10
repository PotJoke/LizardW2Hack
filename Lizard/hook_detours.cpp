#include "pch.h"
#include "hook_config.h"

HookSlotRuntime g_hook_slots[kMaxHookSlots];
void* g_hook_originals[kMaxHookSlots];

static void PrintLogLine(const HookSlotRuntime& c) {
    if (c.log_line.empty()) return;
    LizardLogRawUtf8(c.log_line.c_str());
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

#define DEF_MONO_0(N) \
    static bool __stdcall Detour_Mono0_##N() { \
        const auto& c = g_hook_slots[N]; \
        if (!c.enabled && g_hook_originals[N]) { \
            return ((bool(__fastcall*)())g_hook_originals[N])(); \
        } \
        PrintLogLine(c); \
        return c.return_bool; \
    }

#define DEF_MONO_1(N) \
    static bool __stdcall Detour_Mono1_##N(DWORD* a1) { \
        const auto& c = g_hook_slots[N]; \
        if (!c.enabled && g_hook_originals[N]) { \
            return ((bool(__fastcall*)(DWORD*))g_hook_originals[N])(a1); \
        } \
        PrintLogLine(c); \
        return c.return_bool; \
    }

#define DEF_MONO_2(N) \
    static bool __stdcall Detour_Mono2_##N(DWORD* a1, DWORD* a2) { \
        const auto& c = g_hook_slots[N]; \
        if (!c.enabled && g_hook_originals[N]) { \
            return ((bool(__fastcall*)(DWORD*, DWORD*))g_hook_originals[N])(a1, a2); \
        } \
        PrintLogLine(c); \
        return c.return_bool; \
    }

#define DEF_MONO_3(N) \
    static bool __stdcall Detour_Mono3_##N(DWORD* a1, DWORD* a2, DWORD* a3) { \
        const auto& c = g_hook_slots[N]; \
        if (!c.enabled && g_hook_originals[N]) { \
            return ((bool(__fastcall*)(DWORD*, DWORD*, DWORD*))g_hook_originals[N])(a1, a2, a3); \
        } \
        PrintLogLine(c); \
        return c.return_bool; \
    }

#define DEF_MONO_4(N) \
    static bool __stdcall Detour_Mono4_##N(DWORD* a1, DWORD* a2, DWORD* a3, DWORD* a4) { \
        const auto& c = g_hook_slots[N]; \
        if (!c.enabled && g_hook_originals[N]) { \
            return ((bool(__fastcall*)(DWORD*, DWORD*, DWORD*, DWORD*))g_hook_originals[N])(a1, a2, a3, a4); \
        } \
        PrintLogLine(c); \
        return c.return_bool; \
    }

#define DEF_MONO_5(N) \
    static bool __stdcall Detour_Mono5_##N(DWORD* a1, DWORD* a2, DWORD* a3, DWORD* a4, DWORD* a5) { \
        const auto& c = g_hook_slots[N]; \
        if (!c.enabled && g_hook_originals[N]) { \
            return ((bool(__fastcall*)(DWORD*, DWORD*, DWORD*, DWORD*, DWORD*))g_hook_originals[N])(a1, a2, a3, a4, a5); \
        } \
        PrintLogLine(c); \
        return c.return_bool; \
    }

#define DEF_ALL_MONO(N) \
    DEF_MONO_0(N) \
    DEF_MONO_1(N) \
    DEF_MONO_2(N) \
    DEF_MONO_3(N) \
    DEF_MONO_4(N) \
    DEF_MONO_5(N)

DEF_ALL_MONO(0)
DEF_ALL_MONO(1)
DEF_ALL_MONO(2)
DEF_ALL_MONO(3)
DEF_ALL_MONO(4)
DEF_ALL_MONO(5)
DEF_ALL_MONO(6)
DEF_ALL_MONO(7)
DEF_ALL_MONO(8)
DEF_ALL_MONO(9)
DEF_ALL_MONO(10)
DEF_ALL_MONO(11)
DEF_ALL_MONO(12)
DEF_ALL_MONO(13)
DEF_ALL_MONO(14)
DEF_ALL_MONO(15)
DEF_ALL_MONO(16)
DEF_ALL_MONO(17)
DEF_ALL_MONO(18)
DEF_ALL_MONO(19)
DEF_ALL_MONO(20)
DEF_ALL_MONO(21)
DEF_ALL_MONO(22)
DEF_ALL_MONO(23)
DEF_ALL_MONO(24)
DEF_ALL_MONO(25)
DEF_ALL_MONO(26)
DEF_ALL_MONO(27)
DEF_ALL_MONO(28)
DEF_ALL_MONO(29)
DEF_ALL_MONO(30)
DEF_ALL_MONO(31)

#define PTR_M0(n) (void*)&Detour_Mono0_##n
#define PTR_M1(n) (void*)&Detour_Mono1_##n
#define PTR_M2(n) (void*)&Detour_Mono2_##n
#define PTR_M3(n) (void*)&Detour_Mono3_##n
#define PTR_M4(n) (void*)&Detour_Mono4_##n
#define PTR_M5(n) (void*)&Detour_Mono5_##n

static void* const kTable_Mono0[32] = {
    PTR_M0(0), PTR_M0(1), PTR_M0(2), PTR_M0(3), PTR_M0(4), PTR_M0(5), PTR_M0(6), PTR_M0(7),
    PTR_M0(8), PTR_M0(9), PTR_M0(10), PTR_M0(11), PTR_M0(12), PTR_M0(13), PTR_M0(14), PTR_M0(15),
    PTR_M0(16), PTR_M0(17), PTR_M0(18), PTR_M0(19), PTR_M0(20), PTR_M0(21), PTR_M0(22), PTR_M0(23),
    PTR_M0(24), PTR_M0(25), PTR_M0(26), PTR_M0(27), PTR_M0(28), PTR_M0(29), PTR_M0(30), PTR_M0(31),
};
static void* const kTable_Mono1[32] = {
    PTR_M1(0), PTR_M1(1), PTR_M1(2), PTR_M1(3), PTR_M1(4), PTR_M1(5), PTR_M1(6), PTR_M1(7),
    PTR_M1(8), PTR_M1(9), PTR_M1(10), PTR_M1(11), PTR_M1(12), PTR_M1(13), PTR_M1(14), PTR_M1(15),
    PTR_M1(16), PTR_M1(17), PTR_M1(18), PTR_M1(19), PTR_M1(20), PTR_M1(21), PTR_M1(22), PTR_M1(23),
    PTR_M1(24), PTR_M1(25), PTR_M1(26), PTR_M1(27), PTR_M1(28), PTR_M1(29), PTR_M1(30), PTR_M1(31),
};
static void* const kTable_Mono2[32] = {
    PTR_M2(0), PTR_M2(1), PTR_M2(2), PTR_M2(3), PTR_M2(4), PTR_M2(5), PTR_M2(6), PTR_M2(7),
    PTR_M2(8), PTR_M2(9), PTR_M2(10), PTR_M2(11), PTR_M2(12), PTR_M2(13), PTR_M2(14), PTR_M2(15),
    PTR_M2(16), PTR_M2(17), PTR_M2(18), PTR_M2(19), PTR_M2(20), PTR_M2(21), PTR_M2(22), PTR_M2(23),
    PTR_M2(24), PTR_M2(25), PTR_M2(26), PTR_M2(27), PTR_M2(28), PTR_M2(29), PTR_M2(30), PTR_M2(31),
};
static void* const kTable_Mono3[32] = {
    PTR_M3(0), PTR_M3(1), PTR_M3(2), PTR_M3(3), PTR_M3(4), PTR_M3(5), PTR_M3(6), PTR_M3(7),
    PTR_M3(8), PTR_M3(9), PTR_M3(10), PTR_M3(11), PTR_M3(12), PTR_M3(13), PTR_M3(14), PTR_M3(15),
    PTR_M3(16), PTR_M3(17), PTR_M3(18), PTR_M3(19), PTR_M3(20), PTR_M3(21), PTR_M3(22), PTR_M3(23),
    PTR_M3(24), PTR_M3(25), PTR_M3(26), PTR_M3(27), PTR_M3(28), PTR_M3(29), PTR_M3(30), PTR_M3(31),
};
static void* const kTable_Mono4[32] = {
    PTR_M4(0), PTR_M4(1), PTR_M4(2), PTR_M4(3), PTR_M4(4), PTR_M4(5), PTR_M4(6), PTR_M4(7),
    PTR_M4(8), PTR_M4(9), PTR_M4(10), PTR_M4(11), PTR_M4(12), PTR_M4(13), PTR_M4(14), PTR_M4(15),
    PTR_M4(16), PTR_M4(17), PTR_M4(18), PTR_M4(19), PTR_M4(20), PTR_M4(21), PTR_M4(22), PTR_M4(23),
    PTR_M4(24), PTR_M4(25), PTR_M4(26), PTR_M4(27), PTR_M4(28), PTR_M4(29), PTR_M4(30), PTR_M4(31),
};
static void* const kTable_Mono5[32] = {
    PTR_M5(0), PTR_M5(1), PTR_M5(2), PTR_M5(3), PTR_M5(4), PTR_M5(5), PTR_M5(6), PTR_M5(7),
    PTR_M5(8), PTR_M5(9), PTR_M5(10), PTR_M5(11), PTR_M5(12), PTR_M5(13), PTR_M5(14), PTR_M5(15),
    PTR_M5(16), PTR_M5(17), PTR_M5(18), PTR_M5(19), PTR_M5(20), PTR_M5(21), PTR_M5(22), PTR_M5(23),
    PTR_M5(24), PTR_M5(25), PTR_M5(26), PTR_M5(27), PTR_M5(28), PTR_M5(29), PTR_M5(30), PTR_M5(31),
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

void* GetMonoDetourForTotalArgs(int totalArgs, int slot) {
    if (slot < 0 || slot >= kMaxHookSlots) return nullptr;
    switch (totalArgs) {
    case 0: return kTable_Mono0[slot];
    case 1: return kTable_Mono1[slot];
    case 2: return kTable_Mono2[slot];
    case 3: return kTable_Mono3[slot];
    case 4: return kTable_Mono4[slot];
    case 5: return kTable_Mono5[slot];
    default: return nullptr;
    }
}
