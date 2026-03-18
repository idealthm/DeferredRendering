#pragma once
#include "RenderPass.h"

class FrameBuffer;

class ShadowPass: public RenderPass
{
public:
	ShadowPass();
	~ShadowPass() override;

	void Setup(FBAttachmentInfo& info, uint32 step) override;
	void Execute(Ref<Scene> scene, uint32 step) override;

private:
	Ref<Shader> m_Shader;
};
