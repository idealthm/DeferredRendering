#pragma once

#include "MaterialBinaryArchive.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

// =============================================================================
// ChunkContainer — indexed lazy-loading/-writing chunk container.
//
// Write:   Set<T>(obj)  →  Serialize(ar)    (caller handles dry-run + I/O)
// Read:    Deserialize(ar)  →  Get<T>(out)  (caller handles file I/O)
// =============================================================================

class ChunkContainer
{
public:
    // ── Indexed write ─────────────────────────────────────────────────────
    // Collect objects via move; serialisation is deferred until Serialize().
    template<typename ChunkT>
    void Set(typename ChunkT::Container&& obj)
    {
        using T = std::decay_t<decltype(obj)>;
        m_Pending.push_back({
            ChunkT::Tag,
            [pObj = std::make_shared<T>(std::move(obj))](FArchive& ar) mutable {
                ChunkT::Serialize(ar, *pObj);
            }
        });
    }

    // Write all pending chunks to the archive (no file header).
    void Serialize(FArchive& ar);

    // ── Indexed read ──────────────────────────────────────────────────────
    // Scan chunks from the archive, storing pointers into the caller's buffer.
    // The caller must keep the archive's backing buffer alive.
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
    // ── Index ─────────────────────────────────────────────────────────────
    struct ChunkIndex { uint8_t* offset; size_t size; };
    struct ChunkTypeHash {
        size_t operator()(ChunkType t) const {
            return std::hash<uint64_t>{}(static_cast<uint64_t>(t));
        }
    };
    std::unordered_map<ChunkType, ChunkIndex, ChunkTypeHash> m_Index;

    // ── Pending write list ─────────────────────────────────────────────────
    struct PendingChunk
    {
        ChunkType type;
        std::function<void(FArchive&)> serialize;
    };
    std::vector<PendingChunk> m_Pending;
};

// ── Inline impl ──────────────────────────────────────────────────────────────

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
