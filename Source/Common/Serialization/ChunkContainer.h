#pragma once

#include "MaterialBinaryArchive.h"

#include <cassert>
#include <cstdint>
#include <vector>

class ChunkContainer
{
public:
    static constexpr uint32_t kMagic   = 0x4254414D; // "MATB"
    static constexpr uint32_t kVersion = 1;

    // ── Write path ───────────────────────────────────────────────────────
    void WriteFileHeader(FArchive& ar);
    void BeginChunk(FArchive& ar, ChunkType type);
    void EndChunk(FArchive& ar);
    void PatchChunkCount(FArchive& ar, uint32_t count);

    // ── Read path ────────────────────────────────────────────────────────
    bool ReadFileHeader(FArchive& ar, uint32_t& outChunkCount);
    bool ReadChunkHeader(FArchive& ar, ChunkType& outType, uint32_t& outSize);
    void SkipChunkPayload(FArchive& ar);

private:
    struct ChunkState
    {
        uint64_t Type;
        size_t   PayloadStart;
        size_t   SizeOffset;
        uint32_t PayloadSize;
    };
    std::vector<ChunkState> m_Stack;
};

// ── Inline impl ─────────────────────────────────────────────────────────────

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
