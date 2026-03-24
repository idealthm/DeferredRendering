#include "EnvPreFilter.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "Shader/ShaderLibrary.h"
#include "ShaderPreprocessor/ShaderLoader.h"
#include "Shapes/MeshBuilder.h"

#define MAX_MIPMAP_LEVEL 5
#define ENV_CUBE_SIZE 256

EnvPreFilter::EnvPreFilter()
{
	m_Desc.width = ENV_CUBE_SIZE;
	m_Desc.height = ENV_CUBE_SIZE;
	m_Desc.bGenerateMipmap = true;
	m_Desc.MipLevel = MAX_MIPMAP_LEVEL;
	m_Desc.format = ETextureFormat::RGBA16F;
	m_Desc.SRGB = true;
	m_Desc.FilterS = ETextureWrapMode::EClampToEdge;
	m_Desc.FilterT = ETextureWrapMode::EClampToEdge;
	m_Desc.FilterR = ETextureWrapMode::EClampToEdge;
	m_Desc.minFilter = ETextureFilter::LinearMipmapLinear;
	m_Desc.magFilter = ETextureFilter::Linear;

	m_Shader = ShaderLibrary::Get().GetShader("Shaders/Passes/EnvPreFilter", nullptr);
}

uint32 EnvPreFilter::GetRenderTimes()
{
	return 6 * MAX_MIPMAP_LEVEL;
}

void EnvPreFilter::Setup(FBAttachmentInfo& info, uint32 step)
{
	uint32 mip = step % MAX_MIPMAP_LEVEL;
	step /= MAX_MIPMAP_LEVEL;

	info.Width = ENV_CUBE_SIZE >> mip;
	info.Height = ENV_CUBE_SIZE >> mip;

	CreateResource<TextureCube, true>(g_ctx.IBL_PreFilterMap, m_Desc);
	ETextureTarget target = (ETextureTarget)((uint32)ETextureTarget::Positive_X + step);

	info.Attachments = {
		{g_ctx.IBL_PreFilterMap->GetRendererID(), target, FBTextureLoadAction::Load, FBTextureStoreAction::Store, mip}
	};
}

void EnvPreFilter::Execute(Ref<Scene> scene, uint32 step)
{
	m_Shader->Bind();
	uint32 FreeSlotIndex = m_Shader->GetFreeSlotIndex();
	g_ctx.ERP_Cubemap->Bind(FreeSlotIndex);
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
