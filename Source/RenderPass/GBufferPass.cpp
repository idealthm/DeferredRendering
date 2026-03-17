#include "GBufferPass.h"

#include "Actor.h"
#include "Renderer.h"
#include "Scene.h"
#include "Component/ActorComponent.h"
#include "FrameBuffer/FrameBuffer.h"
#include "Material/Material.h"
#include "Model/StaticMesh.h"
#include "Shader/Shader.h"

GBufferPass::GBufferPass()
{
	// CreateScope<Shader>("Shaders/Basic", 10);
}

void GBufferPass::Setup(FBAttachmentInfo& info)
{
	info.NumSamples = 1;

	info.DSS.depthTest = true;
	info.DSS.depthWrite = true;
	info.DSS.compareFunc = ECompareFunc::Less;

	// 深度：Clear 以开始新的一帧，Store 供后续 Skybox 或 Transparency 使用
	info.Depth = {
		&g_ctx.GBuffer_Depth, 
		CreateDepth(info.Width, info.Height), 
		FBTextureLoadAction::Clear, 
		FBTextureStoreAction::Store 
	};

	info.Attachments = {
		// Slot 0: World Position (可选，如果内存紧张可通过深度重建)
		{ &g_ctx.GBuffer_Position, CreateGBuffer(info.Width, info.Height, ETextureFormat::RGBA16F, false), FBTextureLoadAction::Clear, FBTextureStoreAction::Store },
		// Slot 1: Normal (RG16F 高精度)
		{ &g_ctx.GBuffer_Normal,   CreateGBuffer(info.Width, info.Height, ETextureFormat::RG16F, false), FBTextureLoadAction::Clear, FBTextureStoreAction::Store },
		// Slot 2: Albedo (sRGB 开启)
		{ &g_ctx.GBuffer_Albedo,   CreateGBuffer(info.Width, info.Height, ETextureFormat::SRGB8, true),  FBTextureLoadAction::Clear, FBTextureStoreAction::Store },
		// Slot 3: Material (PBR 参数: Roughness, Metalness, AO)
		{ &g_ctx.GBuffer_Material, CreateGBuffer(info.Width, info.Height, ETextureFormat::RGBA8, false), FBTextureLoadAction::Clear, FBTextureStoreAction::Store },
	};
}

void GBufferPass::Execute(Ref<Scene> scene)
{
	struct RenderItem
	{
		Ref<Shader> shader;
		Ref<MeshSection> Section;
		glm::mat4 ModelTransform;
	};

	std::vector<RenderItem> renderItems;
	for (const auto& Actor : scene->GetActors())
	{
		for (auto& Comp : Actor->GetComponents())
		{
			if (auto MeshComp = std::dynamic_pointer_cast<StaticMeshComponent>(Comp))
			{
				if (MeshComp->GetMesh())
				{
					for (auto& section : MeshComp->GetMesh()->GetMeshSections())
					{
						renderItems.emplace_back(RenderItem{section->GetMaterial()->GetShader(RenderPassType::GBuffer), section, MeshComp->GetModelMatrix()});
					}
				}
			}
		}
	}

	for (auto renderItem : renderItems)
	{
		renderItem.shader->Bind();
		renderItem.shader->SetUniformMatrix4f("uModel", renderItem.ModelTransform);
		renderItem.Section->GetMaterial()->ApplyMaterial(renderItem.shader);
		renderItem.Section->Draw();
	}
}
