#pragma once
#include "RenderPass.h"
#include "Shapes/UnitCube.h"

class SkyLightPass : public RenderPass
{
public:
	SkyLightPass();

	void Setup(FBAttachmentInfo& info, uint32_t step, RenderContext& ctx) override;
	void Execute(Ref<Scene> scene, uint32_t step, RenderContext& ctx) override;

private:
	Ref<Shader> m_Shader;
	UnitCube	m_UnitCube;
};
