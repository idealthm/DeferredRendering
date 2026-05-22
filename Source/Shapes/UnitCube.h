#pragma once
#include "common/Core.h"
#include "RHI/RenderPrimitive.h"

class Material;

class UnitCube
{
public:
	UnitCube(const Ref<Material>& material = nullptr);

	Handle<RHI::HwRenderPrimitive> GetRenderPrimitiveHandle() const;
	Handle<RHI::HwVertexBufferInfo> GetVertexBufferInfoHandle() const;

private:
	Ref<RHI::RenderPrimitive> m_RenderPrimitive;
	uint32_t m_IndexOffset = 0;
	uint32_t m_IndexCount = 0;
};
