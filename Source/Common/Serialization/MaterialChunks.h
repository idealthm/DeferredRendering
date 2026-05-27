#pragma once

#include "ChunkContainer.h"
#include "Common/Material/MaterialTypes.h"

#include <cstdint>
#include <string>
#include <vector>

// =============================================================================
// FlatProperty — used by MatcMain.
// =============================================================================

struct FlatProperty
{
    std::string name;
    uint8_t     uniformType = 0;
    uint8_t     propertyId  = 0;
    std::string defaultValue;
};

inline FArchive& operator<<(FArchive& ar, FlatProperty& v)
{
    ar << v.name << v.uniformType << v.propertyId << v.defaultValue;
    return ar;
}

// =============================================================================
// Element-level operator<<
// =============================================================================

inline FArchive& operator<<(FArchive& ar, FieldInfo& v)
{
    ar << v.name << v.offset << v.stride << v.type
       << v.isArray << v.size << v.structName << v.sizeName;
    return ar;
}

inline FArchive& operator<<(FArchive& ar, SamplerInfo& v)
{
    ar << v.name << v.sampler << v.format << v.binding;
    return ar;
}

inline FArchive& operator<<(FArchive& ar, VariableParam& v)
{
    ar << v.name << v.type << v.location;
    return ar;
}

// =============================================================================
// Chunk definitions
// =============================================================================

struct ChunkSpirv
{
    static constexpr ChunkType Tag = ChunkType::MaterialSpirv;
    struct Container { std::vector<uint8_t> vertexSpirv; std::vector<uint8_t> fragmentSpirv; };
    static bool Serialize(FArchive& ar, Container& obj)
    {
        ar << obj.vertexSpirv << obj.fragmentSpirv;
        return true;
    }
};

struct ChunkGlsl
{
    static constexpr ChunkType Tag = ChunkType::MaterialGlsl;
    struct Container { std::string vertexGlsl; std::string fragmentGlsl; };
    static bool Serialize(FArchive& ar, Container& obj)
    {
        ar << obj.vertexGlsl << obj.fragmentGlsl;
        return true;
    }
};

struct ChunkUib
{
    static constexpr ChunkType Tag = ChunkType::MaterialUib;
    using Container = BufferInterfaceBlock;
    static bool Serialize(FArchive& ar, Container& obj)
    {
        ar << obj.instanceName << obj.structName << obj.size
           << obj.layout;

        uint32_t count = ar.IsLoading() ? 0 : static_cast<uint32_t>(obj.fields.size());
        ar << count;
        if (ar.IsLoading()) obj.fields.resize(count);
        for (auto& f : obj.fields)
            ar << f;
        return true;
    }
};

struct ChunkSib
{
    static constexpr ChunkType Tag = ChunkType::MaterialSib;
    using Container = SamplerInterfaceBlock;
    static bool Serialize(FArchive& ar, Container& obj)
    {
        ar << obj.mName << obj.mStageFlags;
        for (auto& s : obj.mSamplersInfoList)
            ar << s;
        return true;
    }
};

struct ChunkAttributeInfo
{
    static constexpr ChunkType Tag = ChunkType::MaterialAttributeInfo;
    struct Container { std::vector<VariableParam> inputs; std::vector<VariableParam> outputs; };
    static bool Serialize(FArchive& ar, Container& obj)
    {
        ar << obj.inputs << obj.outputs;
        return true;
    }
};

inline FArchive& operator<<(FArchive& ar, Descriptor& v)
{
    ar << v.name << v.type << v.binding;
    return ar;
}

struct ChunkDescriptorSetBindings
{
    static constexpr ChunkType Tag = ChunkType::MaterialDescriptorSetLayoutInfo;
    using Container = DescriptorSetInfo;
    static bool Serialize(FArchive& ar, Container& obj)
    {
        for (auto& bindings : obj)
        {
            uint32_t count = ar.IsLoading() ? 0 : static_cast<uint32_t>(bindings.size());
            ar << count;
            if (ar.IsLoading()) bindings.resize(count);
            for (auto& desc : bindings)
                ar << desc;
        }
        return true;
    }
};

inline FArchive& operator<<(FArchive& ar, RHI::DescriptorSetLayoutBinding& v)
{
    ar << v.type << v.stageFlags << v.binding << v.flags << v.count;
    return ar;
}

struct ChunkDescriptorSetLayout
{
    static constexpr ChunkType Tag = ChunkType::MaterialDescriptorSetLayout;
    using Container = std::array<RHI::DescriptorSetLayout, 2>;
    static bool Serialize(FArchive& ar, Container& obj)
    {
        for (auto& layout : obj)
        {
            uint32_t count = ar.IsLoading() ? 0 : static_cast<uint32_t>(layout.bindings.size());
            ar << count;
            if (ar.IsLoading()) layout.bindings.resize(count);
            for (uint32_t i = 0; i < count; i++)
                ar << layout.bindings[i];
        }
        return true;
    }
};

// ── Scalar metadata ──────────────────────────────────────────────────────

using ChunkName          = TSimpleChunk<ChunkType::MaterialName,             std::string>;
using ChunkVersion       = TSimpleChunk<ChunkType::MaterialVersion,          uint32_t>;
using ChunkShading       = TSimpleChunk<ChunkType::MaterialShading,          std::string>;
using ChunkDomain        = TSimpleChunk<ChunkType::MaterialDomain,           uint8_t>;
using ChunkRequiredAttrs = TSimpleChunk<ChunkType::MaterialRequiredAttributes, uint32_t>;
using ChunkProperties    = TSimpleChunk<ChunkType::MaterialProperties,       std::vector<FlatProperty>>;
using ChunkConstants     = TSimpleChunk<ChunkType::MaterialConstants,        std::vector<std::string>>;
