#pragma once
#include "RenderPass.h"

class SkyLightPass : public RenderPass
{
public:
	SkyLightPass();

	void Setup(FBAttachmentInfo& info) override;
	void Execute(Ref<Scene> scene) override;

private:
	Ref<Shader> m_Shader;
};
