#include "ScreenQuad.h"
#include "MeshBuilder.h"
#include "Model/MeshSection.h"
#include "RHI/VertexBuffer.h"

ScreenQuad::ScreenQuad()
{
	auto mesh = MeshBuilder::BuildQuad(nullptr);
	auto& section = mesh->GetMeshSections()[0];
	m_RenderPrimitive = mesh->GetRenderPrimitive();
	m_IndexOffset = section->GetIndexOffset();
	m_IndexCount = section->GetIndexCount();
}

Handle<RHI::HwRenderPrimitive> ScreenQuad::GetRenderPrimitiveHandle() const
{
	return m_RenderPrimitive->GetHandle();
}

Handle<RHI::HwVertexBufferInfo> ScreenQuad::GetVertexBufferInfoHandle() const
{
	return m_RenderPrimitive->GetDesc().vertexBuffer->GetVertexBufferInfoHandle();
}
