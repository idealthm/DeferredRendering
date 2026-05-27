#include "ShadowPass.h"

#include <glm/gtc/quaternion.hpp>

#include "Actor.h"
#include "Engine.h"
#include "Scene.h"
#include "Lights/Light.h"
#include "Model/Texture.h"

using namespace TextureFactory;

ShadowPass::ShadowPass()
{
}

ShadowPass::~ShadowPass()
{
}

void ShadowPass::Setup(RenderContext& ctx)
{
	CreateResource(ctx.ShadowMap_Depth, CreateShadowMap(ctx.ShadowWidth));

	// info.Attachments = {
	// 	{ &ctx.Test, CreateGBuffer(ctx.ShadowWidth, ctx.ShadowHeight, ETextureFormat::RGBA16F, false), FBTextureLoadAction::Clear, FBTextureStoreAction::Store },
	// };
}

void ShadowPass::Execute(Ref<Scene> scene, RenderContext& ctx)
{
	LightData& data = ctx.LightDataUB.edit();

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
	// ctx.LightDataUB.commit(gEngine->GetDriver());

	// for (const auto& Actor : scene->GetActors())
	// {
	// 	if (!Actor->IsCastShadow()) continue;
	// 	for (auto& Comp : Actor->GetComponents())
	// 	{
	// 		if (auto MeshComp = std::dynamic_pointer_cast<StaticMeshComponent>(Comp))
	// 		{
	// 			if (MeshComp->GetMesh())
	// 			for (auto& section : MeshComp->GetMesh()->GetMeshSections())
	// 			{
	// 				section->Draw();
	// 			}
	// 		}
	// 	}
	// }
}