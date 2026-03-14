#include "ShadowPass.h"

#include <glm/gtc/quaternion.hpp>

#include "Actor.h"
#include "Renderer.h"
#include "Scene.h"
#include "Component/ActorComponent.h"
#include "FrameBuffer/FrameBuffer.h"
#include "Lights/Light.h"
#include "Model/StaticMesh.h"
#include "Shader/Shader.h"
#include "Shader/ShaderLibrary.h"

ShadowPass::ShadowPass(uint32 width, uint32 height)
	: RenderPass(width, height)
{
	m_Shader = ShaderLibrary::Get().GetShader("Shaders/ShadowPass", nullptr);
}

ShadowPass::~ShadowPass()
{
}

void ShadowPass::Setup(RenderContext& ctx, FBAttachmentInfo& info)
{
	info.Width = ctx.ShadowWidth; 
	info.Height = ctx.ShadowHeight;
	info.NumSamples = 1;

	info.DSS.depthTest =true;
	info.DSS.depthWrite = true;
	info.DSS.compareFunc = ECompareFunc::Less;

	info.Depth = { 
		&ctx.ShadowMap_Depth, 
		CreateShadowMap(ctx.ShadowWidth), 
		FBTextureLoadAction::Clear, 
		FBTextureStoreAction::Store 
	};

	// info.Attachments = {
	// 	{ &ctx.Test, CreateGBuffer(ctx.ShadowWidth, ctx.ShadowHeight, ETextureFormat::RGBA16F, false), FBTextureLoadAction::Clear, FBTextureStoreAction::Store },
	// }; 
}

void ShadowPass::Execute(Ref<Scene> scene)
{
	m_Shader->Bind();

	RenderContext& ctx = scene->GetRenderContext();
	LightData& data = ctx.LightDataUB->Data;

	for (const auto& Actor : scene->GetActors())
	{
		for (auto& Comp : Actor->GetComponents())
		{
			if (auto Light = std::dynamic_pointer_cast<DirectionLightComponent>(Comp))
			{
				data.uLightVP = Light->GetViewProjectMatrix(10);
				break;
			}  
		}
	}
	ctx.LightDataUB->Update();

	for (const auto& Actor : scene->GetActors())
	{
		if (!Actor->IsCastShadow()) continue;
		for (auto& Comp : Actor->GetComponents())
		{
			if (auto MeshComp = std::dynamic_pointer_cast<StaticMeshComponent>(Comp))
			{
				if (MeshComp->GetMesh())
				for (auto& section : MeshComp->GetMesh()->GetMeshSections())
				{
					m_Shader->SetUniformMatrix4f("uModel", MeshComp->GetModelMatrix());
					section->Draw();
				}
			}
		}
	}
}