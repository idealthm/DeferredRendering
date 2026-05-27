#include "UniformBuffer.h"

UniformBuffer::UniformBuffer(size_t size)
	: m_BlockSize(size), m_Buffer(m_Storage), m_Dirty(true)
{
	if (size > sizeof(m_Storage))
	{
		m_Buffer = malloc(size);
	}
	memset(m_Buffer, 0, size);
}
