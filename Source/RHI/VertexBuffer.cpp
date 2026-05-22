#include "VertexBuffer.h"
#include "Engine.h"
#include "RHI/BufferDescriptor.h"

namespace RHI
{

VertexBuffer::VertexBuffer(const VertexBufferDesc& desc)
	: m_Desc(desc)
{
	size_t attributeCount = 0;
	for (const auto& layout : desc.bufferLayouts)
		attributeCount += layout.GetElements().size();

	AttributeArray attributes{};
	size_t attrIndex = 0;
	for (size_t bi = 0; bi < desc.bufferLayouts.size(); bi++)
	{
		const auto& layout = desc.bufferLayouts[bi];
		for (const auto& element : layout)
		{
			ASSERT(attrIndex < MAX_VERTEX_ATTRIBUTE_COUNT);
			attributes[attrIndex++] = BufferElementToAttribute(
				element,
				static_cast<uint8_t>(bi),
				static_cast<uint8_t>(layout.GetStride()));
		}
	}

	auto& driver = gEngine->GetDriver();
	m_VBIHandle = driver.CreateVertexBufferInfo(
		desc.bufferLayouts.size(), attributeCount, attributes);
	m_Handle = driver.CreateVertexBuffer(desc.vertexCount, m_VBIHandle);

	m_BufferObjects.resize(desc.bufferLayouts.size());
}

VertexBuffer::~VertexBuffer()
{
	auto& driver = gEngine->GetDriver();
	if (m_Handle)
		driver.DestroyVertexBuffer(m_Handle);
	if (m_VBIHandle)
		driver.DestroyVertexBufferInfo(m_VBIHandle);
	for (auto& boh : m_BufferObjects)
	{
		if (boh)
			driver.DestroyBuffer(boh);
	}
}

void VertexBuffer::SetData(uint8_t bufferSlot, const void* data, size_t size)
{
	ASSERT(bufferSlot < m_BufferObjects.size());

	auto& driver = gEngine->GetDriver();

	// Destroy previous buffer object for this slot if any
	if (m_BufferObjects[bufferSlot])
	{
		driver.DestroyBuffer(m_BufferObjects[bufferSlot]);
	}

	auto boh = driver.CreateBufferObject(size, BufferObjectBinding::VERTEX, m_Desc.usage);
	driver.updateBufferObject(boh, BufferDescriptor(data, size));
	driver.setVertexBufferObject(m_Handle, bufferSlot, boh);
	m_BufferObjects[bufferSlot] = boh;
}

} // namespace RHI
