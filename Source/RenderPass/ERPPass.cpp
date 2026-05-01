#include "ERPPass.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "Actor.h"
#include "Renderer.h"
#include "Component/ActorComponent.h"
#include "FrameBuffer/FrameBuffer.h"
#include "glad/glad.h"
#include "Model/Texture.h"
#include "RHI/RHITypes.h"
#include "RHI/SamplerPool.h"
#include "Shader/Shader.h"
#include "Shader/ShaderLibrary.h"

ERPPass::ERPPass(const std::string& hdrFilePath, uint32_t size)
{
	m_Shader = ShaderLibrary::Get().GetShader("Shaders/Passes/ERPToCubeMap", nullptr);

	m_HDRMap = CreateRef<Texture2D>(hdrFilePath, false);
}

ERPPass::~ERPPass()
{
}

void ERPPass::Setup(FBAttachmentInfo& info, uint32_t step, RenderContext& ctx)
{
	info.Width = 512;
	info.Height = 512;

	RHI::TextureDesc desc;
	desc.Width = 512;
	desc.Height = 512;
	desc.MipLevels = 7;
	desc.Format = RHI::Format::RGBA16F;
	desc.Target = RHI::Sampler::DimCube;
	CreateResource(ctx.ERP_Cubemap, desc);
	if (!ctx.ERP_Cubemap->GetSampler())
	{
		RHI::SamplerParams samplerParams{};
		samplerParams.wrapS = RHI::SamplerWrapMode::ClampToEdge;
		samplerParams.wrapT = RHI::SamplerWrapMode::ClampToEdge;
		samplerParams.wrapR = RHI::SamplerWrapMode::ClampToEdge;
		samplerParams.filterMin = RHI::SamplerMinFilter::Linear;
		samplerParams.filterMag = RHI::SamplerMagFilter::Linear;
		ctx.ERP_Cubemap->SetSampler(SamplerPool::Get().GetOrCreate(samplerParams));
	}
	ETextureTarget target = (ETextureTarget)((uint32_t)ETextureTarget::Positive_X + step);

	info.Attachments = {
		{ctx.ERP_Cubemap->GetRendererID(), target, FBTextureLoadAction::Load, FBTextureStoreAction::Store}
	};
}

void ERPPass::Execute(Ref<Scene> scene, uint32_t step, RenderContext& ctx)
{
	m_Shader->Bind();
	uint32_t FreeSlotIndex = m_Shader->GetFreeSlotIndex();
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

	m_UnitCube.Draw();

	if (step == 5)
	{
		ctx.ERP_Cubemap->GenerateMipmap();
	}
}
