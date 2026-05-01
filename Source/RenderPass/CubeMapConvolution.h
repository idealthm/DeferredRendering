#pragma once
#include "RenderPass.h"

class CubeMapConvolution : public RenderPass
{
public:
	CubeMapConvolution();
	uint32_t GetRenderTimes() override { return 6; }
	void Setup(FBAttachmentInfo& info, uint32_t step, RenderContext& ctx) override;
	void Execute(Ref<Scene> scene, uint32_t step, RenderContext& ctx) override;

private:
	Ref<Shader>			m_Shader;
};
