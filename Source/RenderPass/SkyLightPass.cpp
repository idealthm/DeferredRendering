#include "SkyLightPass.h"

#include "Actor.h"
#include "Renderer.h"
#include "Scene.h"
#include "Component/SkyComponent.h"
#include "FrameBuffer/FrameBuffer.h"
#include "Shader/Shader.h"
#include "glad/glad.h"
#include "Model/Texture.h"
#include "Shapes/MeshBuilder.h"

SkyLightPass::SkyLightPass(int width, int height)
	: RenderPass(width, height)
{
	// CreateRef<Shader>("Shaders/Passes/SkyLightPass", 1);
}

void SkyLightPass::Setup(RenderContext& ctx, FBAttachmentInfo& info)
{
	RenderPass::Setup(ctx, info);
}

void SkyLightPass::Execute(Ref<Scene> scene)
{
	glDepthMask(GL_FALSE);

	m_Shader->Bind();

	uint32 freeSlotIndex = m_Shader->GetFreeSlotIndex();
	for (auto Actor : scene->GetActors())
	{
		if (auto comp = Actor->GetComponent<SkyComponent>())
		{
			comp->GetCubeMapTexture()->Bind(freeSlotIndex);
			m_Shader->SetUniform1i("uCubeMap", freeSlotIndex++);
		}
	}
	
}

