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
	m_Shader = CreateRef<Shader>("Shaders/Passes/ERPToCubeMap", 0, nullptr);
	
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
	m_CubeMapTexture = CreateRef<TextureCube>(desc);
	m_HDRMap = CreateRef<Texture2D>(hdrFilePath, true);
}

ERPPass::~ERPPass()
{
}

void ERPPass::Setup(FBAttachmentInfo& info)
{
}

void ERPPass::Execute(Ref<Scene> scene)
{
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

	m_Shader->SetUniformMatrix4f("uPerspective", captureProjection);
	Scope<StaticMeshActor> Quad = CreateScope<StaticMeshActor>();
	Quad->SetStaticMesh(MeshBuilder::BuildCube(Material::CreateDefault()));

	for (int i = 0; i < 6; i++)
	{
		m_Shader->SetUniformMatrix4f("uView", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_CubeMapTexture->GetRendererID(), 0);
		Quad->GetComponent<StaticMeshComponent>()->GetMesh()->GetMeshSections()[0]->Draw();
	}
}
