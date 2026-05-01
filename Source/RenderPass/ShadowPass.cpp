#include "ShadowPass.h"

#include <glm/gtc/quaternion.hpp>

#include "Actor.h"
#include "Renderer.h"
#include "Scene.h"
#include "Component/ActorComponent.h"
#include "FrameBuffer/FrameBuffer.h"
#include "Lights/Light.h"
#include "Model/StaticMesh.h"
#include "Model/Texture.h"
#include "RHI/SamplerPool.h"
#include "Shader/Shader.h"
#include "Shader/ShaderLibrary.h"

ShadowPass::ShadowPass()
{
	m_Shader = ShaderLibrary::Get().GetShader("Shaders/ShadowPass", nullptr);
}

ShadowPass::~ShadowPass()
{
}

void ShadowPass::Setup(FBAttachmentInfo& info, uint32_t step, RenderContext& ctx)
{
	info.Width = ctx.ShadowWidth; 
	info.Height = ctx.ShadowHeight;
	info.NumSamples = 1;

	info.DSS.depthTest =true;
	info.DSS.depthWrite = true;
	info.DSS.compareFunc = ECompareFunc::Less;

	CreateResource(ctx.ShadowMap_Depth, CreateShadowMap(ctx.ShadowWidth));

	if (!ctx.ShadowMap_Depth->GetSampler())
		ctx.ShadowMap_Depth->SetSampler(SamplerPool::Get().GetOrCreate(ShadowMapSampler()));

	info.Depth = {
		ctx.ShadowMap_Depth->GetRendererID(),
		ETextureTarget::Texture2D,
		FBTextureLoadAction::Clear, 
		FBTextureStoreAction::Store
	};

	// info.Attachments = {
	// 	{ &ctx.Test, CreateGBuffer(ctx.ShadowWidth, ctx.ShadowHeight, ETextureFormat::RGBA16F, false), FBTextureLoadAction::Clear, FBTextureStoreAction::Store },
	// };
}

void ShadowPass::Execute(Ref<Scene> scene, uint32_t step, RenderContext& ctx)
{
	m_Shader->Bind();

	LightData& data = ctx.LightDataUB->Data;

	for (const auto& Actor : scene->GetActors())
	{
		for (auto& Comp : Actor->GetComponents())
		{
			if (auto Light = std::dynamic_pointer_cast<DirectionLightComponent>(Comp))
			{
				data.uLightVP = Light->GetViewProjectMatrix(50);
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