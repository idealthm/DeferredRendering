#pragma once
#include "RenderPass.h"

class SkyLightPass : public RenderPass
{
public:
	SkyLightPass();

	void Setup(FBAttachmentInfo& info, uint32 step) override;
	void Execute(Ref<Scene> scene, uint32 step) override;

private:
	Ref<Shader> m_Shader;
};
