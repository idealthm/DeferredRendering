#pragma once
#include "Common/Core.h"
#include "Common/Handle.h"
#include "DriverEnums.h"

namespace RHI
{

class VertexBuffer;
class IndexBuffer;

struct RenderPrimitiveDesc
{
	Ref<VertexBuffer> vertexBuffer;
	Ref<IndexBuffer> indexBuffer;
	PrimitiveType primitiveType = PrimitiveType::TRIANGLES;
};

class RenderPrimitive
{
public:
	class Builder
	{
	public:
		Builder& SetVertexBuffer(const Ref<VertexBuffer>& vb) { m_Desc.vertexBuffer = vb; return *this; }
		Builder& SetIndexBuffer(const Ref<IndexBuffer>& ib)   { m_Desc.indexBuffer = ib; return *this; }
		Builder& SetPrimitiveType(PrimitiveType t)            { m_Desc.primitiveType = t; return *this; }
		Ref<RenderPrimitive> Build() { return CreateRef<RenderPrimitive>(m_Desc); }

	private:
		RenderPrimitiveDesc m_Desc{};
	};

	RenderPrimitive(const RenderPrimitiveDesc& desc);
	virtual ~RenderPrimitive();

	bool IsValid() const { return !!m_Handle; }
	Handle<HwRenderPrimitive> GetHandle() const { return m_Handle; }
	const RenderPrimitiveDesc& GetDesc() const { return m_Desc; }

private:
	Handle<HwRenderPrimitive> m_Handle;
	RenderPrimitiveDesc m_Desc;
};

} // namespace RHI
