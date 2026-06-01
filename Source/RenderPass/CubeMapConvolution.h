#pragma once
#include "RenderPass.h"
#include "Shapes/UnitCube.h"

class CubeMapConvolution : public RenderPass
{
public:
	CubeMapConvolution();
	uint32_t GetRenderTimes() override { return 6; }
	void Setup(RenderContext& ctx) override;
	void Execute(Ref<Scene> scene, RenderContext& ctx) override;

private:
	Ref<Program>	m_Shader;
};
