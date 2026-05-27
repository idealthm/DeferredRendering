#pragma once
#include <cstdint>
#include <cstring>
#include <vector>

#include "RHI/BufferDescriptor.h"

namespace RHI
{
	class RHIDriver;
}

class UniformBuffer
{
public:
	UniformBuffer() = default;
	explicit UniformBuffer(size_t size);

	// Typed access by field name
	template<typename T>
	bool SetValue(uint16_t offset, const T& value);

	template<typename T>
	bool GetValue(uint16_t offset, T& outValue) const;

	uint32_t GetSize() const { return m_BlockSize; }

	// Dirty tracking
	bool IsDirty() const { return m_Dirty; }
	void ClearDirty() { m_Dirty = false; }
	void MarkDirty() { m_Dirty = true; }

	BufferDescriptor toBufferDescriptor(RHI::RHIDriver& driver) const noexcept {
		return toBufferDescriptor(driver, 0, GetSize());
	}

	// copy the UBO data and cleans the dirty bits
	BufferDescriptor toBufferDescriptor(RHI::RHIDriver& driver, size_t const offset, size_t const size) const noexcept {
		BufferDescriptor p;
		p.size = size;
		p.buffer = malloc(p.size); // TODO: use out-of-line buffer if too large
		memcpy(p.buffer, reinterpret_cast<const char*>(m_Buffer) + offset, p.size); // inlined
		m_Dirty = true;
		p.setCallback([](void* buffer, size_t size, void* user){free(buffer);}, nullptr);
		return p;
	}

private:
	char m_Storage[96];
	void* m_Buffer;
	uint32_t m_BlockSize = 0;
	mutable bool m_Dirty = false;
};

template<typename T>
bool UniformBuffer::SetValue(uint16_t offset, const T& value)
{
	std::memcpy(m_Buffer + offset, &value, sizeof(T));
	return m_Dirty = true;
}

template<typename T>
bool UniformBuffer::GetValue(uint16_t offset, T& outValue) const
{
	std::memcpy(&outValue, m_Buffer + offset, sizeof(T));
	return true;
}
