#include "CubeMapConvolution.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "Shader/ShaderLibrary.h"
#include "ShaderPreprocessor/ShaderLoader.h"
#include "Shapes/MeshBuilder.h"

CubeMapConvolution::CubeMapConvolution()
{
	m_Shader = ShaderLibrary::Get().GetShader("Shaders/Passes/ERPToCubeMap", nullptr);
}

void CubeMapConvolution::Setup(FBAttachmentInfo& info, uint32 step)
{
	info.Width = 32;
	info.Height = 32;

	TextureDescription desc;
	desc.width = 32;
	desc.height = 32;
	desc.bGenerateMipmap = true;
	desc.MipLevel = 5;
	desc.format = ETextureFormat::RGBA16F;
	desc.SRGB = true;
	desc.FilterS = ETextureWrapMode::EClampToEdge;
	desc.FilterT = ETextureWrapMode::EClampToEdge;
	desc.FilterR = ETextureWrapMode::EClampToEdge;

	CreateResource(g_ctx.IBL_IrradianceMap, desc);
	ETextureTarget target = (ETextureTarget)((uint32)ETextureTarget::Positive_X + step);

	info.Attachments = {
		{g_ctx.IBL_IrradianceMap->GetRendererID(), target, FBTextureLoadAction::Load, FBTextureStoreAction::Store}
	};
}

void CubeMapConvolution::Execute(Ref<Scene> scene, uint32 step)
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

	m_Shader->SetUniformMatrix4f("uProjection", captureProjection);
	m_Shader->SetUniformMatrix4f("uView", captureViews[step]);

	MeshBuilder::BuildCube(Material::CreateDefault())->GetMeshSections()[0]->Draw();
}
