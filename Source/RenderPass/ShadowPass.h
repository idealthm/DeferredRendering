#pragma once
#include "RenderPass.h"

class FrameBuffer;

class ShadowPass: public RenderPass
{
public:
	ShadowPass();
	~ShadowPass() override;

	void Setup(RenderContext& ctx) override;
	void Execute(Ref<Scene> scene, RenderContext& ctx) override;

private:
	Ref<Program> m_Shader;
};
