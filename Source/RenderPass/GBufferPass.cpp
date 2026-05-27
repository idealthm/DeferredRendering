#include "GBufferPass.h"

#include "Actor.h"
#include "Engine.h"
#include "EngineEnum.h"
#include "RenderTarget.h"
#include "Scene.h"
#include "Component/ActorComponent.h"
#include "Material/Material.h"
#include "Material/MaterialInstance.h"
#include "Model/StaticMesh.h"
#include "RHI/PipelineState.h"
#include "RHI/TargetBufferInfo.h"
#include "Model/Texture.h"

using namespace TextureFactory;

GBufferPass::GBufferPass()
{
}

void GBufferPass::Setup(RenderContext& ctx)
{
	CreateResource(ctx.GBuffer_Position, CreateGBuffer(ctx.viewportSize.x, ctx.viewportSize.y, RHI::Format::RGB16F));
	CreateResource(ctx.GBuffer_Normal,   CreateGBuffer(ctx.viewportSize.x, ctx.viewportSize.y, RHI::Format::RGB16F));
	CreateResource(ctx.GBuffer_Albedo,   CreateGBuffer(ctx.viewportSize.x, ctx.viewportSize.y, RHI::Format::RGB8));
	CreateResource(ctx.GBuffer_Material, CreateGBuffer(ctx.viewportSize.x, ctx.viewportSize.y, RHI::Format::RGB8));
	CreateResource(ctx.GBuffer_Depth,    CreateDepth(ctx.viewportSize.x, ctx.viewportSize.y));

	RenderTarget::Builder builder;
	builder.texture(AttachmentPoint::COLOR0, ctx.GBuffer_Albedo);
	builder.texture(AttachmentPoint::COLOR1, ctx.GBuffer_Normal);
	builder.texture(AttachmentPoint::COLOR2, ctx.GBuffer_Position);
	builder.texture(AttachmentPoint::COLOR3, ctx.GBuffer_Material);
	builder.texture(AttachmentPoint::DEPTH, ctx.GBuffer_Depth);

	m_RenderTarget = builder.Build();
}

void GBufferPass::Execute(Ref<Scene> scene, RenderContext& ctx)
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

	auto& driver = gEngine->GetDriver();

	if (ctx.ModelDataUB.isDirty())
	{
		if (!ctx.ModelDataHandle)
		{
			ctx.ModelDataHandle = driver.CreateBufferObject(ctx.ModelDataUB.getSize(), RHI::BufferObjectBinding::UNIFORM,
				RHI::BufferUsage::STATIC);
		}
		driver.updateBufferObject(ctx.ModelDataHandle, ctx.ModelDataUB.toBufferDescriptor(driver));
	}

	// Begin GBuffer render pass
	RHI::RenderPassParams rpParams{};
	rpParams.viewport = { 0, 0, ctx.viewportSize.x, ctx.viewportSize.y };
	rpParams.flags.clear = RHI::TargetBufferFlags::COLOR0 | RHI::TargetBufferFlags::COLOR1 |
	                       RHI::TargetBufferFlags::COLOR2 | RHI::TargetBufferFlags::COLOR3 |
	                       RHI::TargetBufferFlags::DEPTH;
	rpParams.clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	rpParams.clearDepth = 1.0;

	driver.beginRenderPass(m_RenderTarget->GetHandle(), rpParams);

	RHI::PipelineState state;
	for (auto& renderItem : renderItems)
	{
		auto* mi = renderItem.mi.get();
		if (!mi) continue;

		mi->Commit(driver);
		mi->Use(driver);

		state.program = mi->GetShader();
		state.vertexBufferInfo = renderItem.vertexBufferInfo;
		state.rasterState = mi->GetMaterial()->GetRasterState();
		state.stencilState = mi->GetMaterial()->GetStencilState();
		state.primitiveType = renderItem.primitiveType;

		driver.draw(state, renderItem.renderPrimitive,
			renderItem.indexOffset, renderItem.indexCount, 1);
	}

	driver.endRenderPass();
}
