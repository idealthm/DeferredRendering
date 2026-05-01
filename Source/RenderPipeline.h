#pragma once

#include <memory>
#include <glm/glm.hpp>

#include "Common/Core.h"
#include "Renderer.h"

class LightingPass;
class Scene;
class ToneMapping;
class SkyLightPass;
class GBufferPass;
class ShadowPass;
class RenderPass;
struct FBAttachmentInfo;

class RenderPipeline
{
public:
	RenderPipeline();
	~RenderPipeline() = default;

	void Render(Ref<Scene>& scene, const glm::u32vec2& viewportSize);

	RenderContext& GetContext() { return m_Context; }

private:
	void StartPass(const Ref<Scene>& scene, const Ref<RenderPass>& renderPass, const glm::u32vec2& viewportSize);

	RenderContext m_Context;

	Ref<GBufferPass>    m_GBufferPass;
	Ref<ShadowPass>     m_ShadowPass;
	Ref<LightingPass>   m_LightPass;
	Ref<SkyLightPass>   m_SkyLightPass;
	Ref<ToneMapping>    m_ToneMappingPass;
};
