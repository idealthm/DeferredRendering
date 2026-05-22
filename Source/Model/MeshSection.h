#pragma once

#include "Renderer.h"
#include "RHI/RenderPrimitive.h"

class MaterialInstance;

class MeshSection {
public:
	MeshSection(const Ref<RHI::RenderPrimitive>& primitive, uint32_t indexOffset, uint32_t indexCount,
	            Ref<MaterialInstance> material = nullptr);

	void SetMaterial(const Ref<MaterialInstance>& material);
	Ref<MaterialInstance> GetMaterial();

	uint32_t GetIndexOffset() const { return m_IndexOffset; }
	uint32_t GetIndexCount()  const { return m_IndexCount; }

	Handle<RHI::HwRenderPrimitive> GetRenderPrimitiveHandle() const { return m_RenderPrimitive->GetHandle(); }
	Handle<RHI::HwVertexBufferInfo> GetVertexBufferInfoHandle() const;
	RHI::PrimitiveType GetPrimitiveType() const;

private:
	Ref<RHI::RenderPrimitive> m_RenderPrimitive;
	uint32_t                  m_IndexOffset;
	uint32_t                  m_IndexCount;
	Ref<MaterialInstance>     m_Material;
};
