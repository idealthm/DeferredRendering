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

	void Setup(FBAttachmentInfo& info, uint32_t step, RenderContext& ctx) override;

	void Execute(Ref<Scene> scene, uint32_t step, RenderContext& ctx) override;

private:
	Ref<Program> m_Shader;
	ScreenQuad  m_ScreenQuad;
	Ref<Material> m_Material;
	Ref<MaterialInstance> m_MaterialInstance;
};
