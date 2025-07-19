#include "ShadowPass.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Actor.h"
#include "Renderer.h"
#include "Component/ActorComponent.h"
#include "FrameBuffer/FrameBuffer.h"
#include "glad/glad.h"
#include "Lights/Light.h"
#include "Shader/Shader.h"

ShadowPass::ShadowPass(uint32 width, uint32 height)
	: RenderPass(width, height)
{
	FramebufferSpecification spec;
	spec.Width = width;
	spec.Height = height;
	spec.Attachments = {
		{"depth", EFBTextureFormat::DEPTH24STENCIL8}
	};
	m_ShadowMapFBO = CreateScope<FrameBuffer>(spec);
	uint32 depthid = m_ShadowMapFBO->GetDepthRendererID();
	glBindTexture(GL_TEXTURE_2D, depthid);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	m_ShadowMapShader = CreateScope<Shader>("Shaders/ShadowPass.glsl");
}

ShadowPass::~ShadowPass()
{
}

uint32 ShadowPass::GetDepthRendererID() const
{
	return m_ShadowMapFBO->GetDepthRendererID();
}

void ShadowPass::OnWindowSizeChanged(int32 width, int32 height)
{
	m_ShadowMapFBO->Resize(width, height);
}

void ShadowPass::PrePass(RenderContext& context)
{
	RenderPass::PrePass(context);
}

void ShadowPass::OnPass(RenderContext& context)
{
	m_ShadowMapFBO->Bind();
	glClear(GL_DEPTH_BUFFER_BIT);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE); 
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_ShadowMapShader->Bind();

	constexpr int32 width = 16;
	constexpr int32 height = 9;

	for (const auto& Actor : context.scene->GetActors())
	{
		for (auto& Comp : Actor->GetComponents())
		{
			if (auto Light = std::dynamic_pointer_cast<DirectionLightComponent>(Comp))
			{
				glm::quat rotation = glm::quat(glm::radians(Light->GetRotation()));
				glm::vec3 location = Light->GetLocation();
				glm::vec3 direction = rotation * glm::vec3(0.f, 0.f, -1.f);
				glm::vec3 Up = rotation * glm::vec3(0.f, 1.f, 0.f);
				m_ShadowMapShader->SetUniformMatrix4f("uView", glm::lookAt(location, location + direction, Up));
				m_ShadowMapShader->SetUniformMatrix4f("uProjection", glm::ortho<float>(-width, width, -height, height, -150.f, 150.f));
				break;
			}  
		}
	}

	for (const auto& Actor : context.scene->GetActors())
	{
		for (auto& Comp : Actor->GetComponents())
		{
			if (auto MeshComp = std::dynamic_pointer_cast<SceneComponent>(Comp))
			{
				m_ShadowMapShader->SetUniformMatrix4f("uModel", MeshComp->GetModelMatrix());
				MeshComp->Draw(*m_ShadowMapShader);
			}
		}
	}
}

void ShadowPass::PostPass(RenderContext& context)
{
	RenderPass::PostPass(context);
}
