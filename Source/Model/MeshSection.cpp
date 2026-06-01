#include "MeshSection.h"

#include "StaticMesh.h"
#include "Material/Material.h"
#include "Material/MaterialInstance.h"
#include "RHI/VertexBuffer.h"

MeshSection::MeshSection(const Ref<RHI::RenderPrimitive>& primitive, uint32_t indexOffset, uint32_t indexCount,
                         Ref<MaterialInstance> material)
	: m_RenderPrimitive(primitive), m_IndexOffset(indexOffset), m_IndexCount(indexCount), m_MaterialOverride(material)
{
}

void MeshSection::SetOwnerMesh(StaticMesh* mesh)
{
	m_OwnerMesh = std::move(mesh);
}

void MeshSection::SetMaterial(const Ref<MaterialInstance>& material)
{
	m_MaterialOverride = material;
}

Ref<MaterialInstance> MeshSection::GetMaterial()
{
	if (!m_MaterialOverride)
	{
		return m_OwnerMesh->GetMaterial();
	}
	return m_MaterialOverride;
}

Handle<RHI::HwVertexBufferInfo> MeshSection::GetVertexBufferInfoHandle() const
{
	return m_RenderPrimitive->GetDesc().vertexBuffer->GetVertexBufferInfoHandle();
}

RHI::PrimitiveType MeshSection::GetPrimitiveType() const
{
	return m_RenderPrimitive->GetDesc().primitiveType;
}
