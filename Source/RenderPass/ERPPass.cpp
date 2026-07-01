#include "ERPPass.h"

#include "Engine.h"
#include "RenderTarget.h"
#include "Component/ActorComponent.h"
#include "Material/MaterialInstance.h"
#include "Material/MaterialLibrary.h"
#include "Model/Texture.h"
#include "RHI/PipelineState.h"
#include "RHI/TextureSampler.h"
#include "Shapes/ScreenQuad.h"

void ERPPass::Render(Ref<Texture> texture, Ref<Texture>& CubeMap)
{
	auto material = MaterialLibrary::Get().GetMaterial("EquirectToCube");
	assert(material);
	Ref<MaterialInstance> mi = CreateRef<MaterialInstance>(material);

	using RHI::TextureCubemapFace;

	const TextureCubemapFace faces[2][3] = {
		{ TextureCubemapFace::POSITIVE_X, TextureCubemapFace::POSITIVE_Y, TextureCubemapFace::POSITIVE_Z },
		{ TextureCubemapFace::NEGATIVE_X, TextureCubemapFace::NEGATIVE_Y, TextureCubemapFace::NEGATIVE_Z }
	};

	TextureSampler environmentSampler;
	environmentSampler.SetMagFilter(RHI::SamplerMagFilter::Linear);
	environmentSampler.SetMinFilter(RHI::SamplerMinFilter::LinearMipmapLinear);

	mi->SetParameter("equirect", texture, environmentSampler);

	texture->GenerateMipmaps();

	RenderTarget::Builder builder;
	builder.texture(AttachmentPoint::COLOR0, CubeMap);
	builder.texture(AttachmentPoint::COLOR1, CubeMap);
	builder.texture(AttachmentPoint::COLOR2, CubeMap);

	mi->SetParameter("mirror", 1.0f);

	RHI::RHIDriver& driver = gEngine->GetDriver();

	RHI::RenderPassParams rpParams{};
	rpParams.viewport = { 0, 0, 256, 256 };
	rpParams.flags.clear = RHI::TargetBufferFlags::COLOR0;
	rpParams.clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

	ScreenQuad& quad = gEngine->GetScreenQuad();

	RHI::PipelineState state;
	state.program = mi->GetShader(MaterialPass::PostProcess);
	state.vertexBufferInfo = quad.GetVertexBufferInfoHandle();
	state.primitiveType = RHI::PrimitiveType::TRIANGLES;

	if (!state.program) return;

	for (int i = 0; i < 2; i ++)
	{
		mi->SetParameter("side", i == 0 ? 1.0f : -1.0f);

		builder.face(AttachmentPoint::COLOR0, faces[i][0])
			   .face(AttachmentPoint::COLOR1, faces[i][1])
			   .face(AttachmentPoint::COLOR2, faces[i][2]);

		Ref<RenderTarget> rt = builder.Build();
		driver.beginRenderPass(rt->GetHandle(), rpParams);

		mi->Commit(driver);
		mi->Use(driver);

		driver.draw(state, quad.GetRenderPrimitiveHandle(),
			quad.GetIndexOffset(), quad.GetIndexCount(), 1);

		driver.endRenderPass();
	}
}