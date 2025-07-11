#include "DeferredPass.h"

#include <iostream>
#include <glad/glad.h>

#include "Actor.h"
#include "Scene.h"
#include "Component/ActorComponent.h"
#include "FrameBuffer/FrameBuffer.h"
#include "Shader/Shader.h"
#include "Shapes/MeshBuilder.h"


enum TextureSlot
{
	COLOR = 0,
	POSITION = 1,
	NORMAL = 2,
	ALBEDO = 3,
};

DeferredPass::DeferredPass(int32 width, int32 height)
	: RenderPass(width, height)
{
	GeometryShader = CreateScope<Shader>("Shaders/Basic.glsl");
	LightShader = CreateScope<Shader>("Shaders/DirectionLight.glsl");

	FramebufferSpecification spec;
	spec.Width = width;
	spec.Height = height;
	spec.Attachments = {FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::Depth};
	m_FrameBuffer = CreateScope<FrameBuffer>(spec);
}

DeferredPass::~DeferredPass()
{
}

void DeferredPass::SetDebugMode(uint32 mode)
{
	DebugMode = mode;
}

void DeferredPass::OnWindowSizeChanged(int32 width, int32 height)
{
	m_FrameBuffer->Resize(width, height);
}

void DeferredPass::PrePass(const Ref<Scene>& scene)
{
	RenderPass::PrePass(scene);
}

void DeferredPass::OnPass(const Ref<Scene>& scene)
{
	m_FrameBuffer->Bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GeometryShader->Bind();
	// GeometryShader->SetUniformMatrix4f("model", glm::mat4(1));
	GeometryShader->SetUniformMatrix4f("uView", scene->GetViewMatrix());
	GeometryShader->SetUniformMatrix4f("uProjection", scene->GetProjectionMatrix());

	for (const auto& Actor : scene->GetActors())
	{
		for (auto& Comp : Actor->GetComponents())
		{
			if (auto MeshComp = std::dynamic_pointer_cast<SceneComponent>(Comp))
			{
				GeometryShader->SetUniformMatrix4f("uModel", MeshComp->GetModelMatrix());
				MeshComp->Draw(*GeometryShader);
			}
		}
	}

	// ourModel.Draw(*GeometryShader);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	LightShader->Bind();

	LightShader->SetUniform1i("uDebugMode", DebugMode);
	LightShader->SetUniform1i("uLightCount", 1);

	LightShader->SetUniform3f("pointLights[0].position", {100.f, 100.f, 100.f});
	LightShader->SetUniform3f("pointLights[0].diffuse", {1.f, 1.f, 1.f});
	LightShader->SetUniform3f("pointLights[0].ambient", glm::vec3{0.1f, 0.1f, 0.1f});
	LightShader->SetUniform3f("pointLights[0].specular", {0.0f, 0.0f, 0.0f});

	BindForReading(std::string("gPosition"), 0, 0);
	BindForReading(std::string("gNormal"), 1, 1);
	BindForReading(std::string("gAlbedo"), 2, 2);

	StaticMeshActor quadActor;
	quadActor.SetStaticMesh(MeshBuilder::BuildQuad());
	quadActor.GetRootComponent()->Draw(*LightShader);
}

void DeferredPass::PostPass(const Ref<Scene>& scene)
{
	RenderPass::PostPass(scene);
}

void DeferredPass::BindForReading(const std::string& name, int32 index, int32 slot) const
{
	LightShader->SetUniform1i(name, slot);
	GLCall(glBindTextureUnit(slot, m_FrameBuffer->GetColorAttachmentRendererID(index)));
}
