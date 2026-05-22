#include "IndexBuffer.h"
#include "Engine.h"
#include "RHI/BufferDescriptor.h"

namespace RHI
{

IndexBuffer::IndexBuffer(const IndexBufferDesc& desc)
	: m_Desc(desc)
{
	auto& driver = gEngine->GetDriver();
	m_Handle = driver.CreateIndexBuffer(desc.elementType, desc.indexCount, desc.usage);
}

IndexBuffer::~IndexBuffer()
{
	auto& driver = gEngine->GetDriver();
	if (m_Handle)
		driver.DestroyIndexBuffer(m_Handle);
	if (m_BufferObject)
		driver.DestroyBuffer(m_BufferObject);
}

void IndexBuffer::SetData(const void* data, size_t size)
{
	auto& driver = gEngine->GetDriver();

	if (m_BufferObject)
		driver.DestroyBuffer(m_BufferObject);

	auto boh = driver.CreateBufferObject(size, BufferObjectBinding::VERTEX, m_Desc.usage);
	driver.updateBufferObject(boh, BufferDescriptor(data, size));
	driver.setIndexBufferObject(m_Handle, boh);
	m_BufferObject = boh;
}

} // namespace RHI
