#pragma once
#include "common/Core.h"
#include "RHI/RenderPrimitive.h"

class ScreenQuad
{
public:
	ScreenQuad();

	Handle<RHI::HwRenderPrimitive> GetRenderPrimitiveHandle() const;
	Handle<RHI::HwVertexBufferInfo> GetVertexBufferInfoHandle() const;
	uint32_t GetIndexOffset() const { return m_IndexOffset; }
	uint32_t GetIndexCount()  const { return m_IndexCount; }

private:
	Ref<RHI::RenderPrimitive> m_RenderPrimitive;
	uint32_t m_IndexOffset = 0;
	uint32_t m_IndexCount = 0;
};
