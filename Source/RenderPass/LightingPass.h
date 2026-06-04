#pragma once

#include "RenderPass.h"
#include "Shapes/ScreenQuad.h"

class MaterialInstance;
class RenderTarget;

class LightingPass : public RenderPass
{
public:
	LightingPass();
	virtual ~LightingPass() = default;

	void Setup(RenderContext& ctx) override;
	void Execute(Ref<Scene> scene, RenderContext& ctx) override;

private:
	Ref<RenderTarget> m_RenderTarget;
	ScreenQuad  m_ScreenQuad;
	Ref<MaterialInstance> m_MaterialInstance;
	DescriptorSet m_GBufferDescriptorSet;
};
