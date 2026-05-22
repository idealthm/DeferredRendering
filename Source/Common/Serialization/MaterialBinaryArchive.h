#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

// ── charTo64bitNum ──────────────────────────────────────────────────────────
template<size_t N>
constexpr uint64_t charTo64bitNum(const char (&str)[N])
{
    return (static_cast<uint64_t>(str[0]) << 56)
         | (static_cast<uint64_t>(str[1]) << 48)
         | (static_cast<uint64_t>(str[2]) << 40)
         | (static_cast<uint64_t>(str[3]) << 32)
         | (static_cast<uint64_t>(str[4]) << 24)
         | (static_cast<uint64_t>(str[5]) << 16)
         | (static_cast<uint64_t>(str[6]) << 8)
         |  static_cast<uint64_t>(str[7]);
}

// ── ChunkType ───────────────────────────────────────────────────────────────
enum class ChunkType : uint64_t
{
    MaterialSpirv                   = charTo64bitNum("MAT_SPIR"),
    MaterialGlsl                    = charTo64bitNum("MAT_GLSL"),
    MaterialUib                     = charTo64bitNum("MAT_UIB "),
    MaterialSib                     = charTo64bitNum("MAT_SIB "),
    MaterialAttributeInfo           = charTo64bitNum("MAT_ATTR"),
    MaterialDescriptorSetLayoutInfo = charTo64bitNum("MAT_DSLI"),
    MaterialProperties              = charTo64bitNum("MAT_PROP"),
    MaterialConstants               = charTo64bitNum("MAT_CONS"),
    MaterialName                    = charTo64bitNum("MAT_NAME"),
    MaterialVersion                 = charTo64bitNum("MAT_VERS"),
    MaterialCacheId                 = charTo64bitNum("MAT_UUID"),
    MaterialShading                 = charTo64bitNum("MAT_SHAD"),
    MaterialDomain                  = charTo64bitNum("MAT_DOMN"),
    MaterialRequiredAttributes      = charTo64bitNum("MAT_REQA"),
    MaterialBlendingMode            = charTo64bitNum("MAT_BLEN"),
    MaterialDoubleSided             = charTo64bitNum("MAT_DOSI"),
    MaterialColorWrite              = charTo64bitNum("MAT_CWRIT"),
    MaterialDepthWrite              = charTo64bitNum("MAT_DWRIT"),
    MaterialDepthTest               = charTo64bitNum("MAT_DTEST"),
    MaterialCullingMode             = charTo64bitNum("MAT_CUMO"),
    MaterialMaskThreshold           = charTo64bitNum("MAT_THRS"),
    MaterialShadowMultiplier        = charTo64bitNum("MAT_SHML"),
};

// ── FArchive (abstract base) ────────────────────────────────────────────────
class FArchive
{
public:
    virtual ~FArchive() = default;

    bool IsLoading() const { return m_IsLoading; }
    bool IsSaving()  const { return !m_IsLoading; }
    bool IsDryRun()  const { return m_Start == nullptr; }

    virtual void Serialize(void* data, size_t size) = 0;

    void Seek(size_t pos)
    {
        if (m_Start)
            m_Cursor = m_Start + pos;
        else
            m_Cursor = reinterpret_cast<uint8_t*>(pos);
    }

    size_t Tell() const
    {
        if (m_Start)
            return static_cast<size_t>(m_Cursor - m_Start);
        return reinterpret_cast<size_t>(m_Cursor);
    }

    void Skip(size_t bytes) { Seek(Tell() + bytes); }
    bool IsEof() const { return m_Start && m_Cursor >= m_End; }

    const uint8_t* GetData() const { return m_Start; }
    uint8_t* GetCursor() const { return m_Cursor; }

    // ── Friend operators ─────────────────────────────────────────────────
    friend FArchive& operator<<(FArchive& ar, uint32_t& v) { ar.Serialize(&v, sizeof(v)); return ar; }
    friend FArchive& operator<<(FArchive& ar, int32_t& v)  { ar.Serialize(&v, sizeof(v)); return ar; }
    friend FArchive& operator<<(FArchive& ar, uint16_t& v) { ar.Serialize(&v, sizeof(v)); return ar; }
    friend FArchive& operator<<(FArchive& ar, int16_t& v)  { ar.Serialize(&v, sizeof(v)); return ar; }
    friend FArchive& operator<<(FArchive& ar, uint8_t& v)  { ar.Serialize(&v, sizeof(v)); return ar; }
    friend FArchive& operator<<(FArchive& ar, float& v)    { ar.Serialize(&v, sizeof(v)); return ar; }
    friend FArchive& operator<<(FArchive& ar, uint64_t& v) { ar.Serialize(&v, sizeof(v)); return ar; }

    friend FArchive& operator<<(FArchive& ar, bool& v)
    {
        uint8_t raw = ar.IsLoading() ? uint8_t(0) : uint8_t(v ? 1 : 0);
        ar.Serialize(&raw, sizeof(raw));
        if (ar.IsLoading()) v = (raw != 0);
        return ar;
    }

    friend FArchive& operator<<(FArchive& ar, std::string& v)
    {
        uint32_t len = ar.IsLoading() ? 0 : static_cast<uint32_t>(v.size());
        ar << len;
        if (ar.IsLoading())
        {
            v.resize(len);
            ar.Serialize(v.data(), len);
        }
        else if (len > 0)
        {
            ar.Serialize(v.data(), len);
        }
        return ar;
    }

protected:
    uint8_t* m_Start    = nullptr;
    uint8_t* m_Cursor   = nullptr;
    uint8_t* m_End      = nullptr;
    bool     m_IsLoading = false;
};

// ── FArchiveWrite ───────────────────────────────────────────────────────────
// If constructed with nullptr buffer → DryRun mode (cursor advances, no copy).
class FArchiveWrite : public FArchive
{
public:
    // DryRun constructor — m_Start remains nullptr, cursor tracks offset only.
    FArchiveWrite() {}

    FArchiveWrite(void* buffer, size_t capacity)
    {
        m_Start  = m_Cursor = static_cast<uint8_t*>(buffer);
        m_End    = m_Start + capacity;
        m_IsLoading = false;
    }

    void Serialize(void* data, size_t size) override
    {
        if (m_Start == nullptr)
        {
            // DryRun: cursor acts as offset counter.
            m_Cursor = reinterpret_cast<uint8_t*>(reinterpret_cast<size_t>(m_Cursor) + size);
            return;
        }
        if (size && m_Cursor + size <= m_End)
            std::memcpy(m_Cursor, data, size);
        m_Cursor += size;
    }
};

// ── FArchiveRead ────────────────────────────────────────────────────────────
class FArchiveRead : public FArchive
{
public:
    FArchiveRead() = default;

    FArchiveRead(const void* data, size_t size)
    {
        m_Start  = m_Cursor = const_cast<uint8_t*>(static_cast<const uint8_t*>(data));
        m_End    = m_Start + size;
        m_IsLoading = true;
    }

    void Serialize(void* data, size_t size) override
    {
        if (size && m_Cursor + size <= m_End)
            std::memcpy(data, m_Cursor, size);
        m_Cursor += size;
    }

    // Create a sub-archive over the next `size` bytes, advancing this cursor.
    FArchiveRead Slice(size_t size)
    {
        size_t clamped = (m_Cursor + size <= m_End) ? size : static_cast<size_t>(m_End - m_Cursor);
        FArchiveRead sub(m_Cursor, clamped);
        m_Cursor += clamped;
        return sub;
    }
};

// ── Container operators (free functions) ────────────────────────────────────

template<typename T>
FArchive& operator<<(FArchive& ar, std::vector<T>& v)
{
    uint32_t count = ar.IsLoading() ? 0 : static_cast<uint32_t>(v.size());
    ar << count;
    if (ar.IsLoading())
        v.resize(count);
    for (auto& item : v)
        ar << item;
    return ar;
}

template<typename T, size_t N>
FArchive& operator<<(FArchive& ar, std::array<T, N>& arr)
{
    for (auto& item : arr)
        ar << item;
    return ar;
}

template<typename E>
std::enable_if_t<std::is_enum_v<E>, FArchive&> operator<<(FArchive& ar, E& v)
{
    using UT = std::underlying_type_t<E>;
    UT raw = ar.IsLoading() ? UT(0) : static_cast<UT>(v);
    ar << raw;
    if (ar.IsLoading())
        v = static_cast<E>(raw);
    return ar;
}

