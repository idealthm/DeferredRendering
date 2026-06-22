#include "LightingPass.h"

#include "DescriptorSets.h"
#include "Engine.h"
#include "EngineEnum.h"
#include "Material/MaterialLibrary.h"
#include "RenderPipeline.h"
#include "RenderTarget.h"
#include "Scene.h"
#include "Lights/Light.h"
#include "Material/MaterialInstance.h"
#include "Model/Texture.h"
#include "RHI/PipelineState.h"
#include "RHI/TextureSampler.h"
#include "Shader/Program.h"
#include "common/glmHelper.h"

LightingPass::LightingPass()
	: m_GBufferDescriptorSet(gEngine->GetGBufferSetLayout())
{
	m_MaterialInstance = CreateRef<MaterialInstance>(
		MaterialLibrary::Get().GetMaterial("Lighting"));
}

void LightingPass::Setup(RenderContext& ctx)
{
	CreateResource(ctx.Final_SceneColor, RHI::TextureDesc{
		ctx.viewportSize.x, ctx.viewportSize.y, 1, 1,
		RHI::Format::RGBA16F,
		RHI::SamplerType::SAMPLER_2D,
		RHI::TextureUsage::COLOR_ATTACHMENT | RHI::TextureUsage::SAMPLEABLE
	});

	RenderTarget::Builder builder;
	builder.texture(AttachmentPoint::COLOR0, ctx.Final_SceneColor);
	m_RenderTarget = builder.Build();
}

void LightingPass::Execute(Ref<Scene> scene, RenderContext& ctx)
{
	auto& driver = gEngine->GetDriver();

	// Collect light data
	int32_t index = 0;
	auto& lightData = ctx.LightDataUB.edit();
	for (auto lightActor : scene->GetActors())
	{
		for (auto& Comp : lightActor->GetComponents())
		{
			if (auto Light = std::dynamic_pointer_cast<DirectionLightComponent>(Comp))
			{
				auto& info = lightData.lights[index++];
				info.position = Light->GetLocation();
				info.color = Light->GetColor();
				info.type = 0;
				info.intensity = Light->GetIntensity() * 10.0f;
				info.direction = Light->GetDirection();
			}
		}
	}
	lightData.NumLights = index;

	driver.updateBufferObject(ctx.LightDataHandle, ctx.LightDataUB.toBufferDescriptor(driver));

	// Bind GBuffer textures via GBUFFER descriptor set
	m_GBufferDescriptorSet.SetTexture(+GBufferBindingPoint::G_BUFFER_DEPTH,
		ctx.GBuffer_Depth->GetHandleForSampling(), TextureSampler::LinearClamp().GetParams());
	m_GBufferDescriptorSet.SetTexture(+GBufferBindingPoint::G_BUFFER_NORMAL,
		ctx.GBuffer_Normal->GetHandleForSampling(), TextureSampler::LinearClamp().GetParams());
	m_GBufferDescriptorSet.SetTexture(+GBufferBindingPoint::G_BUFFER_ALBEDO,
		ctx.GBuffer_Albedo->GetHandleForSampling(), TextureSampler::LinearClamp().GetParams());
	m_GBufferDescriptorSet.SetTexture(+GBufferBindingPoint::G_BUFFER_MATERIAL,
		ctx.GBuffer_Material->GetHandleForSampling(), TextureSampler::LinearClamp().GetParams());

	m_GBufferDescriptorSet.commit(driver, gEngine->GetGBufferSetLayout());
	m_GBufferDescriptorSet.bind(driver, DescriptorSetBindingPoints::G_BUFFER);

	m_MaterialInstance->Commit(driver);
	m_MaterialInstance->Use(driver);

	// Begin render pass
	RHI::RenderPassParams rpParams{};
	rpParams.viewport = { 0, 0, ctx.viewportSize.x, ctx.viewportSize.y };
	rpParams.flags.clear = RHI::TargetBufferFlags::COLOR0;
	rpParams.clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

	driver.beginRenderPass(m_RenderTarget->GetHandle(), rpParams);

	RHI::PipelineState state;
	state.program = m_MaterialInstance->GetShader(MaterialPass::Lighting);
	if (state.program)
	{
		state.vertexBufferInfo = m_ScreenQuad.GetVertexBufferInfoHandle();
		state.rasterState = m_MaterialInstance->GetMaterial()->GetRasterState();
		state.stencilState = m_MaterialInstance->GetMaterial()->GetStencilState();
		state.primitiveType = RHI::PrimitiveType::TRIANGLES;

		driver.draw(state, m_ScreenQuad.GetRenderPrimitiveHandle(),
			m_ScreenQuad.GetIndexOffset(), m_ScreenQuad.GetIndexCount(), 1);
	}

	driver.endRenderPass();
}
