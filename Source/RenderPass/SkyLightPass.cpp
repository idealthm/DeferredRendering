#include "SkyLightPass.h"

#include "Engine.h"
#include "Material/Material.h"
#include "Material/MaterialInstance.h"
#include "Material/MaterialLibrary.h"
#include "Model/Texture.h"
#include "RHI/PipelineState.h"
#include "RHI/TargetBufferInfo.h"
#include "RHI/TextureSampler.h"
#include "RenderTarget.h"
#include "Scene.h"
#include "Shader/Program.h"
#include "Shapes/ScreenQuad.h"

using namespace TextureFactory;

SkyLightPass::SkyLightPass()
{
	auto material = MaterialLibrary::Get().GetMaterial("skyBox");
	if (material)
		m_MaterialInstance = CreateRef<MaterialInstance>(material);
}

void SkyLightPass::Setup(RenderContext& ctx)
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

void SkyLightPass::Execute(Ref<Scene> scene, RenderContext& ctx)
{
	if (!m_MaterialInstance) return;

	auto& driver = gEngine->GetDriver();

	m_MaterialInstance->SetParameter("skybox", ctx.IBL_PreFilterMap,
		TextureSampler::LinearMipmapRepeat());
	m_MaterialInstance->SetParameter("depthMap", ctx.GBuffer_Depth,
		TextureSampler::NearestClamp());
	m_MaterialInstance->Commit(driver);
	m_MaterialInstance->Use(driver);

	auto program = m_MaterialInstance->GetShader(MaterialPass::PostProcess);
	if (!program) return;

	RHI::RenderPassParams rpParams{};
	rpParams.viewport = { 0, 0, ctx.viewportSize.x, ctx.viewportSize.y };
	rpParams.flags.clear = RHI::TargetBufferFlags::COLOR0;
	rpParams.clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

	driver.beginRenderPass(m_RenderTarget->GetHandle(), rpParams);

	auto& quad = gEngine->GetScreenQuad();

	RHI::PipelineState state;
	state.program = program;
	state.vertexBufferInfo = quad.GetVertexBufferInfoHandle();
	state.primitiveType = RHI::PrimitiveType::TRIANGLES;

	driver.draw(state, quad.GetRenderPrimitiveHandle(),
		quad.GetIndexOffset(), quad.GetIndexCount(), 1);

	driver.endRenderPass();
}
