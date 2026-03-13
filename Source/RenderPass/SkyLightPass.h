#pragma once
#include "RenderPass.h"

class SkyLightPass : public RenderPass
{
public:
	SkyLightPass(int width, int height);

	void Setup(RenderContext& ctx, FBAttachmentInfo& info) override;
	void Execute(Ref<Scene> scene) override;

private:
	Ref<Shader> m_Shader;
};
