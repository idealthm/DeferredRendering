#include "ERPPass.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "Actor.h"
#include "Renderer.h"
#include "Component/ActorComponent.h"
#include "FrameBuffer/FrameBuffer.h"
#include "glad/glad.h"
#include "Material/Material.h"
#include "Model/StaticMesh.h"
#include "Model/Texture.h"
#include "Shader/Shader.h"
#include "Shader/ShaderLibrary.h"
#include "Shapes/MeshBuilder.h"

ERPPass::ERPPass(const std::string& hdrFilePath, uint32 size)
{
	m_Shader = CreateRef<Shader>("Shaders/Passes/ERPToCubeMap", 10, nullptr);

	m_HDRMap = CreateRef<Texture2D>(hdrFilePath, false);
}

ERPPass::~ERPPass()
{
}

void ERPPass::Setup(FBAttachmentInfo& info, uint32 step)
{
	info.Width = 512;
	info.Height = 512;

	TextureDescription desc;
	desc.width = 512;
	desc.height = 512;
	desc.bGenerateMipmap = true;
	desc.MipLevel = 7;
	desc.format = ETextureFormat::RGBA16F;
	desc.SRGB = true;
	desc.FilterS = ETextureWrapMode::EClampToEdge;
	desc.FilterT = ETextureWrapMode::EClampToEdge;
	desc.FilterR = ETextureWrapMode::EClampToEdge;

	CreateResource(g_ctx.ERP_Cubemap, desc);
	ETextureTarget target = (ETextureTarget)((uint32)ETextureTarget::Positive_X + step);

	info.Attachments = {
		{g_ctx.ERP_Cubemap->GetRendererID(), target, FBTextureLoadAction::Load, FBTextureStoreAction::Store}
	};
}

void ERPPass::Execute(Ref<Scene> scene, uint32 step)
{
	m_Shader->Bind();
	uint32 FreeSlotIndex = m_Shader->GetFreeSlotIndex();
	m_HDRMap->Bind(FreeSlotIndex);
	m_Shader->SetUniform1i("uHDRMap", FreeSlotIndex++);

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
