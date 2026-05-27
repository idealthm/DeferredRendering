#pragma once

#include "RenderPass.h"
#include "Shapes/ScreenQuad.h"

class MaterialInstance;
class Material;
class FrameBuffer;

class LightingPass : public RenderPass
{
public:
	LightingPass();
	virtual ~LightingPass() = default;

	void Setup(RenderContext& ctx) override;

	void Execute(Ref<Scene> scene, RenderContext& ctx) override;

private:
	Ref<Program> m_Shader;
	ScreenQuad  m_ScreenQuad;
	Ref<Material> m_Material;
	Ref<MaterialInstance> m_MaterialInstance;
};
