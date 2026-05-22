#pragma once
#include "RenderPass.h"

class FrameBuffer;

class ShadowPass: public RenderPass
{
public:
	ShadowPass();
	~ShadowPass() override;

	void Setup(FBAttachmentInfo& info, uint32_t step, RenderContext& ctx) override;
	void Execute(Ref<Scene> scene, uint32_t step, RenderContext& ctx) override;

private:
	Ref<Program> m_Shader;
};
