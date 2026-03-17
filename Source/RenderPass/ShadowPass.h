#pragma once
#include "RenderPass.h"

class FrameBuffer;

class ShadowPass: public RenderPass
{
public:
	ShadowPass();
	~ShadowPass() override;

	void Setup(FBAttachmentInfo & info) override;
	void Execute(Ref<Scene> scene) override;

private:
	Ref<Shader> m_Shader;
};
