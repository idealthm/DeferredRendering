#include "GBufferPass.h"

#include "Actor.h"
#include "Engine.h"
#include "EngineEnum.h"
#include "Renderer.h"
#include "Scene.h"
#include "Component/ActorComponent.h"
#include "FrameBuffer/FrameBuffer.h"
#include "Material/Material.h"
#include "Material/MaterialInstance.h"
#include "Model/StaticMesh.h"
#include "RHI/PipelineState.h"
#include "Shader/Program.h"

using namespace TextureFactory;

GBufferPass::GBufferPass()
{
}

void GBufferPass::Setup(FBAttachmentInfo& info, uint32_t step, RenderContext& ctx)
{
	info.NumSamples = 1;

	info.DSS.depthTest = true;
	info.DSS.depthWrite = true;
	info.DSS.compareFunc = ECompareFunc::Less;

	CreateResource(ctx.GBuffer_Depth, CreateDepth(info.Width, info.Height));

	info.Depth = {
		// ctx.GBuffer_Depth->GetRendererID(),
		// FBTextureLoadAction::Clear,
		// FBTextureStoreAction::Store
	};

	CreateResource(ctx.GBuffer_Position, CreateGBuffer(info.Width, info.Height, RHI::Format::RGBA16F));
	CreateResource(ctx.GBuffer_Normal, CreateGBuffer(info.Width, info.Height, RHI::Format::RG16F));
	CreateResource(ctx.GBuffer_Albedo, CreateGBuffer(info.Width, info.Height, RHI::Format::SRGB8));
	CreateResource(ctx.GBuffer_Material, CreateGBuffer(info.Width, info.Height, RHI::Format::RGBA8));

	info.Attachments = {
		// { ctx.GBuffer_Position->GetRendererID(), FBTextureLoadAction::Clear, FBTextureStoreAction::Store },
		// { ctx.GBuffer_Normal->GetRendererID(), FBTextureLoadAction::Clear, FBTextureStoreAction::Store },
		// { ctx.GBuffer_Albedo->GetRendererID(), FBTextureLoadAction::Clear, FBTextureStoreAction::Store },
		// { ctx.GBuffer_Material->GetRendererID(), FBTextureLoadAction::Clear, FBTextureStoreAction::Store },
	};
}

void GBufferPass::Execute(Ref<Scene> scene, uint32_t step, RenderContext& ctx)
{
	struct PrimitiveInfo
	{
		Ref<MaterialInstance> mi;
		Handle<RHI::HwRenderPrimitive> renderPrimitive;
		Handle<RHI::HwVertexBufferInfo> vertexBufferInfo;
		uint32_t indexOffset;
		uint32_t indexCount;
		uint32_t index; // PerModelUib Index
		RHI::PrimitiveType primitiveType = RHI::PrimitiveType::TRIANGLES;
	};

	uint32_t instanceCount = 0;
	auto& ModelTransform = ctx.ModelDataUB.edit().models;

	std::vector<PrimitiveInfo> renderItems;
	for (const auto& Actor : scene->GetActors())
	{
		for (auto& Comp : Actor->GetComponents())
		{
			if (auto MeshComp = std::dynamic_pointer_cast<StaticMeshComponent>(Comp))
			{
				if (MeshComp->GetMesh())
				{
					auto& mesh = MeshComp->GetMesh();
					renderItems.reserve(renderItems.size() + mesh->GetMeshSections().size());
					for (auto& section : mesh->GetMeshSections())
					{
						ModelTransform[instanceCount].ModelTransform = MeshComp->GetModelMatrix();

						renderItems.push_back({
							section->GetMaterial(),
							section->GetRenderPrimitiveHandle(),
							section->GetVertexBufferInfoHandle(),
							section->GetIndexOffset(),
							section->GetIndexCount(),
							instanceCount++,
							section->GetPrimitiveType()
						});
					}
				}
			}
		}
	}

	ASSERT(instanceCount <= CONFIG_MAX_INSTANCES);

	ctx.ModelDataUB.commit(gEngine->GetDriver());

	auto& driver = gEngine->GetDriver();

	RHI::PipelineState state;
	for (auto& renderItem : renderItems)
	{
		auto* mi = renderItem.mi.get();
		if (!mi) continue;

		mi->CommitUniforms(driver);

		state.program = mi->GetShader();
		state.vertexBufferInfo = renderItem.vertexBufferInfo;
		state.rasterState = mi->GetMaterial()->GetRasterState();
		state.stencilState = mi->GetMaterial()->GetStencilState();
		state.primitiveType = renderItem.primitiveType;

		driver.draw(state, renderItem.renderPrimitive,
			renderItem.indexOffset, renderItem.indexCount, 1);
	}
}
