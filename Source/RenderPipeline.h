#pragma once

#include <memory>
#include <glm/glm.hpp>

#include "Common/Core.h"
#include "RenderContext.h"

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
	static RenderPipeline& Get();

	void Init(uint32_t width, uint32_t height);
	void Shutdown();
	void OnWindowResize(int32_t width, int32_t height);

	void Render(Ref<Scene>& scene, const glm::u32vec2& viewportSize);

	// Standalone pass — Setup → Attach → Execute
	void StartPass(const Ref<Scene>& scene, const Ref<RenderPass>& renderPass,
	               const glm::u32vec2& viewportSize);

	RenderContext& GetContext() { return m_Context; }

private:
	RenderContext m_Context;

	Ref<GBufferPass>    m_GBufferPass;
	Ref<ShadowPass>     m_ShadowPass;
	Ref<LightingPass>   m_LightPass;
	Ref<SkyLightPass>   m_SkyLightPass;
	Ref<ToneMapping>    m_ToneMappingPass;
};
