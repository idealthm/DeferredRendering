#pragma once
#include "RenderPass.h"
#include "Model/Texture.h"

class EnvPreFilter : public RenderPass
{
public:
	EnvPreFilter();
	uint32 GetRenderTimes() override;
	void Setup(FBAttachmentInfo& info, uint32 step) override;
	void Execute(Ref<Scene> scene, uint32 step) override;
private:
	TextureDescription	m_Desc;
	Ref<Shader>			m_Shader;
};
