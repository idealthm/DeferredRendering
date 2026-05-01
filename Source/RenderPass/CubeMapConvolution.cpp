#include "CubeMapConvolution.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "RHI/SamplerPool.h"
#include "Shader/ShaderLibrary.h"
#include "ShaderPreprocessor/ShaderLoader.h"
#include "Shapes/MeshBuilder.h"

CubeMapConvolution::CubeMapConvolution()
{
	m_Shader = ShaderLibrary::Get().GetShader("Shaders/Passes/CubeMapConvolution", nullptr);
}

void CubeMapConvolution::Setup(FBAttachmentInfo& info, uint32_t step, RenderContext& ctx)
{
	info.Width = 32;
	info.Height = 32;

	RHI::TextureDesc desc;
	desc.Width = 32;
	desc.Height = 32;
	desc.MipLevels = 5;
	desc.Format = RHI::Format::RGBA16F;
	desc.Target = RHI::Sampler::DimCube;
	CreateResource(ctx.IBL_IrradianceMap, desc);
	if (!ctx.IBL_IrradianceMap->GetSampler())
	{
		RHI::SamplerParams samplerParams{};
		samplerParams.wrapS = RHI::SamplerWrapMode::ClampToEdge;
		samplerParams.wrapT = RHI::SamplerWrapMode::ClampToEdge;
		samplerParams.wrapR = RHI::SamplerWrapMode::ClampToEdge;
		samplerParams.filterMin = RHI::SamplerMinFilter::Linear;
		samplerParams.filterMag = RHI::SamplerMagFilter::Linear;
		ctx.IBL_IrradianceMap->SetSampler(SamplerPool::Get().GetOrCreate(samplerParams));
	}
	ETextureTarget target = (ETextureTarget)((uint32_t)ETextureTarget::Positive_X + step);

	info.Attachments = {
		{ctx.IBL_IrradianceMap->GetRendererID(), target, FBTextureLoadAction::Load, FBTextureStoreAction::Store}
	};
}

void CubeMapConvolution::Execute(Ref<Scene> scene, uint32_t step, RenderContext& ctx)
{
	m_Shader->Bind();
	uint32_t FreeSlotIndex = m_Shader->GetFreeSlotIndex();
	ctx.ERP_Cubemap->Bind(FreeSlotIndex);
	m_Shader->SetUniform1i("uCubeMap", FreeSlotIndex++);

	// 投影矩阵：90度 FOV，1:1 宽高比
	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

	// 6 个方向的 View 矩阵（注意：GL 的 CubeMap 采样比较特殊，Up 向量需要反转）
	glm::mat4 captureViews[] = {
		glm::lookAt(glm::vec3(0,0,0), glm::vec3( 1, 0, 0), glm::vec3(0,-1, 0)), // +X
		glm::lookAt(glm::vec3(0,0,0), glm::vec3(-1, 0, 0), glm::vec3(0,-1, 0)), // -X
		glm::lookAt(glm::vec3(0,0,0), glm::vec3( 0, 1, 0), glm::vec3(0, 0, 1)), // +Y
		glm::lookAt(glm::vec3(0,0,0), glm::vec3( 0,-1, 0), glm::vec3(0, 0,-1)), // -Y
		glm::lookAt(glm::vec3(0,0,0), glm::vec3( 0, 0, 1), glm::vec3(0,-1, 0)), // +Z
		glm::lookAt(glm::vec3(0,0,0), glm::vec3( 0, 0,-1), glm::vec3(0,-1, 0))  // -Z
	};

	m_Shader->SetUniformMatrix4f("uProjection", captureProjection);
	m_Shader->SetUniformMatrix4f("uView", captureViews[step]);

	MeshBuilder::BuildCube(Material::CreateDefault())->GetMeshSections()[0]->Draw();
}
