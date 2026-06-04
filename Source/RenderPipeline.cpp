#include "RenderPipeline.h"

#include <glad/glad.h>

#include "Engine.h"
#include "EngineEnum.h"
#include "Scene.h"
#include "Lights/Light.h"
#include "Model/Texture.h"
#include "RenderPass/GBufferPass.h"
#include "RenderPass/ShadowPass.h"
#include "RenderPass/LightingPass.h"
#include "RenderPass/SkyLightPass.h"
#include "RenderPass/ToneMapping.h"

RenderPipeline& RenderPipeline::Get()
{
	static RenderPipeline instance;
	return instance;
}

void RenderPipeline::Init(uint32_t width, uint32_t height)
{
	m_Context.ShadowWidth  = 2048.f;
	m_Context.ShadowHeight = 2048.f;
	m_Context.FrameDataHandle = gEngine->GetDriver().CreateBufferObject(m_Context.FrameDataUB.getSize(),
			RHI::BufferObjectBinding::UNIFORM, RHI::BufferUsage::DYNAMIC);
	m_Context.LightDataHandle = gEngine->GetDriver().CreateBufferObject(m_Context.LightDataUB.getSize(),
			RHI::BufferObjectBinding::UNIFORM, RHI::BufferUsage::DYNAMIC);

	auto defaultTexBuilder = Texture::Builder()
		.SetWidth(1).SetHeight(1)
		.SetFormat(RHI::Format::RGBA8)
		.SetTarget(RHI::SamplerType::SAMPLER_2D)
		.SetMipLevels(1)
		.SetUsage(RHI::TextureUsage::SAMPLEABLE)
		.SetDepthOrLayers(1);

	GDefaultTextures.White  = defaultTexBuilder.Build();
	GDefaultTextures.Black  = defaultTexBuilder.Build();
	GDefaultTextures.Gray   = defaultTexBuilder.Build();
	GDefaultTextures.Normal = defaultTexBuilder.Build();

	m_PerViewDescriptorSet = DescriptorSet(gEngine->GetPerViewSetLayout());

	m_GBufferPass    = CreateRef<GBufferPass>();
	m_ShadowPass     = CreateRef<ShadowPass>();
	m_LightPass      = CreateRef<LightingPass>();
	m_SkyLightPass   = CreateRef<SkyLightPass>();
	m_ToneMappingPass = CreateRef<ToneMapping>();
}

void RenderPipeline::Shutdown()
{
	m_GBufferPass    = nullptr;
	m_ShadowPass     = nullptr;
	m_LightPass      = nullptr;
	m_SkyLightPass   = nullptr;
	m_ToneMappingPass = nullptr;

	GDefaultTextures.White  = nullptr;
	GDefaultTextures.Black  = nullptr;
	GDefaultTextures.Gray   = nullptr;
	GDefaultTextures.Normal = nullptr;
}

void RenderPipeline::OnWindowResize(int32_t width, int32_t height)
{
	gEngine->GetDriver().SetViewport(0, 0, width, height);
}

void RenderPipeline::Render(Ref<Scene>& scene, const glm::u32vec2& viewportSize)
{
	auto& driver = gEngine->GetDriver();

	// PerView UBO — once per frame
	if (m_Context.FrameDataUB.isDirty())
	{
		if (!m_Context.FrameDataHandle)
		{
			m_Context.FrameDataHandle = driver.CreateBufferObject(m_Context.FrameDataUB.getSize(),
				RHI::BufferObjectBinding::UNIFORM, RHI::BufferUsage::DYNAMIC);
		}
		driver.updateBufferObject(m_Context.FrameDataHandle, m_Context.FrameDataUB.toBufferDescriptor(driver));
		m_Context.FrameDataUB.clean();
	}
	m_PerViewDescriptorSet.SetBuffer(+PerViewBindingPoints::FRAME_UNIFORM, m_Context.FrameDataHandle,
		0, m_Context.FrameDataUB.getSize());

	// LightData UBO — buffer handle bound here, content uploaded later by LightingPass
	m_PerViewDescriptorSet.SetBuffer(+PerViewBindingPoints::LIGHT_DATA, m_Context.LightDataHandle,
		0, sizeof(LightData));

	m_PerViewDescriptorSet.commit(driver, gEngine->GetPerViewSetLayout());
	m_PerViewDescriptorSet.bind(driver, DescriptorSetBindingPoints::PER_VIEW);

	StartPass(scene, m_GBufferPass, viewportSize);
	StartPass(scene, m_LightPass, viewportSize);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderPipeline::StartPass(const Ref<Scene>& scene, const Ref<RenderPass>& renderPass,
                               const glm::u32vec2& viewportSize)
{
	for (uint32_t i = 0; i < renderPass->GetRenderTimes(); i++)
	{
		renderPass->Setup(m_Context);
		renderPass->Execute(scene, m_Context);
	}
}
