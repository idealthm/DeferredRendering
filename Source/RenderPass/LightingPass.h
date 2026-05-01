#pragma once

#include "RenderPass.h"

class FrameBuffer;

class LightingPass : public RenderPass
{
public:
	LightingPass();
	virtual ~LightingPass() = default;

	void Setup(FBAttachmentInfo& info, uint32_t step, RenderContext& ctx) override;

	void Execute(Ref<Scene> scene, uint32_t step, RenderContext& ctx) override;

private:
	Ref<Shader> m_Shader;
};
