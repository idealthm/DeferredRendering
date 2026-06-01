#pragma once
#include "RenderPass.h"
#include "Shapes/UnitCube.h"

class SkyLightPass : public RenderPass
{
public:
	SkyLightPass();

	void Setup(RenderContext& ctx) override;
	void Execute(Ref<Scene> scene, RenderContext& ctx) override;

private:
};
