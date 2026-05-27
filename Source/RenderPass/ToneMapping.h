#pragma once
#include "RenderPass.h"
#include "Shapes/ScreenQuad.h"

class ToneMapping : public RenderPass
{
public:
	ToneMapping();

	void Setup(RenderContext& ctx) override;
	void Execute(Ref<Scene> scene, RenderContext& ctx) override;

private:
	Ref<Program> m_Shader;
	ScreenQuad  m_ScreenQuad;
};
