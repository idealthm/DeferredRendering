#include "UnitCube.h"
#include "MeshBuilder.h"
#include "Model/MeshSection.h"
#include "Material/Material.h"
#include "RHI/VertexBuffer.h"

UnitCube::UnitCube()
{
	auto mesh = MeshBuilder::BuildCube();
	auto& section = mesh->GetMeshSections()[0];
	m_RenderPrimitive = mesh->GetRenderPrimitive();
	m_IndexOffset = section->GetIndexOffset();
	m_IndexCount = section->GetIndexCount();
}

Handle<RHI::HwRenderPrimitive> UnitCube::GetRenderPrimitiveHandle() const
{
	return m_RenderPrimitive->GetHandle();
}

Handle<RHI::HwVertexBufferInfo> UnitCube::GetVertexBufferInfoHandle() const
{
	return m_RenderPrimitive->GetDesc().vertexBuffer->GetVertexBufferInfoHandle();
}
