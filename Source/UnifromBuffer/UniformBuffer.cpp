#include "UniformBuffer.h"

UniformBuffer::UniformBuffer(size_t size)
	: m_Buffer(m_Storage), m_BlockSize(size), m_Dirty(true)
{
	if (size > sizeof(m_Storage))
	{
		m_Buffer = malloc(size);
	}
	memset(m_Buffer, 0, size);
}

UniformBuffer::UniformBuffer(UniformBuffer&& rhs) noexcept
{
	if (rhs.isLocalStorage()) {
		m_Buffer = m_Storage;
		m_BlockSize = rhs.m_BlockSize;
		memcpy(m_Buffer, rhs.m_Buffer, rhs.m_BlockSize);
	} else {
		m_Buffer = rhs.m_Buffer;
		m_BlockSize = rhs.m_BlockSize;
	}
	rhs.m_Buffer = rhs.m_Storage;
	rhs.m_BlockSize = 0;
}

UniformBuffer& UniformBuffer::operator=(UniformBuffer&& rhs) noexcept
{
	if (this != &rhs) {
		if (!isLocalStorage()) {
			free(m_Buffer);
		}
		m_Dirty = rhs.m_Dirty;
		if (rhs.isLocalStorage()) {
			m_Buffer = m_Storage;
			m_BlockSize = rhs.m_BlockSize;
			memcpy(m_Buffer, rhs.m_Buffer, rhs.m_BlockSize);
		} else {
			m_Buffer = rhs.m_Buffer;
			m_BlockSize = rhs.m_BlockSize;
		}
		rhs.m_Buffer = rhs.m_Storage;
		rhs.m_BlockSize = 0;
	}
	return *this;
}

UniformBuffer::~UniformBuffer()
{
	if (!isLocalStorage())
	{
		free(m_Buffer);
	}
}

bool UniformBuffer::isLocalStorage() const
{
	return m_Buffer == m_Storage;
}
