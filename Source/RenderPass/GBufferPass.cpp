#include "GBufferPass.h"

#include "Actor.h"
#include "Renderer.h"
#include "Scene.h"
#include "Component/ActorComponent.h"
#include "FrameBuffer/FrameBuffer.h"
#include "Material/Material.h"
#include "Model/StaticMesh.h"
#include "Shader/Shader.h"

GBufferPass::GBufferPass(uint32 width, uint32 height)
	: RenderPass(width, height)
{
	// CreateScope<Shader>("Shaders/Basic", 10);
}

void GBufferPass::Setup(RenderContext& ctx, FBAttachmentInfo& info)
{
	info.Width = m_Width;
	info.Height = m_Height;
	info.NumSamples = 1;

	info.DSS.depthTest = true;
	info.DSS.depthWrite = true;
	info.DSS.compareFunc = ECompareFunc::Less;

	// 深度：Clear 以开始新的一帧，Store 供后续 Skybox 或 Transparency 使用
	info.Depth = {
		&ctx.GBuffer_Depth, 
		CreateDepth(m_Width, m_Height), 
		FBTextureLoadAction::Clear, 
		FBTextureStoreAction::Store 
	};

	info.Attachments = {
		// Slot 0: World Position (可选，如果内存紧张可通过深度重建)
		{ &ctx.GBuffer_Position, CreateGBuffer(m_Width, m_Height, ETextureFormat::RGBA16F, false), FBTextureLoadAction::Clear, FBTextureStoreAction::Store },
		// Slot 1: Normal (RG16F 高精度)
		{ &ctx.GBuffer_Normal,   CreateGBuffer(m_Width, m_Height, ETextureFormat::RG16F, false), FBTextureLoadAction::Clear, FBTextureStoreAction::Store },
		// Slot 2: Albedo (sRGB 开启)
		{ &ctx.GBuffer_Albedo,   CreateGBuffer(m_Width, m_Height, ETextureFormat::SRGB8, true),  FBTextureLoadAction::Clear, FBTextureStoreAction::Store },
		// Slot 3: Material (PBR 参数: Roughness, Metalness, AO)
		{ &ctx.GBuffer_Material, CreateGBuffer(m_Width, m_Height, ETextureFormat::RGBA8, false), FBTextureLoadAction::Clear, FBTextureStoreAction::Store },
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
				for (auto& section : MeshComp->GetMesh()->GetMeshSections())
				{
					renderItems.emplace_back(RenderItem{section->GetMaterial()->GetShader(RenderPassType::GBuffer), section, MeshComp->GetModelMatrix()});
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
