#pragma once

#include "MaterialBinaryArchive.h"
#include "Common/Core.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <vector>

struct Chunk {
	virtual ~Chunk() = default;
	virtual ChunkType GetType() const = 0;
	virtual void Serialize(FArchive& ar) = 0;
};

template<ChunkType kTag, typename T>
struct TChunk : Chunk {
	static constexpr ChunkType Tag = kTag;
	using Container = T;
	T value;
	explicit TChunk(T v) : value(std::move(v)) {}
	ChunkType GetType() const override { return kTag; }
	void Serialize(FArchive& ar) override { ar << value; }
	static bool Serialize(FArchive& ar, Container& obj) { ar << obj; return true; }
};

class ChunkContainer {
public:
	template <typename T, std::enable_if_t<std::is_base_of<Chunk, T>::value, int> = 0, typename... Args>
	const T& push(Args&&... args) {
		T* chunk = new T(std::forward<Args>(args)...);
		mChildren.emplace_back(chunk);
		return *chunk;
	}
	void Serialize(FArchive& ar);
	void Deserialize(FArchive& ar);

	template<typename ChunkT>
	bool Get(typename ChunkT::Container& out) {
		auto it = m_Index.find(ChunkT::Tag);
		if (it == m_Index.end()) return false;
		FArchiveRead ar(it->second.offset, it->second.size);
		return ChunkT::Serialize(ar, out);
	}
	size_t getSize() { FArchiveWrite ar; Serialize(ar); return ar.Tell(); }

private:
	std::vector<UniquePtr<Chunk>> mChildren;
	struct ChunkIndex { uint8_t* offset; size_t size; };
	struct ChunkTypeHash { size_t operator()(ChunkType t) const { return std::hash<uint64_t>{}(static_cast<uint64_t>(t)); } };
	std::unordered_map<ChunkType, ChunkIndex, ChunkTypeHash> m_Index;
};

inline void ChunkContainer::Serialize(FArchive& ar) {
	for (auto& chunk : mChildren) {
		ChunkType type = chunk->GetType(); ar << type;
		size_t offset = ar.Tell(); uint32_t payloadSize = 0; ar << payloadSize;
		chunk->Serialize(ar);
		size_t endPos = ar.Tell();
		payloadSize = static_cast<uint32_t>(endPos - offset - sizeof(uint32_t));
		ar.Seek(offset); ar.Serialize(&payloadSize, sizeof(payloadSize)); ar.Seek(endPos);
	}
}
inline void ChunkContainer::Deserialize(FArchive& ar) {
	m_Index.clear();
	while (!ar.IsEof()) { ChunkType type; uint32_t payloadSize = 0; ar << type << payloadSize; m_Index[type] = { ar.GetCursor(), payloadSize }; ar.Skip(payloadSize); }
}

template<ChunkType kTag, typename T>
struct TSimpleChunk {
	static constexpr ChunkType Tag = kTag;
	using Container = T;
	static bool Serialize(FArchive& ar, Container& obj) { ar << obj; return true; }
};
