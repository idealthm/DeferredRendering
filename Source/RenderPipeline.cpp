#include "RenderPipeline.h"

#include <glad/glad.h>

#include "Engine.h"
#include "EngineEnum.h"
#include "Scene.h"
#include "Assimp/Util.h"
#include "Lights/Light.h"
#include "Model/Texture.h"
#include "RHI/PixelBufferDescriptor.h"
#include "RenderPass/CubeMapConvolution.h"
#include "RenderPass/ERPPass.h"
#include "RenderPass/KernelPass.h"
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

	// BRDF LUT — precomputed 2D integration map
	m_Context.BRDF_LUT = Util::LoadTexture("Assets/textures/ibl_brdf_lut.png", false, false);
	if (m_Context.BRDF_LUT)
		m_Context.BRDF_LUT->UpdateLodRange(0, 1);

	m_PerViewDescriptorSet = DescriptorSet(gEngine->GetPerViewSetLayout());

	m_GBufferPass    = CreateRef<GBufferPass>();
	m_ShadowPass     = CreateRef<ShadowPass>();
	m_LightPass      = CreateRef<LightingPass>();
	m_SkyLightPass   = CreateRef<SkyLightPass>();
	m_ToneMappingPass = CreateRef<ToneMapping>();

	Ref<Texture> hdr = Util::LoadTexture("Assets/textures/hdr/newport_loft.hdr", false, true);

	CreateResource(m_Context.ERP_Cubemap, RHI::TextureDesc{
		256, 256, 1, 1,
		RHI::Format::R11G11B10F,
		RHI::SamplerType::SAMPLER_CUBEMAP,
		RHI::TextureUsage::COLOR_ATTACHMENT | RHI::TextureUsage::SAMPLEABLE
	});

	CreateRef<ERPPass>()->Render(hdr, m_Context.ERP_Cubemap);

	// White reference cubemap for kernel validation — replace ERP_Cubemap
	// {
	// 	RHI::TextureDesc whiteDesc{ 1, 1, 1, 1, RHI::Format::RGBA16F,
	// 		RHI::SamplerType::SAMPLER_CUBEMAP, RHI::TextureUsage::SAMPLEABLE };
	// 	auto whiteCube = CreateRef<Texture>(whiteDesc);
	// 	float white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	// 	for (int face = 0; face < 6; face++)
	// 	{
	// 		PixelBufferDescriptor pbd(white, sizeof(white),
	// 			RHI::PixelDataFormat::RGBA, RHI::PixelDataType::FLOAT);
	// 		gEngine->GetDriver().update3DImage(whiteCube->GetHandle(),
	// 			0, 0, 0, face, 1, 1, 1, std::move(pbd));
	// 	}
	// 	whiteCube->UpdateLodRange(0, 1);
	// 	m_Context.ERP_Cubemap = whiteCube;
	// }

	// Generate irradiance kernel (cosine-weighted, 1 row)
	Ref<Texture> irradianceKernel;
	KernelPass().Generate(1, 2048, true, irradianceKernel);

	// Irradiance cubemap
	CreateResource(m_Context.IBL_IrradianceMap, RHI::TextureDesc{
		256, 256, 1, 1,
		RHI::Format::RGBA16F,
		RHI::SamplerType::SAMPLER_CUBEMAP,
		RHI::TextureUsage::COLOR_ATTACHMENT | RHI::TextureUsage::SAMPLEABLE
	});

	CreateRef<CubeMapConvolution>()->RenderIrradiance(irradianceKernel,
		m_Context.ERP_Cubemap, m_Context.IBL_IrradianceMap, 2048);
	m_Context.IBL_IrradianceMap->UpdateLodRange(0, 1);

	// Generate pre-filter kernel (GGX importance sample, 5 roughness levels)
	Ref<Texture> prefilterKernel;
	KernelPass().Generate(5, 2048, false, prefilterKernel);

	// Pre-filter envmap
	uint32_t const prefilterRes = 256;
	CreateResource(m_Context.IBL_PreFilterMap, RHI::TextureDesc{
		prefilterRes, prefilterRes, 1, 5,
		RHI::Format::R11G11B10F,
		RHI::SamplerType::SAMPLER_CUBEMAP,
		RHI::TextureUsage::COLOR_ATTACHMENT | RHI::TextureUsage::SAMPLEABLE
	});

	for (uint32_t mip = 0; mip < 5; mip++)
		CreateRef<CubeMapConvolution>()->RenderPrefilter(prefilterKernel,
			m_Context.ERP_Cubemap, m_Context.IBL_PreFilterMap, mip, 2048);

	m_Context.IBL_PreFilterMap->UpdateLodRange(0, 5);
	
}

void RenderPipeline::Shutdown()
{
	m_GBufferPass    = nullptr;
	m_ShadowPass     = nullptr;
	m_LightPass      = nullptr;
	m_SkyLightPass   = nullptr;
	m_ToneMappingPass = nullptr;
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

	// IBL textures
	if (m_Context.IBL_IrradianceMap)
		m_PerViewDescriptorSet.SetTexture(+PerViewBindingPoints::IBL_IRRADIANCE,
			m_Context.IBL_IrradianceMap->GetHandleForSampling(),
			RHI::SamplerParams{});
	if (m_Context.IBL_PreFilterMap)
		m_PerViewDescriptorSet.SetTexture(+PerViewBindingPoints::IBL_PREFILTER,
			m_Context.IBL_PreFilterMap->GetHandleForSampling(),
			RHI::SamplerParams{});

	if (m_Context.BRDF_LUT)
		m_PerViewDescriptorSet.SetTexture(+PerViewBindingPoints::BRDF_LUT,
			m_Context.BRDF_LUT->GetHandleForSampling(),
			RHI::SamplerParams{});

	// IBL params: x=prefilterMipCount, y=skyboxIntensity, z=envIntensity
	auto& ibl = m_Context.FrameDataUB.edit().iblParams;
	ibl.x = 5.0f;
	ibl.y = 1.0f;
	ibl.z = 1.0f;

	m_PerViewDescriptorSet.commit(driver, gEngine->GetPerViewSetLayout());
	m_PerViewDescriptorSet.bind(driver, DescriptorSetBindingPoints::PER_VIEW);

	StartPass(scene, m_GBufferPass, viewportSize);
	StartPass(scene, m_LightPass, viewportSize);
	StartPass(scene, m_SkyLightPass, viewportSize);
	StartPass(scene, m_ToneMappingPass, viewportSize);

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
