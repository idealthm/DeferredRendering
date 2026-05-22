#pragma once
#include "Common/Core.h"
#include "Common/Handle.h"
#include "DriverEnums.h"
#include "BufferLayout.h"

namespace RHI
{

struct VertexBufferDesc
{
	size_t vertexCount = 0;
	std::vector<BufferLayout> bufferLayouts;
	BufferUsage usage = BufferUsage::STATIC;
};

class VertexBuffer
{
public:
	class Builder
	{
	public:
		Builder& SetVertexCount(size_t c)           { m_Desc.vertexCount = c; return *this; }
		Builder& AddBufferLayout(const BufferLayout& l) { m_Desc.bufferLayouts.push_back(l); return *this; }
		Builder& SetUsage(BufferUsage u)            { m_Desc.usage = u; return *this; }
		Ref<VertexBuffer> Build() { return CreateRef<VertexBuffer>(m_Desc); }

	private:
		VertexBufferDesc m_Desc{};
	};

	VertexBuffer(const VertexBufferDesc& desc);
	virtual ~VertexBuffer();

	bool IsValid() const { return !!m_Handle; }
	Handle<HwVertexBuffer> GetHandle() const { return m_Handle; }
	Handle<HwVertexBufferInfo> GetVertexBufferInfoHandle() const { return m_VBIHandle; }
	const VertexBufferDesc& GetDesc() const { return m_Desc; }

	void SetData(uint8_t bufferSlot, const void* data, size_t size);

private:
	Handle<HwVertexBuffer> m_Handle;
	Handle<HwVertexBufferInfo> m_VBIHandle;
	VertexBufferDesc m_Desc;
	std::vector<Handle<HwBufferObject>> m_BufferObjects;
};

} // namespace RHI
