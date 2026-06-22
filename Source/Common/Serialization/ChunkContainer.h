#pragma once

#include "MaterialBinaryArchive.h"
#include "Common/Core.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <vector>

// =============================================================================
// Chunk — base class for type constraint only. Serialize is static per type.
// =============================================================================

struct Chunk
{
	virtual ~Chunk() = default;
	virtual ChunkType GetType() const = 0;
};

// =============================================================================
// TChunk — simple single-value chunk.
// =============================================================================

template<ChunkType kTag, typename T>
struct TChunk : Chunk
{
	static constexpr ChunkType Tag = kTag;
	using Container = T;

	T data;
	explicit TChunk(T v) : data(std::move(v)) {}

	ChunkType GetType() const override { return kTag; }

	static bool Serialize(FArchive& ar, Container& obj)
	{
		ar << obj;
		return true;
	}
};

// =============================================================================
// ChunkContainer — closure-based write, index-based lazy read.
//
// Write:   push<T>(args...)  →  captures shared_ptr<Container> in closure
//                               →  Serialize(ar) executes closures
// Read:    Deserialize(ar)    →  builds index
//          Get<T>(out)        →  finds index entry, calls T::Serialize(ar, out)
// =============================================================================

class ChunkContainer
{
public:
	// ── Write ───────────────────────────────────────────────────────────

	template <typename T,
	          std::enable_if_t<std::is_base_of<Chunk, T>::value, int> = 0>
	void push(typename T::Container container)
	{
		auto pData = std::make_shared<typename T::Container>(std::move(container));
		m_Pending.push_back({ T::Tag, [pData](FArchive& ar) mutable {
			T::Serialize(ar, *pData);
		}});
	}

	void Serialize(FArchive& ar);

	// ── Read ────────────────────────────────────────────────────────────

	void Deserialize(FArchive& ar);

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
	struct PendingChunk
	{
		ChunkType type;
		std::function<void(FArchive&)> serialize;
	};
	std::vector<PendingChunk> m_Pending;

	struct ChunkIndex { uint8_t* offset; size_t size; };
	struct ChunkTypeHash {
		size_t operator()(ChunkType t) const {
			return std::hash<uint64_t>{}(static_cast<uint64_t>(t));
		}
	};
	std::unordered_map<ChunkType, ChunkIndex, ChunkTypeHash> m_Index;
};

// ── Inline impl ──────────────────────────────────────────────────────────────

inline void ChunkContainer::Serialize(FArchive& ar)
{
	for (auto& p : m_Pending)
	{
		ar << p.type;

		size_t offset = ar.Tell();
		uint32_t payloadSize = 0;
		ar << payloadSize;

		p.serialize(ar);

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
