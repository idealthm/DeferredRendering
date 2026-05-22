#include "UniformBuffer.h"

UniformBuffer::UniformBuffer(size_t size)
	: m_BlockSize(size)
{
}

void UniformBuffer::Clear(size_t size)
{
	m_BlockSize = size;
	m_Data.resize(m_BlockSize);
	std::fill(m_Data.begin(), m_Data.end(), 0);
	m_Dirty = true;
}