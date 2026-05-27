#pragma once
#include "RenderPass.h"
#include "Model/Texture.h"
#include "Shapes/UnitCube.h"

class EnvPreFilter : public RenderPass
{
public:
	EnvPreFilter();
	uint32_t GetRenderTimes() override;
	void Setup(RenderContext& ctx) override;
	void Execute(Ref<Scene> scene, RenderContext& ctx) override;
private:
	RHI::TextureDesc	m_Desc;
	Ref<Program>			m_Shader;
	UnitCube			m_UnitCube;
};
