#include "RenderPrimitive.h"
#include "Engine.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

namespace RHI
{

RenderPrimitive::RenderPrimitive(const RenderPrimitiveDesc& desc)
	: m_Desc(desc)
{
	auto& driver = gEngine->GetDriver();
	m_Handle = driver.CreateRenderPrimitive(
		desc.vertexBuffer->GetHandle(),
		desc.indexBuffer->GetHandle(),
		desc.primitiveType);
}

RenderPrimitive::~RenderPrimitive()
{
	if (m_Handle)
		gEngine->GetDriver().DestroyRenderPrimitive(m_Handle);
}

} // namespace RHI
