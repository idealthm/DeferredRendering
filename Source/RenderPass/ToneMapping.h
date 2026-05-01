#pragma once
#include "RenderPass.h"
#include "Shapes/ScreenQuad.h"

class ToneMapping : public RenderPass
{
public:
	ToneMapping();

	void Setup(FBAttachmentInfo& info, uint32_t step, RenderContext& ctx) override;
	void Execute(Ref<Scene> scene, uint32_t step, RenderContext& ctx) override;

private:
	Ref<Shader> m_Shader;
	ScreenQuad  m_ScreenQuad;
};
