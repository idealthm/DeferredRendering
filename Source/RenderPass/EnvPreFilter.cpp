#include "EnvPreFilter.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "Shader/Program.h"

#define MAX_MIPMAP_LEVEL 5
#define ENV_CUBE_SIZE 256

EnvPreFilter::EnvPreFilter()
{
	m_Desc.Width = ENV_CUBE_SIZE;
	m_Desc.Height = ENV_CUBE_SIZE;
	m_Desc.LevelCount = MAX_MIPMAP_LEVEL;
	m_Desc.Format = RHI::Format::RGBA16F;
	m_Desc.Target = RHI::SamplerType::SAMPLER_CUBEMAP;
}

uint32_t EnvPreFilter::GetRenderTimes()
{
	return 6 * MAX_MIPMAP_LEVEL;
}

void EnvPreFilter::Setup(RenderContext& ctx)
{
	CreateResource<Texture>(ctx.IBL_PreFilterMap, m_Desc);
}

void EnvPreFilter::Execute(Ref<Scene> scene, RenderContext& ctx)
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
