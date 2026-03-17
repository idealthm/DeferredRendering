#pragma once

#include "RenderPass.h"

class FrameBuffer;

class LightingPass : public RenderPass
{
public:
	LightingPass();
	virtual ~LightingPass() = default;

	void Setup(FBAttachmentInfo & info) override;

	void Execute(Ref<Scene> scene) override;

private:
	Ref<Shader> m_Shader;
};
