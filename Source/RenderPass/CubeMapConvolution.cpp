#include "CubeMapConvolution.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "Renderer.h"
#include "FrameBuffer/FrameBuffer.h"
#include "Model/Texture.h"
#include "Shader/Program.h"

CubeMapConvolution::CubeMapConvolution()
{
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
	desc.Target = RHI::SamplerType::SAMPLER_CUBEMAP;
	CreateResource(ctx.IBL_IrradianceMap, desc);
	ETextureTarget target = (ETextureTarget)((uint32_t)ETextureTarget::Positive_X + step);

		// {ctx.IBL_IrradianceMap->GetHandle(), target, FBTextureLoadAction::Load, FBTextureStoreAction::Store}
	info.Attachments = {
	};
}

void CubeMapConvolution::Execute(Ref<Scene> scene, uint32_t step, RenderContext& ctx)
{

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


}
