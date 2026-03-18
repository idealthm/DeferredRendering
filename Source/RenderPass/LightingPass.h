#pragma once

#include "RenderPass.h"

class FrameBuffer;

class LightingPass : public RenderPass
{
public:
	LightingPass();
	virtual ~LightingPass() = default;

	void Setup(FBAttachmentInfo& info, uint32 step) override;

	void Execute(Ref<Scene> scene, uint32 step) override;

private:
	Ref<Shader> m_Shader;
};
