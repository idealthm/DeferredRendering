#include "DeferredPass.h"

#include <iostream>
#include <glad/glad.h>

#include "Actor.h"
#include "Scene.h"
#include "Component/ActorComponent.h"
#include "Shader/Shader.h"


DeferredPass::~DeferredPass()
{
	glDeleteFramebuffers(1, &gBuffer);
	glDeleteTextures(1, &gPosition);
	glDeleteTextures(1, &gNormal);
	glDeleteTextures(1, &gAlbedo);
	glDeleteRenderbuffers(1, &rboDepth);
}

bool DeferredPass::Init(std::shared_ptr<Scene>& scene)
{
	RenderPass::Init(scene);

	GeometryShader = std::make_unique<Shader>("Shaders/Basic.glsl");
	LightShader = std::make_unique<Shader>("Shaders/DirectionLight.glsl");

	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, scene->GetWidth(), scene->GetHeight(), 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, scene->GetWidth(), scene->GetHeight(), 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

	glGenTextures(1, &gAlbedo);
	glBindTexture(GL_TEXTURE_2D, gAlbedo);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, scene->GetWidth(), scene->GetHeight(), 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);

	GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);

	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, scene->GetWidth(), scene->GetHeight());
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "G-Buffer is not complete!\n";
		return false;
	}

	return true;
}

void DeferredPass::PrePass(std::shared_ptr<Scene>& scene)
{
	RenderPass::PrePass(scene);
}

void DeferredPass::OnPass(std::shared_ptr<Scene>& scene)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GeometryShader->Bind();
	// GeometryShader->SetUniformMatrix4f("model", glm::mat4(1));
	GeometryShader->SetUniformMatrix4f("view", scene->GetViewMatrix());
	GeometryShader->SetUniformMatrix4f("projection", scene->GetProjectionMatrix());

	for (const auto& Actor : scene->GetActors())
	{
		for (auto& Comp : Actor->GetComponents())
		{
			if (auto MeshComp = std::dynamic_pointer_cast<SceneComponent>(Comp))
			{
				GeometryShader->SetUniformMatrix4f("model", MeshComp->GetModelMatrix());
				MeshComp->Draw(*GeometryShader);
			}
		}
	}

	// ourModel.Draw(*GeometryShader);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	LightShader->Bind();

	LightShader->SetUniform1i(std::string("gPosition"), 0);
	LightShader->SetUniform1i(std::string("gNormal"), 1);
	LightShader->SetUniform1i(std::string("gAlbedo"), 2);

	LightShader->SetUniform3f("lightPos", {100.f, 100.f, 100.f});
	LightShader->SetUniform3f("lightColor", {1.f, 1.f, 1.f});

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gAlbedo);

	QuadActor quadActor;
	quadActor.GetRootComponent()->Draw(*LightShader);
}

void DeferredPass::PostPass(std::shared_ptr<Scene>& scene)
{
	RenderPass::PostPass(scene);
}
