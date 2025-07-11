#pragma once
#include <cstdint>

class Guid
{
public:
	Guid();
	Guid(uint64_t uuid);

	operator uint64_t() const { return m_UUID; }

	uint64_t m_UUID;
};


template <typename T> struct hash;

template<>
struct hash<Guid>
{
	uint64_t operator()(const Guid& uuid) const
	{
		return (uint64_t)uuid;
	}
};