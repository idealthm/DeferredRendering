#pragma once
#include "RenderPass.h"
#include "Shapes/ScreenQuad.h"

class MaterialInstance;
class RenderTarget;

class ToneMapping : public RenderPass
{
public:
	ToneMapping();

	void Setup(RenderContext& ctx) override;
	void Execute(Ref<Scene> scene, RenderContext& ctx) override;

private:
	Ref<MaterialInstance> m_MaterialInstance;
	Ref<RenderTarget> m_RenderTarget;
	ScreenQuad m_ScreenQuad;
};
