#include "GBufferPass.h"

#include "Actor.h"
#include "DescriptorSets.h"
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
	: m_DescriptorSetPerRender(gEngine->GetPerRenderableSetLayout())
{
}

void GBufferPass::Setup(RenderContext& ctx)
{
	CreateResource(ctx.GBuffer_Position, CreateGBuffer(ctx.viewportSize.x, ctx.viewportSize.y, RHI::Format::RGBA16F));
	CreateResource(ctx.GBuffer_Normal,   CreateGBuffer(ctx.viewportSize.x, ctx.viewportSize.y, RHI::Format::RGBA16F));
	CreateResource(ctx.GBuffer_Albedo,   CreateGBuffer(ctx.viewportSize.x, ctx.viewportSize.y, RHI::Format::RGBA8));
	CreateResource(ctx.GBuffer_Material, CreateGBuffer(ctx.viewportSize.x, ctx.viewportSize.y, RHI::Format::RGBA8));
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
		uint32_t uboIndex;
		RHI::PrimitiveType primitiveType = RHI::PrimitiveType::TRIANGLES;
	};

	// Collect model matrices
	m_PerRenderableData.clear();
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
						glm::mat4 model = MeshComp->GetModelMatrix();
						PerRenderableData entry = {};
						entry.worldFromModelMatrix = model;
						entry.worldFormModelNormalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
						m_PerRenderableData.push_back(entry);

						renderItems.push_back({
							section->GetMaterial(),
							section->GetRenderPrimitiveHandle(),
							section->GetVertexBufferInfoHandle(),
							section->GetIndexOffset(),
							section->GetIndexCount(),
							uint32_t(m_PerRenderableData.size() - 1),
							section->GetPrimitiveType()
						});
					}
				}
			}
		}
	}

	uint32_t totalInstances = uint32_t(m_PerRenderableData.size());
	if (totalInstances == 0) return;

	auto& driver = gEngine->GetDriver();

	// Lazy-create GPU buffer sized for full UBO (CONFIG_MAX_INSTANCES * sizeof(PerRenderableData))
	uint32_t const bufferSize = CONFIG_MAX_INSTANCES * sizeof(PerRenderableData);
	if (!m_ModelDataHandle)
	{
		m_ModelDataHandle = driver.CreateBufferObject(bufferSize, RHI::BufferObjectBinding::UNIFORM,
			RHI::BufferUsage::DYNAMIC);
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

	for (uint32_t i = 0; i < totalInstances; ++i)
	{
		auto& renderItem = renderItems[i];
		auto* mi = renderItem.mi.get();
		if (!mi) continue;

		mi->Commit(driver);
		mi->Use(driver);

		state.program = mi->GetShader();
		if (!state.program) continue;
		state.vertexBufferInfo = renderItem.vertexBufferInfo;
		state.rasterState = mi->GetMaterial()->GetRasterState();
		state.stencilState = mi->GetMaterial()->GetStencilState();
		state.primitiveType = renderItem.primitiveType;

		// Upload this item's model data into the UBO at position 0
		BufferDescriptor bd(&m_PerRenderableData[i], sizeof(PerRenderableData));
		driver.updateBufferObject(m_ModelDataHandle, std::move(bd));

		m_DescriptorSetPerRender.SetBuffer(+PerRenderableBindingPoints::OBJECT_UNIFORM, m_ModelDataHandle,
			0, bufferSize);
		m_DescriptorSetPerRender.commit(driver, gEngine->GetPerRenderableSetLayout());
		m_DescriptorSetPerRender.bind(driver, DescriptorSetBindingPoints::PER_RENDERABLE);

		driver.draw(state, renderItem.renderPrimitive,
			renderItem.indexOffset, renderItem.indexCount, 1);
	}

	driver.endRenderPass();
}
