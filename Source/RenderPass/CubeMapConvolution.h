#pragma once
#include "RenderPass.h"

class CubeMapConvolution : public RenderPass
{
public:
	CubeMapConvolution();
	uint32 GetRenderTimes() override { return 6; }
	void Setup(FBAttachmentInfo& info, uint32 step) override;
	void Execute(Ref<Scene> scene, uint32 step) override;

private:
	Ref<Shader>			m_Shader;
};
