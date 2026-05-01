#include "EnvPreFilter.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "RHI/SamplerPool.h"
#include "Shader/ShaderLibrary.h"
#include "ShaderPreprocessor/ShaderLoader.h"
#include "Shapes/MeshBuilder.h"

#define MAX_MIPMAP_LEVEL 5
#define ENV_CUBE_SIZE 256

EnvPreFilter::EnvPreFilter()
{
	m_Desc.Width = ENV_CUBE_SIZE;
	m_Desc.Height = ENV_CUBE_SIZE;
	m_Desc.MipLevels = MAX_MIPMAP_LEVEL;
	m_Desc.Format = RHI::Format::RGBA16F;
	m_Desc.Target = RHI::Sampler::DimCube;
	m_Shader = ShaderLibrary::Get().GetShader("Shaders/Passes/EnvPreFilter", nullptr);
}

uint32_t EnvPreFilter::GetRenderTimes()
{
	return 6 * MAX_MIPMAP_LEVEL;
}

void EnvPreFilter::Setup(FBAttachmentInfo& info, uint32_t step, RenderContext& ctx)
{
	uint32_t mip = step % MAX_MIPMAP_LEVEL;
	step /= MAX_MIPMAP_LEVEL;

	info.Width = ENV_CUBE_SIZE >> mip;
	info.Height = ENV_CUBE_SIZE >> mip;

	CreateResource<TextureCube>(ctx.IBL_PreFilterMap, m_Desc);
	if (!ctx.IBL_PreFilterMap->GetSampler())
	{
		RHI::SamplerParams samplerParams{};
		samplerParams.wrapS = RHI::SamplerWrapMode::ClampToEdge;
		samplerParams.wrapT = RHI::SamplerWrapMode::ClampToEdge;
		samplerParams.wrapR = RHI::SamplerWrapMode::ClampToEdge;
		samplerParams.filterMin = RHI::SamplerMinFilter::LinearMipmapLinear;
		samplerParams.filterMag = RHI::SamplerMagFilter::Linear;
		ctx.IBL_PreFilterMap->SetSampler(SamplerPool::Get().GetOrCreate(samplerParams));
	}
	ETextureTarget target = (ETextureTarget)((uint32_t)ETextureTarget::Positive_X + step);

	info.Attachments = {
		{ctx.IBL_PreFilterMap->GetRendererID(), target, FBTextureLoadAction::Load, FBTextureStoreAction::Store, mip}
	};
}

void EnvPreFilter::Execute(Ref<Scene> scene, uint32_t step, RenderContext& ctx)
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

	m_Shader->SetUniform1f("uRoughness", 1.f * (step % MAX_MIPMAP_LEVEL) / (MAX_MIPMAP_LEVEL));
	m_Shader->SetUniformMatrix4f("uProjection", captureProjection);
	m_Shader->SetUniformMatrix4f("uView", captureViews[step / MAX_MIPMAP_LEVEL]);

	MeshBuilder::BuildCube(Material::CreateDefault())->GetMeshSections()[0]->Draw();
}
