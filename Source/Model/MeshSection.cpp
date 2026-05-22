#include "MeshSection.h"

#include "Material/Material.h"
#include "Material/MaterialInstance.h"
#include "RHI/VertexBuffer.h"

MeshSection::MeshSection(const Ref<RHI::RenderPrimitive>& primitive, uint32_t indexOffset, uint32_t indexCount,
                         Ref<MaterialInstance> material)
	: m_RenderPrimitive(primitive), m_IndexOffset(indexOffset), m_IndexCount(indexCount), m_Material(material)
{
}

void MeshSection::SetMaterial(const Ref<MaterialInstance>& material)
{
	m_Material = material;
}

Ref<MaterialInstance> MeshSection::GetMaterial()
{
	return m_Material;
}

Handle<RHI::HwVertexBufferInfo> MeshSection::GetVertexBufferInfoHandle() const
{
	return m_RenderPrimitive->GetDesc().vertexBuffer->GetVertexBufferInfoHandle();
}

RHI::PrimitiveType MeshSection::GetPrimitiveType() const
{
	return m_RenderPrimitive->GetDesc().primitiveType;
}
