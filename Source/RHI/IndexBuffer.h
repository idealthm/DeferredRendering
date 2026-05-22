#pragma once
#include "Common/Core.h"
#include "Common/Handle.h"
#include "DriverEnums.h"

namespace RHI
{

struct IndexBufferDesc
{
	ElementType elementType = ElementType::UINT;
	size_t indexCount = 0;
	BufferUsage usage = BufferUsage::STATIC;
};

class IndexBuffer
{
public:
	class Builder
	{
	public:
		Builder& SetElementType(ElementType t) { m_Desc.elementType = t; return *this; }
		Builder& SetIndexCount(size_t c)       { m_Desc.indexCount = c; return *this; }
		Builder& SetUsage(BufferUsage u)       { m_Desc.usage = u; return *this; }
		Ref<IndexBuffer> Build() { return CreateRef<IndexBuffer>(m_Desc); }

	private:
		IndexBufferDesc m_Desc{};
	};

	IndexBuffer(const IndexBufferDesc& desc);
	virtual ~IndexBuffer();

	bool IsValid() const { return !!m_Handle; }
	Handle<HwIndexBuffer> GetHandle() const { return m_Handle; }
	const IndexBufferDesc& GetDesc() const { return m_Desc; }
	uint32_t GetIndexCount() const { return static_cast<uint32_t>(m_Desc.indexCount); }

	void SetData(const void* data, size_t size);

private:
	Handle<HwIndexBuffer> m_Handle;
	Handle<HwBufferObject> m_BufferObject;
	IndexBufferDesc m_Desc;
};

} // namespace RHI
