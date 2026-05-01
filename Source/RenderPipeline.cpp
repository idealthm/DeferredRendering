#include "RenderPipeline.h"

#include <glad/glad.h>

#include "Renderer.h"
#include "Scene.h"
#include "FrameBuffer/FrameBuffer.h"
#include "Lights/Light.h"
#include "Model/Texture.h"
#include "RHI/SamplerPool.h"
#include "RenderPass/GBufferPass.h"
#include "RenderPass/ShadowPass.h"
#include "RenderPass/LightingPass.h"
#include "RenderPass/SkyLightPass.h"
#include "RenderPass/ToneMapping.h"

RenderPipeline::RenderPipeline()
{
	m_Context.BRDF_LUT = CreateRef<Texture2D>("Assets/textures/ibl_brdf_lut.png");
	{
		RHI::SamplerParams brdfParams{};
		brdfParams.wrapS = RHI::SamplerWrapMode::ClampToEdge;
		brdfParams.wrapT = RHI::SamplerWrapMode::ClampToEdge;
		brdfParams.filterMin = RHI::SamplerMinFilter::LinearMipmapLinear;
		brdfParams.filterMag = RHI::SamplerMagFilter::Linear;
		m_Context.BRDF_LUT->SetSampler(SamplerPool::Get().GetOrCreate(brdfParams));
	}
	m_Context.FrameBuffer = CreateScope<FrameBuffer>();
	m_Context.FrameDataUB = CreateScope<ParamBuffer<FrameData>>(0);
	m_Context.LightDataUB = CreateScope<ParamBuffer<LightData>>(1);
	m_Context.ShadowWidth = 2048.f * 1;
	m_Context.ShadowHeight = 2048.f * 1;

	GDefaultTextures.White  = Texture2D::Create(0xFFFFFFFF);
	GDefaultTextures.Black  = Texture2D::Create(0xFF000000);
	GDefaultTextures.Gray   = Texture2D::Create(0xFF808080);
	GDefaultTextures.Normal = Texture2D::Create(0xFFFF8080);

	m_GBufferPass = CreateRef<GBufferPass>();
	m_ShadowPass = CreateRef<ShadowPass>();
	m_LightPass = CreateRef<LightingPass>();
	m_SkyLightPass = CreateRef<SkyLightPass>();
	m_ToneMappingPass = CreateRef<ToneMapping>();
}

void RenderPipeline::Render(Ref<Scene>& scene, const glm::u32vec2& viewportSize)
{
	StartPass(scene, m_ShadowPass, viewportSize);
	StartPass(scene, m_GBufferPass, viewportSize);
	StartPass(scene, m_LightPass, viewportSize);
	StartPass(scene, m_SkyLightPass, viewportSize);
	StartPass(scene, m_ToneMappingPass, viewportSize);

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
