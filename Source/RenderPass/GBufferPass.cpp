#include "GBufferPass.h"

#include "Actor.h"
#include "Renderer.h"
#include "Scene.h"
#include "Component/ActorComponent.h"
#include "FrameBuffer/FrameBuffer.h"
#include "glad/glad.h"
#include "Shader/Shader.h"

GBufferPass::GBufferPass(uint32 width, uint32 height)
	: RenderPass(width, height)
{
	m_GBufferShader = CreateScope<Shader>("Shaders/Basic.glsl");

	FramebufferSpecification spec;
	spec.Width = width;
	spec.Height = height;
	spec.Attachments = {
		FBTextureSpecification{std::string("gPosition"), EFBTextureFormat::RGBA16F},
		FBTextureSpecification{std::string("gNormal"), EFBTextureFormat::RG16},
		FBTextureSpecification{std::string("gAlbedo"), EFBTextureFormat::RGBA8},
		FBTextureSpecification{std::string("Depth"), EFBTextureFormat::Depth}
	};
	m_GBufferFBO = CreateScope<FrameBuffer>(spec);
}

uint32 GBufferPass::GetColorAttachmentRendererID(const std::string& name) const
{
	return m_GBufferFBO->GetColorAttachmentRendererID(name);
}

uint32 GBufferPass::GetDepthRendererID() const
{
	return m_GBufferFBO->GetDepthRendererID();
}

void GBufferPass::OnWindowSizeChanged(int32 width, int32 height)
{
	m_GBufferFBO->Resize(width, height);
}

void GBufferPass::PrePass(RenderContext& context)
{
	RenderPass::PrePass(context);
}

void GBufferPass::OnPass(RenderContext& context)
{
	m_GBufferFBO->Bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_GBufferShader->Bind();

	m_GBufferShader->SetUniformMatrix4f("uView", context.scene->GetViewMatrix());
	m_GBufferShader->SetUniformMatrix4f("uProjection", context.scene->GetProjectionMatrix());

	for (const auto& Actor : context.scene->GetActors())
	{
		for (auto& Comp : Actor->GetComponents())
		{
			if (auto MeshComp = std::dynamic_pointer_cast<SceneComponent>(Comp))
			{
				m_GBufferShader->SetUniformMatrix4f("uModel", MeshComp->GetModelMatrix());
				MeshComp->Draw(*m_GBufferShader);
			}
		}
	}
}

void GBufferPass::PostPass(RenderContext& context)
{
	RenderPass::PostPass(context);
}
