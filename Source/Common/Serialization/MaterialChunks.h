#pragma once

#include "ChunkContainer.h"

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
// Chunk definitions (shared between ShaderCompiler and DeferredRendering)
// =============================================================================

struct ChunkSpirv
{
    static constexpr ChunkType Tag = ChunkType::MaterialSpirv;

    struct Container
    {
        std::vector<uint8_t> vertexSpirv;
        std::vector<uint8_t> fragmentSpirv;
    };

    static bool Serialize(FArchive& ar, Container& obj)
    {
        ar << obj.vertexSpirv << obj.fragmentSpirv;
        return true;
    }
};

struct ChunkGlsl
{
    static constexpr ChunkType Tag = ChunkType::MaterialGlsl;

    struct Container
    {
        std::string vertexGlsl;
        std::string fragmentGlsl;
    };

    static bool Serialize(FArchive& ar, Container& obj)
    {
        ar << obj.vertexGlsl << obj.fragmentGlsl;
        return true;
    }
};

// ── Scalar metadata (TSimpleChunk aliases) ─────────────────────────────────

using ChunkName             = TSimpleChunk<ChunkType::MaterialName,             std::string>;
using ChunkVersion          = TSimpleChunk<ChunkType::MaterialVersion,          uint32_t>;
using ChunkShading          = TSimpleChunk<ChunkType::MaterialShading,          std::string>;
using ChunkDomain           = TSimpleChunk<ChunkType::MaterialDomain,           uint8_t>;
using ChunkRequiredAttrs    = TSimpleChunk<ChunkType::MaterialRequiredAttributes, uint32_t>;
using ChunkProperties       = TSimpleChunk<ChunkType::MaterialProperties,      std::vector<FlatProperty>>;
using ChunkConstants        = TSimpleChunk<ChunkType::MaterialConstants,       std::vector<std::string>>;
