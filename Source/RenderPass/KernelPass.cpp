#include "KernelPass.h"

#include "Engine.h"
#include "Material/MaterialInstance.h"
#include "Material/MaterialLibrary.h"
#include "Model/Texture.h"
#include "RHI/PipelineState.h"
#include "RHI/TextureSampler.h"
#include "RHI/TargetBufferInfo.h"
#include "RenderTarget.h"

void KernelPass::Generate(uint32_t numLevels, uint32_t sampleCount, bool irradiance, Ref<Texture>& outKernel)
{
	auto material = MaterialLibrary::Get().GetMaterial("generateKernel");
	assert(material);
	Ref<MaterialInstance> mi = CreateRef<MaterialInstance>(material);

	CreateResource(outKernel, RHI::TextureDesc{
		numLevels, sampleCount, 1, 1,
		RHI::Format::RGBA16F,
		RHI::SamplerType::SAMPLER_2D,
		RHI::TextureUsage::COLOR_ATTACHMENT | RHI::TextureUsage::SAMPLEABLE
	});

	RenderTarget::Builder builder;
	builder.texture(AttachmentPoint::COLOR0, outKernel);
	Ref<RenderTarget> rt = builder.Build();

	auto& driver = gEngine->GetDriver();

	RHI::RenderPassParams rpParams{};
	rpParams.viewport = { 0, 0, numLevels, sampleCount };
	rpParams.flags.clear = RHI::TargetBufferFlags::COLOR0;
	rpParams.clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };

	mi->SetParameter("size", glm::uvec2(numLevels, sampleCount));
	mi->SetParameter("sampleCount", float(sampleCount));
	mi->SetParameter("irradiance", irradiance);
	mi->Commit(driver);
	mi->Use(driver);

	ScreenQuad& quad = gEngine->GetScreenQuad();

	RHI::PipelineState state;
	state.program = mi->GetShader(MaterialPass::PostProcess);
	state.vertexBufferInfo = quad.GetVertexBufferInfoHandle();
	state.primitiveType = RHI::PrimitiveType::TRIANGLES;
	state.rasterState.depthWrite = false;

	if (!state.program) return;

	driver.beginRenderPass(rt->GetHandle(), rpParams);

	driver.draw(state, quad.GetRenderPrimitiveHandle(),
		quad.GetIndexOffset(), quad.GetIndexCount(), 1);

	driver.endRenderPass();
}
