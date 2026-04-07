#pragma once

#include <Windows.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

static std::string g_globalMetadataPath = "";
static std::string g_gameAssemblyPath = "";
static uintptr_t g_methodPointersRva = 0x0;
static bool g_pathsResolved = false;
static bool g_pathsResolveTried = false;

typedef void* (*il2cpp_domain_get_t)();
typedef void (*il2cpp_domain_get_assemblies_t)(void* domain, const void*** assemblies, size_t* size);
typedef void* (*il2cpp_assembly_get_image_t)(const void* assembly);
typedef void* (*il2cpp_class_from_name_t)(void* image, const char* namespaze, const char* name);
typedef void* (*il2cpp_class_get_methods_t)(void* klass, void** iter);
typedef const char* (*il2cpp_method_get_name_t)(void* method);
typedef uint32_t (*il2cpp_method_get_param_count_t)(void* method);
typedef void* (*il2cpp_method_get_pointer_t)(void* method);

#pragma pack(push, 1)
struct Il2CppGlobalMetadataHeader {
    uint32_t sanity;
    int32_t version;
    uint32_t stringLiteralOffset;
    int32_t stringLiteralCount;
    uint32_t stringLiteralDataOffset;
    int32_t stringLiteralDataCount;
    uint32_t stringOffset;
    int32_t stringCount;
    uint32_t eventsOffset;
    int32_t eventsCount;
    uint32_t propertiesOffset;
    int32_t propertiesCount;
    uint32_t methodsOffset;
    int32_t methodsCount;
    uint32_t parameterDefaultValuesOffset;
    int32_t parameterDefaultValuesCount;
    uint32_t fieldDefaultValuesOffset;
    int32_t fieldDefaultValuesCount;
    uint32_t fieldAndParameterDefaultValueDataOffset;
    int32_t fieldAndParameterDefaultValueDataCount;
    uint32_t fieldMarshaledSizesOffset;
    int32_t fieldMarshaledSizesCount;
    uint32_t parametersOffset;
    int32_t parametersCount;
    uint32_t fieldsOffset;
    int32_t fieldsCount;
    uint32_t genericParametersOffset;
    int32_t genericParametersCount;
    uint32_t genericParameterConstraintsOffset;
    int32_t genericParameterConstraintsCount;
    uint32_t genericContainersOffset;
    int32_t genericContainersCount;
    uint32_t nestedTypesOffset;
    int32_t nestedTypesCount;
    uint32_t interfacesOffset;
    int32_t interfacesCount;
    uint32_t vtableMethodsOffset;
    int32_t vtableMethodsCount;
    uint32_t interfaceOffsetsOffset;
    int32_t interfaceOffsetsCount;
    uint32_t typeDefinitionsOffset;
    int32_t typeDefinitionsCount;
    uint32_t rgctxEntriesOffset;
    int32_t rgctxEntriesCount;
    uint32_t imagesOffset;
    int32_t imagesCount;
    uint32_t assembliesOffset;
    int32_t assembliesCount;
    uint32_t metadataUsageListsOffset;
    int32_t metadataUsageListsCount;
    uint32_t metadataUsagePairsOffset;
    int32_t metadataUsagePairsCount;
    uint32_t fieldRefsOffset;
    int32_t fieldRefsCount;
    uint32_t referencedAssembliesOffset;
    int32_t referencedAssembliesCount;
    uint32_t attributesInfoOffset;
    int32_t attributesInfoCount;
    uint32_t attributeTypesOffset;
    int32_t attributeTypesCount;
    uint32_t attributeDataOffset;
    int32_t attributeDataSize;
    uint32_t attributeDataRangeOffset;
    int32_t attributeDataRangeSize;
    uint32_t unresolvedVirtualCallParameterTypesOffset;
    int32_t unresolvedVirtualCallParameterTypesCount;
    uint32_t unresolvedVirtualCallParameterRangesOffset;
    int32_t unresolvedVirtualCallParameterRangesCount;
    uint32_t windowsRuntimeTypeNamesOffset;
    int32_t windowsRuntimeTypeNamesSize;
    uint32_t windowsRuntimeStringsOffset;
    int32_t windowsRuntimeStringsSize;
    uint32_t exportedTypeDefinitionsOffset;
    int32_t exportedTypeDefinitionsCount;
};

struct Il2CppMethodDefinitionV27Plus {
    int32_t nameIndex;
    int32_t declaringType;
    int32_t returnType;
    int32_t parameterStart;
    int32_t genericContainerIndex;
    uint32_t token;
    uint16_t flags;
    uint16_t iflags;
    uint16_t slot;
    uint16_t parameterCount;
};

struct Il2CppMethodDefinitionV31 {
    int32_t nameIndex;
    int32_t declaringType;
    int32_t returnType;
    int32_t returnParameterToken;
    int32_t parameterStart;
    int32_t genericContainerIndex;
    uint32_t token;
    uint16_t flags;
    uint16_t iflags;
    uint16_t slot;
    uint16_t parameterCount;
};

struct Il2CppMethodDefinitionV24 {
    uint32_t nameIndex;
    int32_t declaringType;
    int32_t returnType;
    int32_t parameterStart;
    int32_t customAttributeIndex;
    int32_t genericContainerIndex;
    int32_t methodIndex;
    int32_t invokerIndex;
    int32_t delegateWrapperIndex;
    int32_t rgctxStartIndex;
    int32_t rgctxCount;
    uint32_t token;
    uint16_t flags;
    uint16_t iflags;
    uint16_t slot;
    uint16_t parameterCount;
};

struct Il2CppTypeDefinitionV27Plus {
    int32_t nameIndex;
    int32_t namespaceIndex;
    int32_t byvalTypeIndex;
    int32_t byrefTypeIndex;
    int32_t declaringTypeIndex;
    int32_t parentIndex;
    int32_t elementTypeIndex;
    int32_t genericContainerIndex;
    uint32_t flags;
    int32_t fieldStart;
    int32_t methodStart;
    int32_t eventStart;
    int32_t propertyStart;
    int32_t nestedTypesStart;
    int32_t interfacesStart;
    int32_t vtableStart;
    int32_t interfaceOffsetsStart;
    uint16_t method_count;
    uint16_t property_count;
    uint16_t field_count;
    uint16_t event_count;
    uint16_t nested_type_count;
    uint16_t vtable_count;
    uint16_t interfaces_count;
    uint16_t interface_offsets_count;
    uint32_t bitfield;
    uint32_t token;
};

struct Il2CppTypeDefinitionV24 {
    uint32_t nameIndex;
    uint32_t namespaceIndex;
    int32_t customAttributeIndex;
    int32_t byvalTypeIndex;
    int32_t byrefTypeIndex;
    int32_t declaringTypeIndex;
    int32_t parentIndex;
    int32_t elementTypeIndex;
    int32_t rgctxStartIndex;
    int32_t rgctxCount;
    int32_t genericContainerIndex;
    uint32_t flags;
    int32_t fieldStart;
    int32_t methodStart;
    int32_t eventStart;
    int32_t propertyStart;
    int32_t nestedTypesStart;
    int32_t interfacesStart;
    int32_t vtableStart;
    int32_t interfaceOffsetsStart;
    uint16_t method_count;
    uint16_t property_count;
    uint16_t field_count;
    uint16_t event_count;
    uint16_t nested_type_count;
    uint16_t vtable_count;
    uint16_t interfaces_count;
    uint16_t interface_offsets_count;
    uint32_t bitfield;
    uint32_t token;
};

struct Il2CppImageDefinitionV24 {
    uint32_t nameIndex;
    int32_t assemblyIndex;
    int32_t typeStart;
    uint32_t typeCount;
    int32_t exportedTypeStart;
    uint32_t exportedTypeCount;
    int32_t entryPointIndex;
    uint32_t token;
    int32_t customAttributeStart;
    uint32_t customAttributeCount;
};
#pragma pack(pop)

static uint64_t GetImageBase(const std::vector<uint8_t>& pe) {
    if (pe.size() < sizeof(IMAGE_DOS_HEADER)) return 0;
    const IMAGE_DOS_HEADER* dos = (const IMAGE_DOS_HEADER*)pe.data();
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) return 0;
    if ((size_t)dos->e_lfanew + sizeof(IMAGE_NT_HEADERS64) > pe.size()) return 0;
    const IMAGE_NT_HEADERS64* nt = (const IMAGE_NT_HEADERS64*)(pe.data() + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) return 0;
    return nt->OptionalHeader.ImageBase;
}

static const char* MetadataString(const std::vector<uint8_t>& metadata, const Il2CppGlobalMetadataHeader& header, int32_t index) {
    if (index < 0) return "";
    uint64_t pos = (uint64_t)header.stringOffset + (uint64_t)index;
    if (pos >= metadata.size()) return "";
    return (const char*)(metadata.data() + pos);
}

static uint32_t RvaToFileOffset(const std::vector<uint8_t>& pe, uint32_t rva) {
    if (pe.size() < sizeof(IMAGE_DOS_HEADER)) return 0;
    const IMAGE_DOS_HEADER* dos = (const IMAGE_DOS_HEADER*)pe.data();
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) return 0;
    if ((size_t)dos->e_lfanew + sizeof(IMAGE_NT_HEADERS64) > pe.size()) return 0;

    const IMAGE_NT_HEADERS64* nt = (const IMAGE_NT_HEADERS64*)(pe.data() + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) return 0;

    const IMAGE_SECTION_HEADER* sections = IMAGE_FIRST_SECTION(nt);
    for (uint16_t i = 0; i < nt->FileHeader.NumberOfSections; ++i) {
        uint32_t va = sections[i].VirtualAddress;
        uint32_t vs = sections[i].Misc.VirtualSize;
        uint32_t raw = sections[i].PointerToRawData;
        uint32_t rawSize = sections[i].SizeOfRawData;
        uint32_t maxSize = (vs > rawSize) ? vs : rawSize;
        if (rva >= va && rva < va + maxSize) return raw + (rva - va);
    }

    return 0;
}

static bool UseV24MetadataLayout(int32_t version) {
    return version <= 24;
}

static bool UseV31MethodLayout(int32_t version) {
    return version >= 31;
}

static size_t TypeDefSizeForVersion(int32_t version) {
    return UseV24MetadataLayout(version) ? sizeof(Il2CppTypeDefinitionV24) : sizeof(Il2CppTypeDefinitionV27Plus);
}

static size_t MethodDefSizeForVersion(int32_t version) {
    if (UseV31MethodLayout(version)) return sizeof(Il2CppMethodDefinitionV31);
    return UseV24MetadataLayout(version) ? sizeof(Il2CppMethodDefinitionV24) : sizeof(Il2CppMethodDefinitionV27Plus);
}

static const uint8_t* GetTypeDefAt(const std::vector<uint8_t>& metadata, const Il2CppGlobalMetadataHeader& header, int32_t version, int32_t index) {
    size_t typeSize = TypeDefSizeForVersion(version);
    uint64_t pos = (uint64_t)header.typeDefinitionsOffset + (uint64_t)index * (uint64_t)typeSize;
    if (pos + typeSize > metadata.size()) return nullptr;
    return metadata.data() + pos;
}

static uint32_t TypeDefNameIndex(const uint8_t* p, int32_t version) {
    if (UseV24MetadataLayout(version)) return ((const Il2CppTypeDefinitionV24*)p)->nameIndex;
    return (uint32_t)((const Il2CppTypeDefinitionV27Plus*)p)->nameIndex;
}

static uint32_t TypeDefNamespaceIndex(const uint8_t* p, int32_t version) {
    if (UseV24MetadataLayout(version)) return ((const Il2CppTypeDefinitionV24*)p)->namespaceIndex;
    return (uint32_t)((const Il2CppTypeDefinitionV27Plus*)p)->namespaceIndex;
}

static int32_t TypeDefMethodStart(const uint8_t* p, int32_t version) {
    if (UseV24MetadataLayout(version)) return ((const Il2CppTypeDefinitionV24*)p)->methodStart;
    return ((const Il2CppTypeDefinitionV27Plus*)p)->methodStart;
}

static uint16_t TypeDefMethodCount(const uint8_t* p, int32_t version) {
    if (UseV24MetadataLayout(version)) return ((const Il2CppTypeDefinitionV24*)p)->method_count;
    return ((const Il2CppTypeDefinitionV27Plus*)p)->method_count;
}

static const uint8_t* GetMethodDefAt(const std::vector<uint8_t>& metadata, const Il2CppGlobalMetadataHeader& header, int32_t version, int32_t index) {
    size_t methodSize = MethodDefSizeForVersion(version);
    uint64_t pos = (uint64_t)header.methodsOffset + (uint64_t)index * (uint64_t)methodSize;
    if (pos + methodSize > metadata.size()) return nullptr;
    return metadata.data() + pos;
}

static uint32_t MethodDefNameIndex(const uint8_t* p, int32_t version) {
    if (UseV31MethodLayout(version)) return (uint32_t)((const Il2CppMethodDefinitionV31*)p)->nameIndex;
    if (UseV24MetadataLayout(version)) return ((const Il2CppMethodDefinitionV24*)p)->nameIndex;
    return (uint32_t)((const Il2CppMethodDefinitionV27Plus*)p)->nameIndex;
}

static uint16_t MethodDefParamCount(const uint8_t* p, int32_t version) {
    if (UseV31MethodLayout(version)) return ((const Il2CppMethodDefinitionV31*)p)->parameterCount;
    if (UseV24MetadataLayout(version)) return ((const Il2CppMethodDefinitionV24*)p)->parameterCount;
    return ((const Il2CppMethodDefinitionV27Plus*)p)->parameterCount;
}

static int32_t MethodDefDeclaringType(const uint8_t* p, int32_t version) {
    if (UseV31MethodLayout(version)) return ((const Il2CppMethodDefinitionV31*)p)->declaringType;
    if (UseV24MetadataLayout(version)) return ((const Il2CppMethodDefinitionV24*)p)->declaringType;
    return ((const Il2CppMethodDefinitionV27Plus*)p)->declaringType;
}

static int32_t MethodDefMethodIndex(const uint8_t* p, int32_t version) {
    if (UseV24MetadataLayout(version)) return ((const Il2CppMethodDefinitionV24*)p)->methodIndex;
    return -1;
}

static uint32_t MethodDefToken(const uint8_t* p, int32_t version) {
    if (UseV31MethodLayout(version)) return ((const Il2CppMethodDefinitionV31*)p)->token;
    if (UseV24MetadataLayout(version)) return ((const Il2CppMethodDefinitionV24*)p)->token;
    return ((const Il2CppMethodDefinitionV27Plus*)p)->token;
}

struct PeSectionRange {
    uint32_t va;
    uint32_t size;
    uint32_t raw;
    uint32_t rawSize;
    uint32_t characteristics;
};

static bool GetPeSections(const std::vector<uint8_t>& pe, std::vector<PeSectionRange>& outSections, uint64_t& outImageBase, uint32_t& outImageSize) {
    if (pe.size() < sizeof(IMAGE_DOS_HEADER)) return false;
    const IMAGE_DOS_HEADER* dos = (const IMAGE_DOS_HEADER*)pe.data();
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) return false;
    if ((size_t)dos->e_lfanew + sizeof(IMAGE_NT_HEADERS64) > pe.size()) return false;

    const IMAGE_NT_HEADERS64* nt = (const IMAGE_NT_HEADERS64*)(pe.data() + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) return false;

    outImageBase = nt->OptionalHeader.ImageBase;
    outImageSize = nt->OptionalHeader.SizeOfImage;

    const IMAGE_SECTION_HEADER* sections = IMAGE_FIRST_SECTION(nt);
    outSections.clear();
    outSections.reserve(nt->FileHeader.NumberOfSections);
    for (uint16_t i = 0; i < nt->FileHeader.NumberOfSections; ++i) {
        PeSectionRange s{};
        s.va = sections[i].VirtualAddress;
        s.size = sections[i].Misc.VirtualSize ? sections[i].Misc.VirtualSize : sections[i].SizeOfRawData;
        s.raw = sections[i].PointerToRawData;
        s.rawSize = sections[i].SizeOfRawData;
        s.characteristics = sections[i].Characteristics;
        outSections.push_back(s);
    }

    return true;
}

static std::string GetImageNameForTypeIndex(
    const std::vector<uint8_t>& metadata,
    const Il2CppGlobalMetadataHeader& header,
    int32_t typeIndex
) {
    if (header.imagesOffset <= 0 || header.imagesCount <= 0) return {};
    int32_t imageCount = header.imagesCount / (int32_t)sizeof(Il2CppImageDefinitionV24);
    if (imageCount <= 0) return {};
    if ((uint64_t)header.imagesOffset + (uint64_t)header.imagesCount > metadata.size()) return {};

    const Il2CppImageDefinitionV24* images = (const Il2CppImageDefinitionV24*)(metadata.data() + header.imagesOffset);
    for (int32_t i = 0; i < imageCount; ++i) {
        int32_t start = images[i].typeStart;
        uint32_t count = images[i].typeCount;
        if (start < 0) continue;
        if (typeIndex >= start && (uint32_t)(typeIndex - start) < count) {
            return std::string(MetadataString(metadata, header, (int32_t)images[i].nameIndex));
        }
    }
    return {};
}

static bool IsExecutableRva(uint32_t rva, const std::vector<PeSectionRange>& sections) {
    for (const auto& s : sections) {
        if ((s.characteristics & IMAGE_SCN_MEM_EXECUTE) == 0) continue;
        if (rva >= s.va && rva < s.va + s.size) return true;
    }
    return false;
}

static bool IsPointerInSectionVa(uint64_t va, const std::vector<PeSectionRange>& sections, uint64_t imageBase) {
    if (va <= imageBase) return false;
    uint32_t rva = (uint32_t)(va - imageBase);
    for (const auto& s : sections) {
        if (rva >= s.va && rva < s.va + s.size) return true;
    }
    return false;
}

static bool ReadAsciiZAtVa(
    const std::vector<uint8_t>& pe,
    uint64_t imageBase,
    uint64_t va,
    std::string& out
) {
    out.clear();
    if (va <= imageBase) return false;
    uint32_t rva = (uint32_t)(va - imageBase);
    uint32_t raw = RvaToFileOffset(pe, rva);
    if (raw == 0 || raw >= pe.size()) return false;
    for (uint32_t i = raw; i < pe.size() && out.size() < 260; ++i) {
        char c = (char)pe[i];
        if (c == '\0') return !out.empty();
        if ((unsigned char)c < 0x20 || (unsigned char)c > 0x7E) return false;
        out.push_back(c);
    }
    return false;
}

static std::string NormalizeModuleName(const std::string& name) {
    std::string out;
    out.reserve(name.size());
    for (char c : name) {
        if (c == '\\' || c == '/') continue;
        out.push_back((char)tolower((unsigned char)c));
    }
    if (out.size() >= 4 && out.substr(out.size() - 4) == ".dll") {
        out.resize(out.size() - 4);
    }
    return out;
}

static bool LooksLikeSystemType(const char* namespaze, const char* className) {
    if (!namespaze) namespaze = "";
    if (!className) className = "";
    if (strncmp(namespaze, "System", 6) == 0) return true;
    if (strncmp(namespaze, "Unity", 5) == 0) return true;
    if (strncmp(className, "System.", 7) == 0) return true;
    return false;
}

static bool FindCodeGenModulesArray(
    const std::vector<uint8_t>& pe,
    uint64_t imageBase,
    const std::vector<PeSectionRange>& sections,
    uint32_t expectedImageCount,
    uint64_t& outModulesArrayVa,
    uint64_t& outModulesCount
) {
    outModulesArrayVa = 0;
    outModulesCount = 0;

    for (const auto& s : sections) {
        bool isData = (s.characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA) != 0;
        if (!isData || (s.characteristics & IMAGE_SCN_MEM_EXECUTE)) continue;
        if (s.raw == 0 || s.rawSize < 0x20) continue;

        uint32_t begin = s.raw;
        uint32_t end = s.raw + s.rawSize;
        for (uint32_t off = begin; off + 16 <= end; off += 8) {
            uint64_t maybeCount = *(const uint64_t*)(pe.data() + off);
            if (maybeCount == 0 || maybeCount > 4096) continue;

            uint64_t maybeArrayVa = *(const uint64_t*)(pe.data() + off + 8);
            if (!IsPointerInSectionVa(maybeArrayVa, sections, imageBase)) continue;

            uint32_t arrRva = (uint32_t)(maybeArrayVa - imageBase);
            uint32_t arrRaw = RvaToFileOffset(pe, arrRva);
            if (arrRaw == 0 || arrRaw + sizeof(uint64_t) > pe.size()) continue;

            uint64_t module0Va = *(const uint64_t*)(pe.data() + arrRaw);
            if (!IsPointerInSectionVa(module0Va, sections, imageBase)) continue;

            uint32_t module0Rva = (uint32_t)(module0Va - imageBase);
            uint32_t module0Raw = RvaToFileOffset(pe, module0Rva);
            if (module0Raw == 0 || module0Raw + 24 > pe.size()) continue;

            uint64_t moduleNameVa = *(const uint64_t*)(pe.data() + module0Raw + 0);
            uint64_t methodCount = *(const uint64_t*)(pe.data() + module0Raw + 8);
            uint64_t methodPtrsVa = *(const uint64_t*)(pe.data() + module0Raw + 16);
            if (methodCount == 0 || methodCount > 2000000) continue;
            if (!IsPointerInSectionVa(methodPtrsVa, sections, imageBase)) continue;

            std::string moduleName;
            if (!ReadAsciiZAtVa(pe, imageBase, moduleNameVa, moduleName)) continue;
            if (moduleName.find(".dll") == std::string::npos) continue;

            // Score candidate: prefer counts close to metadata image count.
            int score = 0;
            if (expectedImageCount > 0) {
                uint64_t diff = maybeCount > expectedImageCount ? (maybeCount - expectedImageCount) : (expectedImageCount - maybeCount);
                if (diff == 0) score += 5;
                else if (diff <= 2) score += 3;
                else if (diff <= 8) score += 1;
            }

            // Basic validation on a few module pointers.
            uint64_t sample = maybeCount < 4 ? maybeCount : 4;
            int validModules = 0;
            for (uint64_t i = 0; i < sample; ++i) {
                uint64_t modVa = *(const uint64_t*)(pe.data() + arrRaw + i * sizeof(uint64_t));
                if (!IsPointerInSectionVa(modVa, sections, imageBase)) continue;
                uint32_t modRaw = RvaToFileOffset(pe, (uint32_t)(modVa - imageBase));
                if (modRaw == 0 || modRaw + 24 > pe.size()) continue;
                uint64_t modNameVa = *(const uint64_t*)(pe.data() + modRaw + 0);
                std::string mn;
                if (!ReadAsciiZAtVa(pe, imageBase, modNameVa, mn)) continue;
                if (mn.find(".dll") == std::string::npos) continue;
                validModules++;
            }
            score += validModules;
            if (score < 2) continue;

            outModulesArrayVa = maybeArrayVa;
            outModulesCount = maybeCount;
            return true;
        }
    }

    return false;
}

static void FindPointerReferencesInDataSections(
    const std::vector<uint8_t>& pe,
    uint64_t value,
    const std::vector<PeSectionRange>& sections,
    uint64_t imageBase,
    std::vector<uint64_t>& outRefsVa
) {
    outRefsVa.clear();
    for (const auto& s : sections) {
        bool isData = (s.characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA) != 0;
        if (!isData || (s.characteristics & IMAGE_SCN_MEM_EXECUTE)) continue;
        if (s.raw == 0 || s.rawSize < 8) continue;
        uint32_t begin = s.raw;
        uint32_t end = s.raw + s.rawSize;
        for (uint32_t off = begin; off + 8 <= end; off += 8) {
            uint64_t v = *(const uint64_t*)(pe.data() + off);
            if (v == value) {
                uint64_t va = imageBase + s.va + (off - s.raw);
                outRefsVa.push_back(va);
            }
        }
    }
}

static bool FindCodeRegistrationByMscorlib(
    const std::vector<uint8_t>& pe,
    const std::vector<PeSectionRange>& sections,
    uint64_t imageBase,
    uint32_t imageCount,
    int32_t metadataVersion,
    uint64_t& outCodeRegVa
) {
    outCodeRegVa = 0;
    const char marker[] = "mscorlib.dll";
    const size_t markerLen = sizeof(marker);

    for (const auto& sec : sections) {
        bool isExec = (sec.characteristics & IMAGE_SCN_MEM_EXECUTE) != 0;
        bool isData = (sec.characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA) != 0;
        if (!isExec && !isData) continue;
        if (sec.raw == 0 || sec.rawSize < markerLen) continue;

        uint32_t begin = sec.raw;
        uint32_t end = sec.raw + sec.rawSize;
        for (uint32_t off = begin; off + markerLen <= end; ++off) {
            if (memcmp(pe.data() + off, marker, markerLen) != 0) continue;
            uint64_t dllVa = imageBase + sec.va + (off - sec.raw);

            std::vector<uint64_t> refs1;
            FindPointerReferencesInDataSections(pe, dllVa, sections, imageBase, refs1);
            for (uint64_t r1 : refs1) {
                std::vector<uint64_t> refs2;
                FindPointerReferencesInDataSections(pe, r1, sections, imageBase, refs2);
                for (uint64_t r2 : refs2) {
                    for (uint32_t i = 0; i < imageCount; ++i) {
                        uint64_t want = r2 - (uint64_t)i * 8ull;
                        std::vector<uint64_t> refs3;
                        FindPointerReferencesInDataSections(pe, want, sections, imageBase, refs3);
                        for (uint64_t r3 : refs3) {
                            uint64_t countFieldVa = r3 - 8ull;
                            if (!IsPointerInSectionVa(countFieldVa, sections, imageBase)) continue;
                            uint32_t countRaw = RvaToFileOffset(pe, (uint32_t)(countFieldVa - imageBase));
                            if (countRaw == 0 || countRaw + 8 > pe.size()) continue;
                            uint64_t countValue = *(const uint64_t*)(pe.data() + countRaw);
                            if (countValue != imageCount) continue;

                            uint64_t baseShift = (metadataVersion >= 29) ? 14ull : 13ull;
                            uint64_t codeRegVa = r3 - baseShift * 8ull;
                            if (!IsPointerInSectionVa(codeRegVa, sections, imageBase)) continue;
                            outCodeRegVa = codeRegVa;
                            return true;
                        }
                    }
                }
            }
        }
    }

    return false;
}

static bool GetCodeGenModulesFromCodeRegistration(
    const std::vector<uint8_t>& pe,
    const std::vector<PeSectionRange>& sections,
    uint64_t imageBase,
    uint32_t imageCount,
    int32_t metadataVersion,
    uint64_t& outModulesArrayVa,
    uint64_t& outModulesCount
) {
    outModulesArrayVa = 0;
    outModulesCount = 0;

    uint64_t codeRegVa = 0;
    if (!FindCodeRegistrationByMscorlib(pe, sections, imageBase, imageCount, metadataVersion, codeRegVa)) {
        return false;
    }

    uint64_t countOff = (metadataVersion >= 29) ? 13ull * 8ull : 12ull * 8ull;
    uint64_t ptrOff = countOff + 8ull;
    uint64_t countVa = codeRegVa + countOff;
    uint64_t ptrVa = codeRegVa + ptrOff;
    if (!IsPointerInSectionVa(countVa, sections, imageBase) || !IsPointerInSectionVa(ptrVa, sections, imageBase)) {
        return false;
    }
    uint32_t countRaw = RvaToFileOffset(pe, (uint32_t)(countVa - imageBase));
    uint32_t ptrRaw = RvaToFileOffset(pe, (uint32_t)(ptrVa - imageBase));
    if (countRaw == 0 || ptrRaw == 0 || countRaw + 8 > pe.size() || ptrRaw + 8 > pe.size()) return false;

    uint64_t count = *(const uint64_t*)(pe.data() + countRaw);
    uint64_t arrVa = *(const uint64_t*)(pe.data() + ptrRaw);
    if (count == 0 || count > 4096) return false;
    if (!IsPointerInSectionVa(arrVa, sections, imageBase)) return false;

    outModulesCount = count;
    outModulesArrayVa = arrVa;
    return true;
}

static uintptr_t ResolveMethodOffsetFromCodeGenModule(
    const std::vector<uint8_t>& metadata,
    const Il2CppGlobalMetadataHeader& header,
    int32_t version,
    int32_t methodDefinitionIndex,
    const std::vector<uint8_t>& gameAssemblyBytes,
    const char* namespaze,
    const char* className
) {
    const uint8_t* m = GetMethodDefAt(metadata, header, version, methodDefinitionIndex);
    if (!m) return 0;

    uint32_t token = MethodDefToken(m, version);
    uint32_t methodPointerIndex = token & 0x00FFFFFFu;
    if (methodPointerIndex == 0) return 0;
    methodPointerIndex -= 1;

    int32_t declaringType = MethodDefDeclaringType(m, version);
    if (declaringType < 0) return 0;
    std::string imageName = GetImageNameForTypeIndex(metadata, header, declaringType);
    if (imageName.empty()) return 0;
    std::string imageNameNormalized = NormalizeModuleName(imageName);

    std::vector<std::string> moduleCandidates;
    moduleCandidates.push_back(imageNameNormalized);

    // If metadata mapping says "mscorlib" for a non-System gameplay type, try gameplay modules first.
    if (imageNameNormalized == "mscorlib" && !LooksLikeSystemType(namespaze, className)) {
        moduleCandidates.insert(moduleCandidates.begin(), "assembly-csharp");
        moduleCandidates.insert(moduleCandidates.begin() + 1, "assembly-csharp-firstpass");
    }

    std::vector<PeSectionRange> sections;
    uint64_t imageBase = 0;
    uint32_t imageSize = 0;
    if (!GetPeSections(gameAssemblyBytes, sections, imageBase, imageSize)) return 0;

    auto IsReadableRva = [](uint32_t rva, const std::vector<PeSectionRange>& secs) -> bool {
        for (const auto& s : secs) {
            if (rva >= s.va && rva < s.va + s.size) return true;
        }
        return false;
    };

    auto ScoreMethodPointersTable = [&](uint64_t tableVa, uint64_t count) -> int {
        if (tableVa <= imageBase || count == 0 || count > 2000000) return -1;
        uint32_t tableRva = (uint32_t)(tableVa - imageBase);
        uint32_t tableRaw = RvaToFileOffset(gameAssemblyBytes, tableRva);
        if (tableRaw == 0) return -1;
        uint64_t needBytes = (uint64_t)count * sizeof(uint64_t);
        if ((uint64_t)tableRaw + needBytes > gameAssemblyBytes.size()) return -1;

        // Validate first 16 pointers (or fewer) are mostly executable/non-null.
        uint64_t sample = count < 16 ? count : 16;
        int valid = 0;
        for (uint64_t i = 0; i < sample; ++i) {
            uint64_t va = *(const uint64_t*)(gameAssemblyBytes.data() + tableRaw + i * sizeof(uint64_t));
            if (va > imageBase && IsExecutableRva((uint32_t)(va - imageBase), sections)) {
                valid++;
            }
        }
        return valid;
    };

    uint32_t expectedImageCount = header.imagesCount / (int32_t)sizeof(Il2CppImageDefinitionV24);
    uint64_t modulesArrayVa = 0;
    uint64_t modulesCount = 0;
    bool gotFromCodeReg = GetCodeGenModulesFromCodeRegistration(
        gameAssemblyBytes,
        sections,
        imageBase,
        expectedImageCount,
        version,
        modulesArrayVa,
        modulesCount
    );
    if (!gotFromCodeReg) {
        if (!FindCodeGenModulesArray(gameAssemblyBytes, imageBase, sections, expectedImageCount, modulesArrayVa, modulesCount)) {
            return 0;
        }
    }

    uint32_t modulesArrayRaw = RvaToFileOffset(gameAssemblyBytes, (uint32_t)(modulesArrayVa - imageBase));
    if (modulesArrayRaw == 0) return 0;
    if ((uint64_t)modulesArrayRaw + modulesCount * sizeof(uint64_t) > gameAssemblyBytes.size()) return 0;

    const uint8_t* bytes = gameAssemblyBytes.data();
    std::vector<uint32_t> candidateRvas;
    std::string usedModuleCandidate;
    for (const std::string& wantedModule : moduleCandidates) {
        candidateRvas.clear();
        for (uint64_t i = 0; i < modulesCount; ++i) {
        uint64_t modVa = *(const uint64_t*)(bytes + modulesArrayRaw + i * sizeof(uint64_t));
        if (!IsPointerInSectionVa(modVa, sections, imageBase)) continue;
        uint32_t modRaw = RvaToFileOffset(gameAssemblyBytes, (uint32_t)(modVa - imageBase));
        if (modRaw == 0 || modRaw + 24 > gameAssemblyBytes.size()) continue;

        uint64_t moduleNameVa = *(const uint64_t*)(bytes + modRaw + 0);
        uint64_t methodCount = *(const uint64_t*)(bytes + modRaw + 8);
        uint64_t methodPtrsVa = *(const uint64_t*)(bytes + modRaw + 16);
        if (methodCount == 0 || methodCount > 2000000) continue;
        if (methodPointerIndex >= methodCount) continue;
        if (methodPtrsVa <= imageBase) continue;
        uint32_t methodPtrsRva = (uint32_t)(methodPtrsVa - imageBase);
        if (!IsReadableRva(methodPtrsRva, sections)) continue;

        std::string moduleName;
        if (!ReadAsciiZAtVa(gameAssemblyBytes, imageBase, moduleNameVa, moduleName)) continue;
        std::string moduleNameNormalized = NormalizeModuleName(moduleName);
        if (moduleNameNormalized != wantedModule) continue;

        int score = ScoreMethodPointersTable(methodPtrsVa, methodCount);
        if (score < 0) continue;

        uint32_t methodPtrsRaw = RvaToFileOffset(gameAssemblyBytes, methodPtrsRva);
        if (methodPtrsRaw == 0) continue;
        uint64_t entryRaw = (uint64_t)methodPtrsRaw + (uint64_t)methodPointerIndex * sizeof(uint64_t);
        if (entryRaw + sizeof(uint64_t) > gameAssemblyBytes.size()) continue;
        uint64_t methodVa = *(const uint64_t*)(bytes + entryRaw);
        if (methodVa <= imageBase) continue;
        uint32_t methodRva = (uint32_t)(methodVa - imageBase);
        if (!IsExecutableRva(methodRva, sections)) continue;

        candidateRvas.push_back(methodRva);
        }

        if (!candidateRvas.empty()) {
            usedModuleCandidate = wantedModule;
            break;
        }
    }

    if (candidateRvas.empty()) return 0;

    // Heuristic: prefer lower executable RVAs first (matches Il2CppDumper-like outputs in PE builds).
    uint32_t best = candidateRvas[0];
    for (uint32_t rva : candidateRvas) {
        if (rva < best) best = rva;
    }

    printf("[Resolver:file] module candidates for %s (wanted=%s, source=%s): %llu, chosen RVA=0x%X\n",
        imageName.c_str(),
        usedModuleCandidate.c_str(),
        gotFromCodeReg ? "CodeRegistration" : "Heuristic",
        (unsigned long long)candidateRvas.size(),
        best);
    return (uintptr_t)best;
}

static int32_t GetMethodPointersTableIndex(
    const std::vector<uint8_t>& metadata,
    const Il2CppGlobalMetadataHeader& header,
    int32_t version,
    int32_t methodDefinitionIndex
) {
    const uint8_t* m = GetMethodDefAt(metadata, header, version, methodDefinitionIndex);
    if (!m) return methodDefinitionIndex;

    // Il2CppDumper logic for >= 24.2:
    // methodPointerIndex = (methodDef.token & 0x00FFFFFF) - 1
    if (version >= 24) {
        uint32_t token = MethodDefToken(m, version);
        uint32_t tokenIndex = token & 0x00FFFFFFu;
        if (tokenIndex > 0) return (int32_t)(tokenIndex - 1);
    }

    // Older layouts rely on explicit methodIndex.
    int32_t methodIndex = MethodDefMethodIndex(m, version);
    if (methodIndex >= 0) return methodIndex;

    // Final fallback.
    return methodDefinitionIndex;
}

static bool ReadAllBytes(const std::string& path, std::vector<uint8_t>& out) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return false;
    std::streamsize size = file.tellg();
    if (size <= 0) return false;
    file.seekg(0, std::ios::beg);
    out.resize((size_t)size);
    return file.read((char*)out.data(), size).good();
}

static std::string GetModuleFilePath(HMODULE module) {
    if (!module) return {};
    char path[MAX_PATH] = { 0 };
    if (GetModuleFileNameA(module, path, MAX_PATH) == 0) return {};
    return std::string(path);
}

static std::string GetFileDirectory(const std::string& path) {
    size_t pos = path.find_last_of("\\/");
    return (pos == std::string::npos) ? std::string() : path.substr(0, pos);
}

static std::string GetFileNameWithoutExtension(const std::string& path) {
    size_t pos = path.find_last_of("\\/");
    std::string name = (pos == std::string::npos) ? path : path.substr(pos + 1);
    size_t ext = name.find_last_of('.');
    return (ext == std::string::npos) ? name : name.substr(0, ext);
}

static std::string JoinPath(const std::string& a, const std::string& b) {
    if (a.empty()) return b;
    char last = a.back();
    if (last == '\\' || last == '/') return a + b;
    return a + "\\" + b;
}

static bool FileExists(const std::string& path) {
    DWORD attr = GetFileAttributesA(path.c_str());
    return attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY);
}

static bool ResolvePathsRelativeToGameAssembly() {
    if (g_pathsResolveTried) {
        return g_pathsResolved;
    }
    g_pathsResolveTried = true;

    HMODULE gameAssembly = GetModuleHandleA("GameAssembly.dll");
    if (!gameAssembly) {
        printf("[Resolver] GameAssembly module not found.\n");
        g_pathsResolved = false;
        return false;
    }

    std::string assemblyPath = GetModuleFilePath(gameAssembly);
    if (assemblyPath.empty()) {
        printf("[Resolver] Failed to resolve GameAssembly path.\n");
        g_pathsResolved = false;
        return false;
    }

    g_gameAssemblyPath = assemblyPath;
    std::string assemblyDir = GetFileDirectory(assemblyPath);
    std::string exePath = GetModuleFilePath(GetModuleHandleA(NULL));
    std::string exeName = GetFileNameWithoutExtension(exePath);
    std::string exeDir = GetFileDirectory(exePath);

    if (!g_globalMetadataPath.empty() && FileExists(g_globalMetadataPath)) {
        g_pathsResolved = true;
        return true;
    }

    std::vector<std::string> candidates;
    if (!exeName.empty()) {
        candidates.push_back(JoinPath(assemblyDir, exeName + "_Data\\il2cpp_data\\Metadata\\global-metadata.dat"));
        candidates.push_back(JoinPath(exeDir, exeName + "_Data\\il2cpp_data\\Metadata\\global-metadata.dat"));
        candidates.push_back(JoinPath(assemblyDir, exeName + "_Data\\il2cpp_data\\global-metadata.dat"));
        candidates.push_back(JoinPath(exeDir, exeName + "_Data\\il2cpp_data\\global-metadata.dat"));
    }
    candidates.push_back(JoinPath(assemblyDir, "il2cpp_data\\Metadata\\global-metadata.dat"));
    candidates.push_back(JoinPath(assemblyDir, "..\\il2cpp_data\\Metadata\\global-metadata.dat"));
    candidates.push_back(JoinPath(assemblyDir, "il2cpp_data\\global-metadata.dat"));
    candidates.push_back(JoinPath(assemblyDir, "..\\il2cpp_data\\global-metadata.dat"));
    candidates.push_back(JoinPath(exeDir, "il2cpp_data\\Metadata\\global-metadata.dat"));
    candidates.push_back(JoinPath(exeDir, "..\\il2cpp_data\\Metadata\\global-metadata.dat"));

    for (const auto& candidate : candidates) {
        if (FileExists(candidate)) {
            g_globalMetadataPath = candidate;
            g_pathsResolved = true;
            return true;
        }
    }

    printf("[Resolver] Failed to resolve global-metadata.dat relative to GameAssembly.\n");
    g_pathsResolved = false;
    return false;
}

static bool IsExecutableVa(uint64_t va, uint64_t imageBase, const std::vector<PeSectionRange>& sections) {
    if (va <= imageBase) return false;
    uint64_t rva64 = va - imageBase;
    if (rva64 > 0xFFFFFFFFull) return false;
    uint32_t rva = (uint32_t)rva64;

    for (const auto& s : sections) {
        if ((s.characteristics & IMAGE_SCN_MEM_EXECUTE) == 0) continue;
        if (rva >= s.va && rva < (uint32_t)(s.va + s.size)) return true;
    }
    return false;
}

static int32_t FindMethodDefinitionIndexInMetadata(
    const std::vector<uint8_t>& metadata,
    const char* namespaze,
    const char* className,
    const char* methodName,
    int paramCount
) {
    // We only need early header fields (up to typeDefinitions/methods offsets).
    if (metadata.size() < 0x80) return -1;
    const Il2CppGlobalMetadataHeader* header = (const Il2CppGlobalMetadataHeader*)metadata.data();
    if (header->sanity != 0xFAB11BAF) return -1;

    auto FindStringIndicesInMetadata = [&](const char* needle) -> std::vector<int32_t> {
        std::vector<int32_t> indices;
        if (!needle || !needle[0]) return indices;
        uint64_t start = header->stringOffset;
        uint64_t size = (uint64_t)header->stringCount; // this field is byte-size in Unity metadata
        if (start >= metadata.size()) return indices;
        if (start + size > metadata.size()) size = metadata.size() - start;
        const uint8_t* s = metadata.data() + start;
        size_t needleLen = strlen(needle);
        if (needleLen == 0 || needleLen + 1 > size) return indices;

        for (uint64_t i = 0; i + needleLen < size; ++i) {
            if (memcmp(s + i, needle, needleLen) == 0 && s[i + needleLen] == 0) {
                indices.push_back((int32_t)i);
            }
        }
        return indices;
    };

    int32_t version = header->version;
    int32_t typeCount = header->typeDefinitionsCount / (int32_t)TypeDefSizeForVersion(version);
    int32_t methodCount = header->methodsCount / (int32_t)MethodDefSizeForVersion(version);
    if (typeCount <= 0 || methodCount <= 0) return -1;
    if ((uint64_t)header->typeDefinitionsOffset + (uint64_t)header->typeDefinitionsCount > metadata.size()) return -1;
    if ((uint64_t)header->methodsOffset + (uint64_t)header->methodsCount > metadata.size()) return -1;

    auto TypeMatches = [](const char* expectedNs, const char* expectedClass, const char* currentNs, const char* currentClass) -> bool {
        // Standard split style: namespace + class.
        if (strcmp(currentClass, expectedClass) == 0 && strcmp(currentNs, expectedNs) == 0) return true;

        // Some builds store full type name in class and empty namespace.
        if (currentNs[0] == '\0' && expectedNs[0] != '\0') {
            std::string merged = std::string(expectedNs) + "." + expectedClass;
            if (strcmp(currentClass, merged.c_str()) == 0) return true;
        }

        // Inverse case: expected class already comes as "Namespace.Class".
        if (expectedNs[0] == '\0') {
            const char* dot = strrchr(expectedClass, '.');
            if (dot && strcmp(currentClass, dot + 1) == 0) {
                std::string nsPart(expectedClass, (size_t)(dot - expectedClass));
                if (strcmp(currentNs, nsPart.c_str()) == 0) return true;
            }
        }

        return false;
    };

    auto EndsWith = [](const std::string& text, const std::string& suffix) -> bool {
        if (suffix.size() > text.size()) return false;
        return text.compare(text.size() - suffix.size(), suffix.size(), suffix) == 0;
    };

    auto TypeHeuristicMatch = [&](const char* currentNs, const char* currentClass) -> bool {
        if (TypeMatches(namespaze, className, currentNs, currentClass)) return true;

        std::string currentFull = currentNs[0] ? (std::string(currentNs) + "." + currentClass) : std::string(currentClass);
        std::string expectedFull = namespaze[0] ? (std::string(namespaze) + "." + className) : std::string(className);

        // Fallbacks for weird metadata formatting:
        // - full type stored as class when namespace is empty
        // - partial namespace differences between tools and metadata
        if (currentFull == expectedFull) return true;
        if (EndsWith(currentFull, "." + std::string(className))) return true;
        if (EndsWith(expectedFull, "." + std::string(currentClass))) return true;
        if (std::string(currentClass) == expectedFull) return true;
        if (std::string(className) == currentFull) return true;

        return false;
    };

    auto FindWithParamRule = [&](int requiredParamCount, bool heuristicTypeMatch) -> int32_t {
        for (int32_t i = 0; i < typeCount; ++i) {
            const uint8_t* t = GetTypeDefAt(metadata, *header, version, i);
            if (!t) continue;
            const char* currentClass = MetadataString(metadata, *header, (int32_t)TypeDefNameIndex(t, version));
            const char* currentNs = MetadataString(metadata, *header, (int32_t)TypeDefNamespaceIndex(t, version));
            if (heuristicTypeMatch) {
                if (!TypeHeuristicMatch(currentNs, currentClass)) continue;
            }
            else {
                if (!TypeMatches(namespaze, className, currentNs, currentClass)) continue;
            }

            int32_t start = TypeDefMethodStart(t, version);
            int32_t count = (int32_t)TypeDefMethodCount(t, version);
            if (start < 0 || count < 0 || start + count > methodCount) continue;

            for (int32_t j = 0; j < count; ++j) {
                const uint8_t* m = GetMethodDefAt(metadata, *header, version, start + j);
                if (!m) continue;
                const char* currentMethod = MetadataString(metadata, *header, (int32_t)MethodDefNameIndex(m, version));
                if (strcmp(currentMethod, methodName) != 0) continue;
                if (requiredParamCount >= 0 && MethodDefParamCount(m, version) != (uint16_t)requiredParamCount) continue;
                return start + j;
            }
        }
        return -1;
    };

    int32_t idx = FindWithParamRule(paramCount, false);
    if (idx >= 0) return idx;

    // Retry with relaxed type matching.
    idx = FindWithParamRule(paramCount, true);
    if (idx >= 0) return idx;

    if (paramCount >= 0) {
        // Some game builds expose different implicit parameters in metadata.
        idx = FindWithParamRule(-1, false);
        if (idx >= 0) return idx;
        idx = FindWithParamRule(-1, true);
        if (idx >= 0) return idx;
    }

    // Final fallback (Il2CppDumper-like approach): search by method name globally,
    // then rank candidates by declaring type similarity.
    auto ScoreDeclaringType = [&](const char* currentNs, const char* currentClass) -> int {
        int score = 0;
        if (strcmp(currentClass, className) == 0) score += 4;
        if (strcmp(currentNs, namespaze) == 0) score += 3;

        std::string currentFull = currentNs[0] ? (std::string(currentNs) + "." + currentClass) : std::string(currentClass);
        std::string expectedFull = namespaze[0] ? (std::string(namespaze) + "." + className) : std::string(className);
        if (currentFull == expectedFull) score += 6;
        if (!namespaze[0] && currentFull.find(className) != std::string::npos) score += 1;
        if (namespaze[0] && currentFull.find(namespaze) != std::string::npos) score += 1;
        return score;
    };

    std::vector<int32_t> rawNameIndices = FindStringIndicesInMetadata(methodName);
    auto NameMatches = [&](const uint8_t* m) -> bool {
        uint32_t ni = MethodDefNameIndex(m, version);
        const char* currentMethod = MetadataString(metadata, *header, (int32_t)ni);
        if (strcmp(currentMethod, methodName) == 0) return true;
        for (int32_t rawIndex : rawNameIndices) {
            if ((int32_t)ni == rawIndex) return true;
        }
        return false;
    };

    int bestScore = -1;
    int bestIndex = -1;
    int sameBest = 0;
    for (int32_t mi = 0; mi < methodCount; ++mi) {
        const uint8_t* m = GetMethodDefAt(metadata, *header, version, mi);
        if (!m) continue;
        if (!NameMatches(m)) continue;
        if (paramCount >= 0 && MethodDefParamCount(m, version) != (uint16_t)paramCount) continue;

        int32_t declType = MethodDefDeclaringType(m, version);
        const uint8_t* t = GetTypeDefAt(metadata, *header, version, declType);
        if (!t) continue;
        const char* currentClass = MetadataString(metadata, *header, (int32_t)TypeDefNameIndex(t, version));
        const char* currentNs = MetadataString(metadata, *header, (int32_t)TypeDefNamespaceIndex(t, version));

        int score = ScoreDeclaringType(currentNs, currentClass);
        if (score > bestScore) {
            bestScore = score;
            bestIndex = mi;
            sameBest = 1;
        }
        else if (score == bestScore) {
            sameBest++;
        }
    }

    if (bestIndex >= 0 && sameBest == 1) {
        return bestIndex;
    }

    // If method name is unique across metadata, take it even when type scoring is ambiguous.
    if (bestIndex >= 0 && sameBest > 1 && !rawNameIndices.empty()) {
        return bestIndex;
    }

    return -1;
}

struct MethodAnchor {
    const char* namespaze;
    const char* className;
    const char* methodName;
    int paramCount;
};

static bool AutoDetectMethodPointersRva(
    const std::vector<uint8_t>& metadataBytes,
    const std::vector<uint8_t>& gameAssemblyBytes,
    uintptr_t& outRva
) {
    if (metadataBytes.size() < 0x80) return false;
    const Il2CppGlobalMetadataHeader* header = (const Il2CppGlobalMetadataHeader*)metadataBytes.data();
    int32_t version = header->version;

    static const MethodAnchor anchors[] = {
        {"", "GameController", "GetStoryModeEnabled", 0},
        {"", "CustomStoryFunctions", "StoryModeEnabled", 0},
        {"Controllers", "StoryStateUtilities", "CheckOneOfScenariosUnlocked", 1},
        {"", "FStoryModel", "CheckSetOfScenariosUnlocked", 1},
        {"", "Player", "isSceneOpened", 2},
        {"YMatchThree.Core", "LevelGameplay", "IsLevelComplete", 0},
        {"", "GameController", "checkCoins", 3},
    };

    std::vector<int32_t> methodIndices;
    methodIndices.reserve(sizeof(anchors) / sizeof(anchors[0]));
    for (const auto& a : anchors) {
        int32_t defIdx = FindMethodDefinitionIndexInMetadata(metadataBytes, a.namespaze, a.className, a.methodName, a.paramCount);
        if (defIdx >= 0) {
            int32_t tableIdx = GetMethodPointersTableIndex(metadataBytes, *header, version, defIdx);
            if (tableIdx >= 0) methodIndices.push_back(tableIdx);
        }
    }

    if (methodIndices.size() < 3) {
        printf("[Resolver:file] Auto-detect skipped: not enough metadata anchors.\n");
        return false;
    }

    std::vector<PeSectionRange> sections;
    uint64_t imageBase = 0;
    uint32_t imageSize = 0;
    if (!GetPeSections(gameAssemblyBytes, sections, imageBase, imageSize)) {
        printf("[Resolver:file] Auto-detect failed: invalid PE.\n");
        return false;
    }

    for (const auto& sec : sections) {
        bool isExecutable = (sec.characteristics & IMAGE_SCN_MEM_EXECUTE) != 0;
        bool isInitializedData = (sec.characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA) != 0;
        if (isExecutable || !isInitializedData) continue;
        if (sec.raw == 0 || sec.rawSize < 0x100) continue;
        if ((uint64_t)sec.raw + (uint64_t)sec.rawSize > gameAssemblyBytes.size()) continue;

        uint32_t rawBegin = sec.raw;
        uint32_t rawEnd = sec.raw + sec.rawSize;

        for (uint32_t candidateRaw = rawBegin; candidateRaw + 8 <= rawEnd; candidateRaw += 8) {
            size_t matched = 0;
            bool failed = false;

            for (int32_t idx : methodIndices) {
                uint64_t ptrRaw = (uint64_t)candidateRaw + (uint64_t)idx * sizeof(uint64_t);
                if (ptrRaw + sizeof(uint64_t) > rawEnd) {
                    failed = true;
                    break;
                }

                uint64_t methodVa = *(const uint64_t*)(gameAssemblyBytes.data() + ptrRaw);
                if (!IsExecutableVa(methodVa, imageBase, sections)) {
                    failed = true;
                    break;
                }
                ++matched;
            }

            if (!failed && matched >= 3) {
                uint32_t candidateRva = sec.va + (candidateRaw - rawBegin);
                // Minimal sanity: table should be inside image and have non-null first pointer.
                uint32_t candidateOffset = RvaToFileOffset(gameAssemblyBytes, candidateRva);
                if (candidateOffset == 0 || candidateOffset + sizeof(uint64_t) > gameAssemblyBytes.size()) continue;
                uint64_t firstVa = *(const uint64_t*)(gameAssemblyBytes.data() + candidateOffset);
                if (!IsExecutableVa(firstVa, imageBase, sections)) continue;

                outRva = (uintptr_t)candidateRva;
                printf("[Resolver:file] Auto-detected methodPointers RVA: 0x%p\n", (void*)outRva);
                return true;
            }
        }
    }

    printf("[Resolver:file] Auto-detect methodPointers RVA failed.\n");
    return false;
}

static bool ResolveIl2CppApi(
    HMODULE gameAssembly,
    il2cpp_domain_get_t& domainGet,
    il2cpp_domain_get_assemblies_t& domainGetAssemblies,
    il2cpp_assembly_get_image_t& assemblyGetImage,
    il2cpp_class_from_name_t& classFromName,
    il2cpp_class_get_methods_t& classGetMethods,
    il2cpp_method_get_name_t& methodGetName,
    il2cpp_method_get_param_count_t& methodGetParamCount,
    il2cpp_method_get_pointer_t& methodGetPointer
) {
    if (!gameAssembly) return false;

    domainGet = (il2cpp_domain_get_t)GetProcAddress(gameAssembly, "il2cpp_domain_get");
    domainGetAssemblies = (il2cpp_domain_get_assemblies_t)GetProcAddress(gameAssembly, "il2cpp_domain_get_assemblies");
    assemblyGetImage = (il2cpp_assembly_get_image_t)GetProcAddress(gameAssembly, "il2cpp_assembly_get_image");
    classFromName = (il2cpp_class_from_name_t)GetProcAddress(gameAssembly, "il2cpp_class_from_name");
    classGetMethods = (il2cpp_class_get_methods_t)GetProcAddress(gameAssembly, "il2cpp_class_get_methods");
    methodGetName = (il2cpp_method_get_name_t)GetProcAddress(gameAssembly, "il2cpp_method_get_name");
    methodGetParamCount = (il2cpp_method_get_param_count_t)GetProcAddress(gameAssembly, "il2cpp_method_get_param_count");
    methodGetPointer = (il2cpp_method_get_pointer_t)GetProcAddress(gameAssembly, "il2cpp_method_get_pointer");

    return domainGet && domainGetAssemblies && assemblyGetImage && classFromName &&
        classGetMethods && methodGetName && methodGetParamCount && methodGetPointer;
}

static uintptr_t ResolveMethodOffsetFromFiles(
    const char* namespaze,
    const char* className,
    const char* methodName,
    int paramCount
) {
    std::vector<uint8_t> metadataBytes;
    if (!ReadAllBytes(g_globalMetadataPath, metadataBytes)) {
        printf("[Resolver:file] Failed to read metadata: %s\n", g_globalMetadataPath.c_str());
        return 0;
    }

    if (metadataBytes.size() < 0x80) return 0;
    const Il2CppGlobalMetadataHeader* header = (const Il2CppGlobalMetadataHeader*)metadataBytes.data();
    int32_t version = header->version;

    int32_t methodDefIndex = FindMethodDefinitionIndexInMetadata(metadataBytes, namespaze, className, methodName, paramCount);
    if (methodDefIndex < 0) {
        printf("[Resolver:file] Method not found in metadata: %s.%s::%s\n", namespaze, className, methodName);
        return 0;
    }
    int32_t methodIndex = GetMethodPointersTableIndex(metadataBytes, *header, version, methodDefIndex);
    if (methodIndex < 0) {
        printf("[Resolver:file] Invalid method pointer index for: %s.%s::%s\n", namespaze, className, methodName);
        return 0;
    }

    std::vector<uint8_t> gameAssemblyBytes;
    if (!ReadAllBytes(g_gameAssemblyPath, gameAssemblyBytes)) {
        printf("[Resolver:file] Failed to read GameAssembly: %s\n", g_gameAssemblyPath.c_str());
        return 0;
    }

    // Newer IL2CPP usually resolves by token + module-specific methodPointers.
    if (version >= 24) {
        uintptr_t moduleResolved = ResolveMethodOffsetFromCodeGenModule(
            metadataBytes,
            *header,
            version,
            methodDefIndex,
            gameAssemblyBytes,
            namespaze,
            className
        );
        if (moduleResolved != 0) {
            printf("[Resolver:file] module path used: %s.%s::%s -> 0x%p\n", namespaze, className, methodName, (void*)moduleResolved);
            return moduleResolved;
        }
        printf("[Resolver:file] module path failed for: %s.%s::%s, fallback to global table\n", namespaze, className, methodName);
    }

    if (g_methodPointersRva == 0) {
        uintptr_t autoRva = 0;
        if (AutoDetectMethodPointersRva(metadataBytes, gameAssemblyBytes, autoRva)) {
            g_methodPointersRva = autoRva;
        }
        else {
            printf("[Resolver:file] g_methodPointersRva is not set and auto-detect failed.\n");
            return 0;
        }
    }

    uint32_t tableOffset = RvaToFileOffset(gameAssemblyBytes, (uint32_t)g_methodPointersRva);
    if (tableOffset == 0) {
        printf("[Resolver:file] Invalid methodPointers RVA: 0x%p\n", (void*)g_methodPointersRva);
        return 0;
    }

    uint64_t ptrOffset = (uint64_t)tableOffset + (uint64_t)methodIndex * sizeof(uint64_t);
    if (ptrOffset + sizeof(uint64_t) > gameAssemblyBytes.size()) {
        printf("[Resolver:file] methodPointers index out of range: %d\n", methodIndex);
        return 0;
    }

    uint64_t methodVa = *(uint64_t*)(gameAssemblyBytes.data() + ptrOffset);
    uint64_t imageBase = GetImageBase(gameAssemblyBytes);
    if (imageBase == 0 || methodVa <= imageBase) {
        printf("[Resolver:file] Invalid method VA or image base.\n");
        return 0;
    }

    return (uintptr_t)(methodVa - imageBase);
}

static uintptr_t FindMethodOffsetByClassAndMethod(
    const char* namespaze,
    const char* className,
    const char* methodName,
    int paramCount // -1 means any overload
) {
    // First priority: file-based resolver (RVA path).
    if (ResolvePathsRelativeToGameAssembly()) {
        uintptr_t fileResolved = ResolveMethodOffsetFromFiles(namespaze, className, methodName, paramCount);
        if (fileResolved != 0) return fileResolved;
    }

    HMODULE gameAssembly = GetModuleHandleA("GameAssembly.dll");

    il2cpp_domain_get_t domainGet = nullptr;
    il2cpp_domain_get_assemblies_t domainGetAssemblies = nullptr;
    il2cpp_assembly_get_image_t assemblyGetImage = nullptr;
    il2cpp_class_from_name_t classFromName = nullptr;
    il2cpp_class_get_methods_t classGetMethods = nullptr;
    il2cpp_method_get_name_t methodGetName = nullptr;
    il2cpp_method_get_param_count_t methodGetParamCount = nullptr;
    il2cpp_method_get_pointer_t methodGetPointer = nullptr;

    if (ResolveIl2CppApi(
        gameAssembly,
        domainGet,
        domainGetAssemblies,
        assemblyGetImage,
        classFromName,
        classGetMethods,
        methodGetName,
        methodGetParamCount,
        methodGetPointer
    )) {
        void* domain = domainGet();
        if (!domain) {
            printf("[Resolver] il2cpp_domain_get returned null.\n");
            return 0;
        }

        const void** assemblies = nullptr;
        size_t assemblyCount = 0;
        domainGetAssemblies(domain, &assemblies, &assemblyCount);

        for (size_t i = 0; i < assemblyCount; ++i) {
            void* image = assemblyGetImage(assemblies[i]);
            if (!image) continue;

            void* klass = classFromName(image, namespaze, className);
            if (!klass) continue;

            void* iter = nullptr;
            while (void* method = classGetMethods(klass, &iter)) {
                const char* currentName = methodGetName(method);
                if (!currentName || strcmp(currentName, methodName) != 0) continue;

                uint32_t currentParamCount = methodGetParamCount(method);
                if (paramCount >= 0 && currentParamCount != (uint32_t)paramCount) continue;

                void* methodPtr = methodGetPointer(method);
                if (!methodPtr) continue;

                return (uintptr_t)methodPtr - (uintptr_t)gameAssembly;
            }
        }

        printf("[Resolver] Runtime API did not find: %s.%s::%s\n", namespaze, className, methodName);
        return 0;
    }

    printf("[Resolver] IL2CPP exports unavailable after RVA attempt.\n");
    return 0;
}
