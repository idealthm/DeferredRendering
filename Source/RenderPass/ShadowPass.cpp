#include "ShadowPass.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Actor.h"
#include "Renderer.h"
#include "Scene.h"
#include "Component/ActorComponent.h"
#include "FrameBuffer/FrameBuffer.h"
#include "glad/glad.h"
#include "Lights/Light.h"
#include "Shader/Shader.h"

ShadowPass::ShadowPass(uint32 width, uint32 height)
	: RenderPass(width, height)
{
	// m_Shader = CreateScope<Shader>("Shaders/ShadowPass");
}

ShadowPass::~ShadowPass()
{
}

void ShadowPass::Setup(RenderContext& ctx, FBAttachmentInfo& info)
{
	info.Width = 2048; 
	info.Height = 2048;
	info.NumSamples = 1;

	info.Depth = { 
		&ctx.ShadowMap_Depth, 
		CreateShadowMap(2048), 
		FBTextureLoadAction::Clear, 
		FBTextureStoreAction::Store 
	};

	info.Attachments = {}; 
}

void ShadowPass::Execute(Ref<Scene> scene)
{
	for (const auto& Actor : scene->GetActors())
	{
		for (auto& Comp : Actor->GetComponents())
		{
			if (auto Light = std::dynamic_pointer_cast<DirectionLightComponent>(Comp))
			{
				// glm::quat rotation = glm::quat(glm::radians(Light->GetRotation()));
				// glm::vec3 location = Light->GetLocation();
				// glm::vec3 direction = rotation * glm::vec3(0.f, 0.f, -1.f);
				// glm::vec3 Up = rotation * glm::vec3(0.f, 1.f, 0.f);
				// m_Shader->SetUniformMatrix4f("uView", glm::lookAt(location, location + direction, Up));
				// m_Shader->SetUniformMatrix4f("uProjection", glm::ortho<float>(-width, width, -height, height, -150.f, 150.f));
				break;
			}  
		}
	}

	for (const auto& Actor : scene->GetActors())
	{
		for (auto& Comp : Actor->GetComponents())
		{
			if (auto MeshComp = std::dynamic_pointer_cast<SceneComponent>(Comp))
			{
				// m_Shader->SetUniformMatrix4f("uModel", MeshComp->GetModelMatrix());
				// MeshComp->Draw(*m_Shader);
			}
		}
	}
}