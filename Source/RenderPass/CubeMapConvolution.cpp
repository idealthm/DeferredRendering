#include "CubeMapConvolution.h"

#include "Engine.h"
#include "Material/MaterialInstance.h"
#include "Material/MaterialLibrary.h"
#include "Model/Texture.h"
#include "RHI/PipelineState.h"
#include "RHI/TextureSampler.h"
#include "RHI/TargetBufferInfo.h"
#include "RenderTarget.h"

using namespace TextureFactory;

void CubeMapConvolution::RenderCubemapFaces(Ref<MaterialInstance>& mi, Ref<Texture>& srcCubeMap,
	Ref<Texture>& kernelTex, Ref<Texture>& dstCubeMap,
	uint32_t mipLevel, uint32_t sampleCount)
{
	using RHI::TextureCubemapFace;

	const TextureCubemapFace faces[2][3] = {
		{ TextureCubemapFace::POSITIVE_X, TextureCubemapFace::POSITIVE_Y, TextureCubemapFace::POSITIVE_Z },
		{ TextureCubemapFace::NEGATIVE_X, TextureCubemapFace::NEGATIVE_Y, TextureCubemapFace::NEGATIVE_Z }
	};

	mi->SetParameter("sampleCount", sampleCount);
	mi->SetParameter("environment", srcCubeMap, TextureSampler::LinearMipmapRepeat());
	mi->SetParameter("kernel", kernelTex, TextureSampler::NearestClamp());

	uint32_t const faceSize = dstCubeMap->GetSizeX(0);

	RenderTarget::Builder builder;
	builder.texture(AttachmentPoint::COLOR0, dstCubeMap);
	builder.texture(AttachmentPoint::COLOR1, dstCubeMap);
	builder.texture(AttachmentPoint::COLOR2, dstCubeMap);
	builder.mipLevel(AttachmentPoint::COLOR0, static_cast<uint8_t>(mipLevel));
	builder.mipLevel(AttachmentPoint::COLOR1, static_cast<uint8_t>(mipLevel));
	builder.mipLevel(AttachmentPoint::COLOR2, static_cast<uint8_t>(mipLevel));

	auto& driver = gEngine->GetDriver();

	RHI::RenderPassParams rpParams{};
	rpParams.flags.clear = RHI::TargetBufferFlags::COLOR0;
	rpParams.clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

	ScreenQuad& quad = gEngine->GetScreenQuad();

	RHI::PipelineState state;
	state.program = mi->GetShader(MaterialPass::PostProcess);
	state.vertexBufferInfo = quad.GetVertexBufferInfoHandle();
	state.primitiveType = RHI::PrimitiveType::TRIANGLES;

	if (!state.program) return;

	for (int i = 0; i < 2; i++)
	{
		mi->SetParameter("side", i == 0 ? 1.0f : -1.0f);

		builder.face(AttachmentPoint::COLOR0, faces[i][0])
		       .face(AttachmentPoint::COLOR1, faces[i][1])
		       .face(AttachmentPoint::COLOR2, faces[i][2]);

		Ref<RenderTarget> rt = builder.Build();

		rpParams.viewport = { 0, 0, faceSize >> mipLevel, faceSize >> mipLevel};
		
		driver.beginRenderPass(rt->GetHandle(), rpParams);

		mi->Commit(driver);
		mi->Use(driver);

		driver.draw(state, quad.GetRenderPrimitiveHandle(),
			quad.GetIndexOffset(), quad.GetIndexCount(), 1);

		driver.endRenderPass();
	}
}

void CubeMapConvolution::RenderIrradiance(Ref<Texture>& kernelTexture, Ref<Texture>& srcCubeMap,
	Ref<Texture>& dstCubeMap, uint32_t sampleCount)
{
	auto material = MaterialLibrary::Get().GetMaterial("IBLPrefilter");
	assert(material);
	Ref<MaterialInstance> mi = CreateRef<MaterialInstance>(material);

	mi->SetParameter("irradiance", true);
	mi->SetParameter("attachmentLevel", 0.0f);
	mi->SetParameter("lodOffset", 0.0f);
	mi->SetParameter("compress", glm::vec2{1024.f, 16384.f});

	RenderCubemapFaces(mi, srcCubeMap, kernelTexture, dstCubeMap, 0, sampleCount);
}

void CubeMapConvolution::RenderPrefilter(Ref<Texture>& kernelTexture, Ref<Texture>& srcCubeMap,
	Ref<Texture>& dstCubeMap, uint32_t mipLevel, uint32_t sampleCount)
{
	auto material = MaterialLibrary::Get().GetMaterial("IBLPrefilter");
	assert(material);
	Ref<MaterialInstance> mi = CreateRef<MaterialInstance>(material);

	mi->SetParameter("irradiance", false);
	mi->SetParameter("attachmentLevel", float(mipLevel));
	mi->SetParameter("lodOffset", 0.0f);
	mi->SetParameter("compress", glm::vec2{1024.f, 16384.f});

	RenderCubemapFaces(mi, srcCubeMap, kernelTexture, dstCubeMap, mipLevel, sampleCount);
}
