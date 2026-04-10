#pragma once

#include <Windows.h>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "lizard_log.h"

typedef void* (*mono_get_root_domain_t)();
typedef void* (*mono_thread_attach_t)(void* domain);
typedef void (*mono_assembly_foreach_t)(void (*func)(void* assembly, void* user_data), void* user_data);
typedef void* (*mono_assembly_get_image_t)(void* assembly);
typedef const char* (*mono_image_get_name_t)(void* image);
typedef void* (*mono_class_from_name_t)(void* image, const char* namespaze, const char* name);
typedef void* (*mono_class_get_methods_t)(void* klass, void** iter);
typedef const char* (*mono_method_get_name_t)(void* method);
typedef void* (*mono_method_signature_t)(void* method);
typedef uint32_t (*mono_signature_get_param_count_t)(void* signature);
typedef void* (*mono_compile_method_t)(void* method);
typedef uint32_t (*mono_method_get_flags_t)(void* method, uint32_t* iflags);
typedef void* (*mono_signature_get_return_type_t)(void* signature);
typedef int (*mono_type_get_type_t)(void* type);

static HMODULE GetMonoModuleHandle() {
    static const char* kMonoModuleNames[] = {
        "mono-2.0-bdwgc.dll",
        "mono-2.0-sgen.dll",
        "mono.dll",
        "MonoBleedingEdge\\EmbedRuntime\\mono-2.0-bdwgc.dll",
    };
    for (const char* name : kMonoModuleNames) {
        HMODULE h = GetModuleHandleA(name);
        if (h) return h;
    }
    return nullptr;
}

struct MonoApi {
    mono_get_root_domain_t get_root_domain = nullptr;
    mono_thread_attach_t thread_attach = nullptr;
    mono_assembly_foreach_t assembly_foreach = nullptr;
    mono_assembly_get_image_t assembly_get_image = nullptr;
    mono_image_get_name_t image_get_name = nullptr;
    mono_class_from_name_t class_from_name = nullptr;
    mono_class_get_methods_t class_get_methods = nullptr;
    mono_method_get_name_t method_get_name = nullptr;
    mono_method_signature_t method_signature = nullptr;
    mono_signature_get_param_count_t signature_get_param_count = nullptr;
    mono_compile_method_t compile_method = nullptr;
    mono_method_get_flags_t method_get_flags = nullptr;
    mono_signature_get_return_type_t signature_get_return_type = nullptr;
    mono_type_get_type_t type_get_type = nullptr;
};

static bool ResolveMonoApi(MonoApi& api) {
    HMODULE mono = GetMonoModuleHandle();
    if (!mono) return false;

    api.get_root_domain = (mono_get_root_domain_t)GetProcAddress(mono, "mono_get_root_domain");
    api.thread_attach = (mono_thread_attach_t)GetProcAddress(mono, "mono_thread_attach");
    api.assembly_foreach = (mono_assembly_foreach_t)GetProcAddress(mono, "mono_assembly_foreach");
    api.assembly_get_image = (mono_assembly_get_image_t)GetProcAddress(mono, "mono_assembly_get_image");
    api.image_get_name = (mono_image_get_name_t)GetProcAddress(mono, "mono_image_get_name");
    api.class_from_name = (mono_class_from_name_t)GetProcAddress(mono, "mono_class_from_name");
    api.class_get_methods = (mono_class_get_methods_t)GetProcAddress(mono, "mono_class_get_methods");
    api.method_get_name = (mono_method_get_name_t)GetProcAddress(mono, "mono_method_get_name");
    api.method_signature = (mono_method_signature_t)GetProcAddress(mono, "mono_method_signature");
    api.signature_get_param_count = (mono_signature_get_param_count_t)GetProcAddress(mono, "mono_signature_get_param_count");
    api.compile_method = (mono_compile_method_t)GetProcAddress(mono, "mono_compile_method");
    api.method_get_flags = (mono_method_get_flags_t)GetProcAddress(mono, "mono_method_get_flags");
    api.signature_get_return_type = (mono_signature_get_return_type_t)GetProcAddress(mono, "mono_signature_get_return_type");
    api.type_get_type = (mono_type_get_type_t)GetProcAddress(mono, "mono_type_get_type");

    return api.get_root_domain &&
        api.thread_attach &&
        api.assembly_foreach &&
        api.assembly_get_image &&
        api.class_from_name &&
        api.class_get_methods &&
        api.method_get_name &&
        api.method_signature &&
        api.signature_get_param_count &&
        api.compile_method &&
        api.method_get_flags &&
        api.signature_get_return_type &&
        api.type_get_type;
}

struct MonoAssemblyListCtx {
    std::vector<void*> assemblies;
};

static void __cdecl MonoCollectAssembly(void* assembly, void* user_data) {
    if (!assembly || !user_data) return;
    MonoAssemblyListCtx* ctx = (MonoAssemblyListCtx*)user_data;
    ctx->assemblies.push_back(assembly);
}

static bool EnsureMonoThreadAttached(const MonoApi& api) {
    static bool s_attached = false;
    if (s_attached) return true;
    if (!api.get_root_domain || !api.thread_attach) return false;
    void* root = api.get_root_domain();
    if (!root) return false;
    if (!api.thread_attach(root)) return false;
    s_attached = true;
    return true;
}

static uintptr_t FindMonoMethodAddressByClassAndMethod(
    const char* namespaze,
    const char* className,
    const char* methodName,
    int paramCount // -1 means any overload
) {
    MonoApi api{};
    if (!ResolveMonoApi(api)) {
        LizardResolverLogf("[MonoResolver] Required mono exports are unavailable.\n");
        return 0;
    }
    if (!EnsureMonoThreadAttached(api)) {
        LizardResolverLogf("[MonoResolver] mono_thread_attach failed.\n");
        return 0;
    }

    MonoAssemblyListCtx ctx;
    api.assembly_foreach(&MonoCollectAssembly, &ctx);
    if (ctx.assemblies.empty()) {
        LizardResolverLogf("[MonoResolver] mono_assembly_foreach returned no assemblies.\n");
        return 0;
    }

    for (void* assembly : ctx.assemblies) {
        void* image = api.assembly_get_image(assembly);
        if (!image) continue;

        void* klass = api.class_from_name(image, namespaze ? namespaze : "", className ? className : "");
        if (!klass) continue;

        void* iter = nullptr;
        while (void* method = api.class_get_methods(klass, &iter)) {
            const char* currentName = api.method_get_name(method);
            if (!currentName || strcmp(currentName, methodName) != 0) continue;

            void* sig = api.method_signature(method);
            if (!sig) continue;
            uint32_t currentParamCount = api.signature_get_param_count(sig);
            if (paramCount >= 0 && currentParamCount != (uint32_t)paramCount) continue;

            void* compiled = api.compile_method(method);
            if (compiled) {
                return (uintptr_t)compiled;
            }
        }
    }

    LizardResolverLogf("[MonoResolver] Method not found: %s.%s::%s\n",
        namespaze ? namespaze : "",
        className ? className : "",
        methodName ? methodName : "");
    return 0;
}

struct MonoResolvedMethodInfo {
    uintptr_t address = 0;
    int managedParamCount = 0;
    bool isStatic = false;
    bool returnsBool = false;
    bool found = false;
};

static MonoResolvedMethodInfo ResolveMonoMethodForHook(
    const char* namespaze,
    const char* className,
    const char* methodName,
    int paramCount // -1 means any overload
) {
    MonoResolvedMethodInfo out{};
    MonoApi api{};
    if (!ResolveMonoApi(api)) return out;
    if (!EnsureMonoThreadAttached(api)) return out;

    // Mono type enum: BOOLEAN = 0x02
    const int MONO_TYPE_BOOLEAN = 0x02;
    const uint32_t METHOD_ATTRIBUTE_STATIC = 0x0010;

    MonoAssemblyListCtx ctx;
    api.assembly_foreach(&MonoCollectAssembly, &ctx);
    for (void* assembly : ctx.assemblies) {
        void* image = api.assembly_get_image(assembly);
        if (!image) continue;
        void* klass = api.class_from_name(image, namespaze ? namespaze : "", className ? className : "");
        if (!klass) continue;

        void* iter = nullptr;
        while (void* method = api.class_get_methods(klass, &iter)) {
            const char* currentName = api.method_get_name(method);
            if (!currentName || strcmp(currentName, methodName) != 0) continue;

            void* sig = api.method_signature(method);
            if (!sig) continue;
            uint32_t currentParamCount = api.signature_get_param_count(sig);
            if (paramCount >= 0 && currentParamCount != (uint32_t)paramCount) continue;

            uint32_t iflags = 0;
            uint32_t flags = api.method_get_flags(method, &iflags);
            bool isStatic = (flags & METHOD_ATTRIBUTE_STATIC) != 0;

            bool returnsBool = false;
            if (void* rt = api.signature_get_return_type(sig)) {
                returnsBool = (api.type_get_type(rt) == MONO_TYPE_BOOLEAN);
            }

            void* compiled = api.compile_method(method);
            if (!compiled) continue;

            out.address = (uintptr_t)compiled;
            out.managedParamCount = (int)currentParamCount;
            out.isStatic = isStatic;
            out.returnsBool = returnsBool;
            out.found = true;
            return out;
        }
    }
    return out;
}
