#pragma once
#include <cstdint>
#include <cstring>
#include <vector>

class UniformBuffer
{
public:
	UniformBuffer() = default;
	explicit UniformBuffer(size_t size);

	void Clear(size_t size);

	// Typed access by field name
	template<typename T>
	bool SetValue(uint16_t offset, const T& value);

	template<typename T>
	bool GetValue(uint16_t offset, T& outValue) const;

	// Raw data access (for Commit)
	const uint8_t* GetData() const { return m_Data.data(); }
	uint8_t* data() { return m_Data.data(); }
	uint32_t GetSize() const { return static_cast<uint32_t>(m_Data.size()); }

	// Dirty tracking
	bool IsDirty() const { return m_Dirty; }
	void ClearDirty() { m_Dirty = false; }
	void MarkDirty() { m_Dirty = true; }

private:
	std::vector<uint8_t> m_Data;
	uint32_t m_BlockSize = 0;
	bool m_Dirty = false;
};

template<typename T>
bool UniformBuffer::SetValue(uint16_t offset, const T& value)
{
	std::memcpy(m_Data.data() + offset, &value, sizeof(T));
	return m_Dirty = true;
}

template<typename T>
bool UniformBuffer::GetValue(uint16_t offset, T& outValue) const
{
	std::memcpy(&outValue, m_Data.data() + offset, sizeof(T));
	return true;
}
