#pragma once
#include "RenderPass.h"
#include "Shapes/UnitCube.h"

class MaterialInstance;
class RenderTarget;

class SkyLightPass : public RenderPass
{
public:
	SkyLightPass();

	void Setup(RenderContext& ctx) override;
	void Execute(Ref<Scene> scene, RenderContext& ctx) override;

private:
	Ref<MaterialInstance> m_MaterialInstance;
	Ref<RenderTarget> m_RenderTarget;
};
