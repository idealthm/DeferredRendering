#pragma once

#include "MaterialBinaryArchive.h"

#include <cassert>
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

// =============================================================================
// ChunkContainer
// =============================================================================
//
// Two APIs coexist during the transition period:
//
//   Legacy (streaming): WriteFileHeader / BeginChunk / EndChunk / ReadFileHeader
//     etc. — used by existing chunk structs (MaterialSpirvChunk, …) that carry
//     a member Serialize(FArchive&).
//
//   New (indexed): Serialize / Deserialize / Get<T> / Set<T> — lazy-loading/
//     -writing driven by ChunkT::Tag, ChunkT::Container, ChunkT::Serialize.
//     No header, no file I/O — the caller owns the archive and the file.
// =============================================================================

class ChunkContainer
{
public:
    static constexpr uint32_t kMagic   = 0x4254414D; // "MATB"
    static constexpr uint32_t kVersion = 1;

    // ── Legacy: write helpers ──────────────────────────────────────────────
    void WriteFileHeader(FArchive& ar);
    void BeginChunk(FArchive& ar, ChunkType type);
    void EndChunk(FArchive& ar);
    void PatchChunkCount(FArchive& ar, uint32_t count);

    // ── Legacy: read helpers ───────────────────────────────────────────────
    bool ReadFileHeader(FArchive& ar, uint32_t& outChunkCount);
    bool ReadChunkHeader(FArchive& ar, ChunkType& outType, uint32_t& outSize);
    void SkipChunkPayload(FArchive& ar);

    // ── New: indexed write ─────────────────────────────────────────────────
    // Collect objects via move; serialisation is deferred until Serialize().
    // Only rvalue references accepted — the pending list owns the data.
    template<typename ChunkT>
    void Set(typename ChunkT::Container&& obj)
    {
        m_Pending.push_back({
            ChunkT::Tag,
            [obj = std::move(obj)](FArchive& ar) mutable {
                ChunkT::Serialize(ar, obj);
            }
        });
    }

    // Write all pending chunks to the archive (no header).
    // Caller does dry-run → allocate → real-write → file I/O.
    void Serialize(FArchive& ar);

    // ── New: indexed read ──────────────────────────────────────────────────
    // Read all chunks from the archive into m_Buffer and build the index.
    // The archive contents are copied — caller's buffer does not need to
    // survive after Deserialize returns.
    void Deserialize(FArchive& ar);

    // Look up T::Tag in the index and deserialise into out.
    // Returns false when the chunk is not present.
    template<typename ChunkT>
    bool Get(typename ChunkT::Container& out)
    {
        auto it = m_Index.find(ChunkT::Tag);
        if (it == m_Index.end())
            return false;

        FArchiveRead ar(it->second.offset, it->second.size);
        return ChunkT::Serialize(ar, out);
    }

private:
    // ── Legacy stack ───────────────────────────────────────────────────────
    struct ChunkState
    {
        uint64_t Type;
        size_t   PayloadStart;
        size_t   SizeOffset;
        uint32_t PayloadSize;
    };
    std::vector<ChunkState> m_Stack;

    // ── Indexed read ───────────────────────────────────────────────────────
    struct ChunkIndex { uint8_t* offset; size_t size; };
    struct ChunkTypeHash {
        size_t operator()(ChunkType t) const {
            return std::hash<uint64_t>{}(static_cast<uint64_t>(t));
        }
    };
    std::unordered_map<ChunkType, ChunkIndex, ChunkTypeHash> m_Index;

    // ── Indexed write ──────────────────────────────────────────────────────
    struct PendingChunk
    {
        ChunkType type;
        std::function<void(FArchive&)> serialize;
    };
    std::vector<PendingChunk> m_Pending;
};

// ── Legacy inline impl ───────────────────────────────────────────────────────

inline void ChunkContainer::WriteFileHeader(FArchive& ar)
{
    assert(ar.IsSaving());

    uint32_t magic      = kMagic;
    uint32_t version    = kVersion;
    uint32_t chunkCount = 0;
    ar.Serialize(&magic,      sizeof(magic));
    ar.Serialize(&version,    sizeof(version));
    ar.Serialize(&chunkCount, sizeof(chunkCount));
}

inline void ChunkContainer::BeginChunk(FArchive& ar, ChunkType type)
{
    uint64_t rawType = static_cast<uint64_t>(type);

    if (ar.IsLoading())
    {
        uint64_t fileType;
        ar.Serialize(&fileType, sizeof(fileType));
        uint32_t payloadSize;
        ar.Serialize(&payloadSize, sizeof(payloadSize));

        if (fileType != rawType)
        {
            ar.Skip(payloadSize);
            return;
        }
        m_Stack.push_back({ fileType, ar.Tell(), 0, payloadSize });
    }
    else
    {
        ar.Serialize(&rawType, sizeof(rawType));
        size_t sizeOffset = ar.Tell();
        uint32_t placeholder = 0;
        ar.Serialize(&placeholder, sizeof(placeholder));
        m_Stack.push_back({ rawType, ar.Tell(), sizeOffset, 0 });
    }
}

inline void ChunkContainer::EndChunk(FArchive& ar)
{
    assert(!m_Stack.empty());
    ChunkState state = m_Stack.back();
    m_Stack.pop_back();

    if (ar.IsLoading())
    {
        assert(ar.Tell() == state.PayloadStart + state.PayloadSize);
        (void)state;
    }
    else
    {
        size_t curPos = ar.Tell();
        uint32_t payloadSize = static_cast<uint32_t>(curPos - state.PayloadStart);

        ar.Seek(state.SizeOffset);
        ar.Serialize(&payloadSize, sizeof(payloadSize));
        ar.Seek(curPos);
    }
}

inline void ChunkContainer::PatchChunkCount(FArchive& ar, uint32_t count)
{
    assert(ar.IsSaving());

    size_t curPos = ar.Tell();
    ar.Seek(8); // skip magic(4) + version(4)
    ar.Serialize(&count, sizeof(count));
    ar.Seek(curPos);
}

inline bool ChunkContainer::ReadFileHeader(FArchive& ar, uint32_t& outChunkCount)
{
    assert(ar.IsLoading());

    uint32_t magic = 0, version = 0;
    ar.Serialize(&magic,   sizeof(magic));
    ar.Serialize(&version, sizeof(version));
    ar.Serialize(&outChunkCount, sizeof(outChunkCount));
    return magic == kMagic;
}

inline bool ChunkContainer::ReadChunkHeader(FArchive& ar, ChunkType& outType, uint32_t& outSize)
{
    assert(ar.IsLoading());

    if (ar.IsEof())
        return false;

    uint64_t rawType = 0;
    ar.Serialize(&rawType, sizeof(rawType));
    if (ar.IsEof())
        return false;

    outType = static_cast<ChunkType>(rawType);
    ar.Serialize(&outSize, sizeof(outSize));

    m_Stack.push_back({ rawType, ar.Tell(), 0, outSize });
    return true;
}

inline void ChunkContainer::SkipChunkPayload(FArchive& ar)
{
    assert(ar.IsLoading());
    assert(!m_Stack.empty());

    ChunkState state = m_Stack.back();
    m_Stack.pop_back();
    ar.Skip(state.PayloadSize);
}

// ── New indexed inline impl ───────────────────────────────────────────────────

inline void ChunkContainer::Serialize(FArchive& ar)
{
    for (auto& p : m_Pending)
    {
        ar << p.type;

        // Write placeholder, remember offset
        size_t offset = ar.Tell();
        uint32_t payloadSize = 0;
        ar << payloadSize;

        // Write payload
        p.serialize(ar);

        // Backpatch size
        size_t endPos = ar.Tell();
        payloadSize = static_cast<uint32_t>(endPos - offset - sizeof(uint32_t));
        ar.Seek(offset);
        ar.Serialize(&payloadSize, sizeof(payloadSize));
        ar.Seek(endPos);
    }
}

inline void ChunkContainer::Deserialize(FArchive& ar)
{
    m_Index.clear();

    while (!ar.IsEof())
    {
        ChunkType type;
        uint32_t payloadSize = 0;

        ar << type << payloadSize;

        m_Index[type] = { ar.GetCursor(), payloadSize };
        ar.Skip(payloadSize);
    }
}

// =============================================================================
// TSimpleChunk — single-value chunk without a dedicated Container struct.
//
// Usage:
//   using ChunkMaterialName = TSimpleChunk<ChunkType::MaterialName, std::string>;
//   cc.Set<ChunkMaterialName>(std::string("MyMat"));
// =============================================================================

template<ChunkType kTag, typename T>
struct TSimpleChunk
{
    static constexpr ChunkType Tag = kTag;
    using Container = T;

    static bool Serialize(FArchive& ar, Container& obj)
    {
        ar << obj;
        return true;
    }
};

// =============================================================================
// Test chunk types — exercise the new API with mixed data types.
// =============================================================================

namespace TestChunks
{

// ── DataA — flat primitives ───────────────────────────────────────────────────
struct DataA
{
    std::string name;
    int32_t     value = 0;
    float       scale = 1.0f;
};

struct ChunkA
{
    static constexpr ChunkType Tag       = ChunkType::MaterialName;
    using Container = DataA;

    static bool Serialize(FArchive& ar, Container& obj)
    {
        ar << obj.name << obj.value << obj.scale;
        return true;
    }
};

// ── DataB — array of integers ─────────────────────────────────────────────────
struct DataB
{
    std::vector<uint32_t> ids;
};

struct ChunkB
{
    static constexpr ChunkType Tag       = ChunkType::MaterialVersion;
    using Container = DataB;

    static bool Serialize(FArchive& ar, Container& obj)
    {
        ar << obj.ids;
        return true;
    }
};

// ── DataC — nested struct ─────────────────────────────────────────────────────
struct Vec3 { float x = 0, y = 0, z = 0; };

inline FArchive& operator<<(FArchive& ar, Vec3& v)
{
    ar << v.x << v.y << v.z;
    return ar;
}

struct DataC
{
    Vec3                position;
    std::vector<Vec3>   waypoints;
    bool                active = false;
};

struct ChunkC
{
    static constexpr ChunkType Tag       = ChunkType::MaterialShading;
    using Container = DataC;

    static bool Serialize(FArchive& ar, Container& obj)
    {
        ar << obj.position << obj.waypoints << obj.active;
        return true;
    }
};

// ── DataD — single-value via TSimpleChunk ──────────────────────────────────────
using ChunkD = TSimpleChunk<ChunkType::MaterialDomain, uint32_t>;

} // namespace TestChunks
