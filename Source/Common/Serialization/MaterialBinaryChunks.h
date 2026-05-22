#pragma once

#include "MaterialBinaryArchive.h"

// ── Flat serializable structs ───────────────────────────────────────────────

struct FlatFieldInfo
{
    std::string name;
    uint16_t    offset = 0;
    uint8_t     stride = 0;
    uint8_t     fieldType = 0;
    uint8_t     isArray = 0;
    uint32_t    arraySize = 0;
    std::string structName;
    std::string sizeName;
};

inline FArchive& operator<<(FArchive& ar, FlatFieldInfo& v)
{
    ar << v.name << v.offset << v.stride << v.fieldType << v.isArray << v.arraySize << v.structName << v.sizeName;
    return ar;
}

struct FlatSamplerInfo
{
    std::string name;
    uint8_t     samplerType = 0;
    uint8_t     binding = 0;
};

inline FArchive& operator<<(FArchive& ar, FlatSamplerInfo& v)
{
    ar << v.name << v.samplerType << v.binding;
    return ar;
}

struct FlatVariableParam
{
    std::string name;
    uint8_t     fieldType = 0;
    uint8_t     location = 0;
};

inline FArchive& operator<<(FArchive& ar, FlatVariableParam& v)
{
    ar << v.name << v.fieldType << v.location;
    return ar;
}

struct FlatDescriptorBinding
{
    std::string name;
    uint8_t     descriptorType = 0;
    uint8_t     binding = 0;
};

inline FArchive& operator<<(FArchive& ar, FlatDescriptorBinding& v)
{
    ar << v.name << v.descriptorType << v.binding;
    return ar;
}

struct FlatProperty
{
    std::string name;
    uint8_t     uniformType = 0;
    uint8_t     propertyId = 0;
    std::string defaultValue;
};

inline FArchive& operator<<(FArchive& ar, FlatProperty& v)
{
    ar << v.name << v.uniformType << v.propertyId << v.defaultValue;
    return ar;
}

// ── Chunk structs ───────────────────────────────────────────────────────────

struct MaterialSpirvChunk
{
    std::vector<uint8_t> vertexSpirv;
    std::vector<uint8_t> fragmentSpirv;
    void Serialize(FArchive& ar) { ar << vertexSpirv << fragmentSpirv; }
};

struct MaterialGlslChunk
{
    std::string vertexGlsl;
    std::string fragmentGlsl;
    void Serialize(FArchive& ar) { ar << vertexGlsl << fragmentGlsl; }
};

struct MaterialUibChunk
{
    std::string              structName;
    std::string              instanceName;
    uint32_t                 blockSize = 0;
    uint8_t                  binding = 0;
    uint8_t                  memoryLayout = 0;
    std::vector<FlatFieldInfo> fields;
    void Serialize(FArchive& ar)
    {
        ar << structName << instanceName << blockSize << binding << memoryLayout << fields;
    }
};

struct MaterialSibChunk
{
    std::string                blockName;
    std::vector<FlatSamplerInfo> samplers;
    void Serialize(FArchive& ar) { ar << blockName << samplers; }
};

struct MaterialAttributeInfoChunk
{
    std::vector<FlatVariableParam> inputVariables;
    std::vector<FlatVariableParam> outputVariables;
    void Serialize(FArchive& ar) { ar << inputVariables << outputVariables; }
};

struct MaterialDescriptorSetLayoutInfoChunk
{
    static constexpr size_t kDescriptorSetCount = 4;
    std::vector<FlatDescriptorBinding> setBindings[kDescriptorSetCount];
    void Serialize(FArchive& ar)
    {
        for (auto& bindings : setBindings)
            ar << bindings;
    }
};

struct MaterialPropertiesChunk
{
    std::vector<FlatProperty> properties;
    void Serialize(FArchive& ar) { ar << properties; }
};

struct MaterialConstantsChunk
{
    std::vector<std::string> constants;
    void Serialize(FArchive& ar) { ar << constants; }
};

// ── Scalar chunks ───────────────────────────────────────────────────────────

struct MaterialNameChunk        { std::string name;         void Serialize(FArchive& ar) { ar << name; } };
struct MaterialVersionChunk     { uint32_t version = 0;     void Serialize(FArchive& ar) { ar << version; } };
struct MaterialCacheIdChunk     { uint64_t cacheId = 0;     void Serialize(FArchive& ar) { ar << cacheId; } };
struct MaterialShadingChunk     { std::string shadingModel; void Serialize(FArchive& ar) { ar << shadingModel; } };
struct MaterialDomainChunk      { uint8_t domain = 0;      void Serialize(FArchive& ar) { ar << domain; } };
struct MaterialRequiredAttributesChunk { uint32_t attributeMask = 0; void Serialize(FArchive& ar) { ar << attributeMask; } };
struct MaterialBlendingModeChunk      { uint8_t blendingMode = 0;   void Serialize(FArchive& ar) { ar << blendingMode; } };
struct MaterialDoubleSidedChunk       { uint8_t doubleSided = 0;    void Serialize(FArchive& ar) { ar << doubleSided; } };
struct MaterialColorWriteChunk        { uint8_t colorWrite = 1;     void Serialize(FArchive& ar) { ar << colorWrite; } };
struct MaterialDepthWriteChunk        { uint8_t depthWrite = 1;     void Serialize(FArchive& ar) { ar << depthWrite; } };
struct MaterialDepthTestChunk         { uint8_t depthFunc = 0;      void Serialize(FArchive& ar) { ar << depthFunc; } };
struct MaterialCullingModeChunk       { uint8_t cullingMode = 0;    void Serialize(FArchive& ar) { ar << cullingMode; } };
struct MaterialMaskThresholdChunk     { float threshold = 0.5f;     void Serialize(FArchive& ar) { ar << threshold; } };
struct MaterialShadowMultiplierChunk  { uint8_t shadowMultiplier = 1; void Serialize(FArchive& ar) { ar << shadowMultiplier; } };
