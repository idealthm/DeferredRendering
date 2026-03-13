#pragma once

#include "RenderPass.h"

class FrameBuffer;

class LightingPass : public RenderPass
{
public:
	LightingPass(uint32 width, uint32 height);
	virtual ~LightingPass() = default;

	void Setup(RenderContext& ctx, FBAttachmentInfo & info) override;

	void Execute(Ref<Scene> scene) override;

private:
	Ref<Shader> m_Shader;
};
