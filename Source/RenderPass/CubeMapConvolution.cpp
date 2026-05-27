#include "CubeMapConvolution.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "Model/Texture.h"
#include "Shader/Program.h"

CubeMapConvolution::CubeMapConvolution()
{
}

void CubeMapConvolution::Setup(RenderContext& ctx)
{
	
	RHI::TextureDesc desc;
	desc.Width = 32;
	desc.Height = 32;
	desc.LevelCount = 5;
	desc.Format = RHI::Format::RGBA16F;
	desc.Target = RHI::SamplerType::SAMPLER_CUBEMAP;
	CreateResource(ctx.IBL_IrradianceMap, desc);
}

void CubeMapConvolution::Execute(Ref<Scene> scene, RenderContext& ctx)
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
