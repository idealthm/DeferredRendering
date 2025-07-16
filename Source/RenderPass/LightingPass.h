#pragma once

#include "RenderPass.h"

class FrameBuffer;

class LightingPass : public RenderPass
{
public:
	LightingPass(uint32 width, uint32 height);
	virtual ~LightingPass() = default;

	virtual void OnWindowSizeChanged(int32 width, int32 height) override;

	virtual void PrePass(RenderContext& context) override;
	virtual void OnPass(RenderContext& context) override;
	virtual void PostPass(RenderContext& context) override;

private:
	Ref<Shader>			m_LightingShader;
	Ref<FrameBuffer>	m_LightingFBO;
};
