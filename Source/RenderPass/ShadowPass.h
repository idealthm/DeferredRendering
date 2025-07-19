#pragma once
#include "RenderPass.h"

class FrameBuffer;

class ShadowPass: public RenderPass
{
public:
	ShadowPass(uint32 width, uint32 height);
	~ShadowPass();

	uint32 GetDepthRendererID() const override;

	void OnWindowSizeChanged(int32 width, int32 height) override;
	void PrePass(RenderContext& context) override;
	void OnPass(RenderContext& context) override;
	void PostPass(RenderContext& context) override;

private:
	Ref<FrameBuffer>	m_ShadowMapFBO;
	Ref<Shader>			m_ShadowMapShader;
};
