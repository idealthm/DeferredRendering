#pragma once
#include "RenderPass.h"
#include "Model/Texture.h"
#include "Shapes/UnitCube.h"

class EnvPreFilter : public RenderPass
{
public:
	EnvPreFilter();
	uint32_t GetRenderTimes() override;
	void Setup(FBAttachmentInfo& info, uint32_t step, RenderContext& ctx) override;
	void Execute(Ref<Scene> scene, uint32_t step, RenderContext& ctx) override;
private:
	RHI::TextureDesc	m_Desc;
	Ref<Program>			m_Shader;
	UnitCube			m_UnitCube;
};
