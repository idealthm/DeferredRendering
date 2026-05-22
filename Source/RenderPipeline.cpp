#include "RenderPipeline.h"

#include <glad/glad.h>

#include "Renderer.h"
#include "Scene.h"
#include "FrameBuffer/FrameBuffer.h"
#include "Lights/Light.h"
#include "Model/Texture.h"
#include "RenderPass/GBufferPass.h"
#include "RenderPass/ShadowPass.h"
#include "RenderPass/LightingPass.h"
#include "RenderPass/SkyLightPass.h"
#include "RenderPass/ToneMapping.h"

RenderPipeline::RenderPipeline()
{
	// m_Context.BRDF_LUT = CreateRef<Texture>("Assets/textures/ibl_brdf_lut.png");
	m_Context.FrameBuffer = CreateScope<FrameBuffer>();
	m_Context.ShadowWidth = 2048.f * 1;
	m_Context.ShadowHeight = 2048.f * 1;

	auto defaultTexBuilder = Texture::Builder()
		.SetWidth(1).SetHeight(1)
		.SetFormat(RHI::Format::RGBA8)
		.SetTarget(RHI::SamplerType::SAMPLER_2D)
		.SetMipLevels(0)
		.SetUsage(RHI::TextureUsage::SAMPLEABLE)
		.SetDepthOrLayers(1);

	GDefaultTextures.White  = defaultTexBuilder.Build();
	GDefaultTextures.Black  = defaultTexBuilder.Build();
	GDefaultTextures.Gray   = defaultTexBuilder.Build();
	GDefaultTextures.Normal = defaultTexBuilder.Build();

	m_GBufferPass = CreateRef<GBufferPass>();
	m_ShadowPass = CreateRef<ShadowPass>();
	m_LightPass = CreateRef<LightingPass>();
	m_SkyLightPass = CreateRef<SkyLightPass>();
	m_ToneMappingPass = CreateRef<ToneMapping>();
}

void RenderPipeline::Render(Ref<Scene>& scene, const glm::u32vec2& viewportSize)
{
	// StartPass(scene, m_ShadowPass, viewportSize);
	StartPass(scene, m_GBufferPass, viewportSize);
	// StartPass(scene, m_LightPass, viewportSize);
	// StartPass(scene, m_SkyLightPass, viewportSize);
	// StartPass(scene, m_ToneMappingPass, viewportSize);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderPipeline::StartPass(const Ref<Scene>& scene, const Ref<RenderPass>& renderPass, const glm::u32vec2& viewportSize)
{
	for (uint32_t i = 0; i < renderPass->GetRenderTimes(); i++)
	{
		FBAttachmentInfo FBInfo;
		FBInfo.Width = viewportSize.x;
		FBInfo.Height = viewportSize.y;
		renderPass->Setup(FBInfo, i, m_Context);
		m_Context.FrameBuffer->Attach(FBInfo);
		renderPass->Execute(scene, i, m_Context);
	}
}