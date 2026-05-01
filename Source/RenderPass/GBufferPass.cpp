#include "GBufferPass.h"

#include "Actor.h"
#include "Renderer.h"
#include "Scene.h"
#include "Component/ActorComponent.h"
#include "FrameBuffer/FrameBuffer.h"
#include "Material/Material.h"
#include "Model/StaticMesh.h"
#include "Model/Texture.h"
#include "RHI/SamplerPool.h"
#include "Shader/Shader.h"

GBufferPass::GBufferPass()
{
	// CreateScope<Shader>("Shaders/Basic", 10);
}

void GBufferPass::Setup(FBAttachmentInfo& info, uint32_t step, RenderContext& ctx)
{
	info.NumSamples = 1;

	info.DSS.depthTest = true;
	info.DSS.depthWrite = true;
	info.DSS.compareFunc = ECompareFunc::Less;

	// 深度：Clear 以开始新的一帧，Store 供后续 Skybox 或 Transparency 使用
	CreateResource(ctx.GBuffer_Depth, CreateDepth(info.Width, info.Height));

	info.Depth = {
		ctx.GBuffer_Depth->GetRendererID(), 
		FBTextureLoadAction::Clear, 
		FBTextureStoreAction::Store 
	};

	CreateResource(ctx.GBuffer_Position, CreateGBuffer(info.Width, info.Height, RHI::Format::RGBA16F));
	CreateResource(ctx.GBuffer_Normal, CreateGBuffer(info.Width, info.Height, RHI::Format::RG16F));
	CreateResource(ctx.GBuffer_Albedo, CreateGBuffer(info.Width, info.Height, RHI::Format::SRGB8));
	CreateResource(ctx.GBuffer_Material, CreateGBuffer(info.Width, info.Height, RHI::Format::RGBA8));

	if (!ctx.GBuffer_Position->GetSampler())
		ctx.GBuffer_Position->SetSampler(SamplerPool::Get().GetOrCreate(DefaultClampSampler()));
	if (!ctx.GBuffer_Normal->GetSampler())
		ctx.GBuffer_Normal->SetSampler(SamplerPool::Get().GetOrCreate(DefaultClampSampler()));
	if (!ctx.GBuffer_Albedo->GetSampler())
		ctx.GBuffer_Albedo->SetSampler(SamplerPool::Get().GetOrCreate(DefaultClampSampler()));
	if (!ctx.GBuffer_Material->GetSampler())
		ctx.GBuffer_Material->SetSampler(SamplerPool::Get().GetOrCreate(DefaultClampSampler()));

	info.Attachments = {
		// Slot 0: World Position (可选，如果内存紧张可通过深度重建)
		{ ctx.GBuffer_Position->GetRendererID(), FBTextureLoadAction::Clear, FBTextureStoreAction::Store },
		// Slot 1: Normal (RG16F 高精度)
		{ ctx.GBuffer_Normal->GetRendererID(), FBTextureLoadAction::Clear, FBTextureStoreAction::Store },
		// Slot 2: Albedo (sRGB 开启)
		{ ctx.GBuffer_Albedo->GetRendererID(), FBTextureLoadAction::Clear, FBTextureStoreAction::Store },
		// Slot 3: Material (PBR 参数: Roughness, Metalness, AO)
		{ ctx.GBuffer_Material->GetRendererID(), FBTextureLoadAction::Clear, FBTextureStoreAction::Store },
	};
}

void GBufferPass::Execute(Ref<Scene> scene, uint32_t step, RenderContext& ctx)
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
