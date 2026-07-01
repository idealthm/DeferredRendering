#include "ToneMapping.h"

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

using namespace TextureFactory;

ToneMapping::ToneMapping()
{
	auto material = MaterialLibrary::Get().GetMaterial("ToneMapping");
	if (material)
		m_MaterialInstance = CreateRef<MaterialInstance>(material);
}

void ToneMapping::Setup(RenderContext& ctx)
{
	CreateResource(ctx.Final_SceneColor, RHI::TextureDesc{
		ctx.viewportSize.x, ctx.viewportSize.y, 1, 1,
		RHI::Format::RGBA8,
		RHI::SamplerType::SAMPLER_2D,
		RHI::TextureUsage::COLOR_ATTACHMENT | RHI::TextureUsage::SAMPLEABLE
	});

	RenderTarget::Builder builder;
	builder.texture(AttachmentPoint::COLOR0, ctx.Final_SceneColor);
	m_RenderTarget = builder.Build();
}

void ToneMapping::Execute(Ref<Scene> scene, RenderContext& ctx)
{
	if (!m_MaterialInstance) return;

	auto& driver = gEngine->GetDriver();

	m_MaterialInstance->SetParameter("lightMap", ctx.LightMap_SceneColor,
		TextureSampler::LinearClamp());
	m_MaterialInstance->Commit(driver);
	m_MaterialInstance->Use(driver);

	auto program = m_MaterialInstance->GetShader(MaterialPass::PostProcess);
	if (!program) return;

	RHI::RenderPassParams rpParams{};
	rpParams.viewport = { 0, 0, ctx.viewportSize.x, ctx.viewportSize.y };
	rpParams.flags.clear = RHI::TargetBufferFlags::COLOR0;
	rpParams.clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

	driver.beginRenderPass(m_RenderTarget->GetHandle(), rpParams);

	RHI::PipelineState state;
	state.program = program;
	state.vertexBufferInfo = m_ScreenQuad.GetVertexBufferInfoHandle();
	state.primitiveType = RHI::PrimitiveType::TRIANGLES;

	driver.draw(state, m_ScreenQuad.GetRenderPrimitiveHandle(),
		m_ScreenQuad.GetIndexOffset(), m_ScreenQuad.GetIndexCount(), 1);

	driver.endRenderPass();
}
