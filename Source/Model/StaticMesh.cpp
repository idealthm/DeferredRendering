#include "StaticMesh.h"

#include <utility>

#include "Engine.h"
#include "Texture.h"
#include "RHI/BufferDescriptor.h"
#include "RHI/VertexBuffer.h"
#include "RHI/IndexBuffer.h"
#include "RHI/RenderPrimitive.h"
#include "RHI/BufferLayout.h"

StaticMesh::StaticMesh(const StaticMeshDesc& desc, std::vector<Ref<MeshSection>> sections, const Ref<MaterialInstance>& mi)
	: m_Desc(desc), m_LoadedMeshes(std::move(sections)), m_Material(mi)
{
	for (auto& section : m_LoadedMeshes)
	{
		section->SetOwnerMesh(this);
	}
}

Ref<StaticMesh> StaticMesh::Create(const Asset& asset, const Ref<MaterialInstance>& mi)
{
	using namespace RHI;

	auto& driver = gEngine->GetDriver();
	uint32_t const vertexCount = static_cast<uint32_t>(asset.positions.size());
	uint8_t bufferIdx = 0;

	// 1. Build VertexBuffer
	VertexBuffer::Builder vbBuilder;
	vbBuilder.vertexCount(vertexCount);

	// Buffer 0: Positions
	vbBuilder.attribute(VertexAttribute::POSITION, bufferIdx, ElementType::FLOAT4);
	vbBuilder.bufferCount(++bufferIdx);

	// Buffer 1: Tangents (quaternion-encoded TBN frame)
	vbBuilder.attribute(VertexAttribute::TANGENTS, bufferIdx, ElementType::FLOAT4);
	vbBuilder.bufferCount(++bufferIdx);

	// Buffer 2: TexCoords0
	if (!asset.texCoords0.empty())
	{
		vbBuilder.attribute(VertexAttribute::UV0, bufferIdx, ElementType::USHORT2);
		vbBuilder.normalized(VertexAttribute::UV0, asset.snormUV0);
		vbBuilder.bufferCount(++bufferIdx);
	}

	// Buffer 3: TexCoords1
	if (!asset.texCoords1.empty())
	{
		vbBuilder.attribute(VertexAttribute::UV1, bufferIdx, ElementType::USHORT2);
		vbBuilder.normalized(VertexAttribute::UV1, asset.snormUV1);
		vbBuilder.bufferCount(++bufferIdx);
	}

	auto vb = vbBuilder.build(driver);

	// 2. Upload vertex data
	bufferIdx = 0;
	vb->setBufferAt(driver, bufferIdx++,
		BufferDescriptor(asset.positions.data(), vertexCount * sizeof(glm::vec4)));
	vb->setBufferAt(driver, bufferIdx++,
		BufferDescriptor(asset.tangents.data(), vertexCount * sizeof(glm::vec4)));
	if (!asset.texCoords0.empty())
	{
		vb->setBufferAt(driver, bufferIdx++,
			BufferDescriptor(asset.texCoords0.data(), vertexCount * sizeof(glm::u16vec2)));
	}
	if (!asset.texCoords1.empty())
	{
		vb->setBufferAt(driver, bufferIdx++,
			BufferDescriptor(asset.texCoords1.data(), vertexCount * sizeof(glm::u16vec2)));
	}

	// 3. Create IndexBuffer
	IndexBufferDesc ibDesc;
	ibDesc.elementType = ElementType::UINT;
	ibDesc.indexCount = asset.indices.size();
	auto ibo = CreateRef<IndexBuffer>(ibDesc);
	ibo->SetData(asset.indices.data(), asset.indices.size() * sizeof(uint32_t));

	// 4. Create RenderPrimitive
	RenderPrimitiveDesc rpDesc;
	rpDesc.vertexBuffer = vb;
	rpDesc.indexBuffer = ibo;
	rpDesc.primitiveType = PrimitiveType::TRIANGLES;
	auto primitive = CreateRef<RenderPrimitive>(rpDesc);

	// 5. Build MeshSections from Asset parts
	std::vector<Ref<MeshSection>> sections;
	for (const auto& mesh : asset.meshes)
	{
		for (const auto& part : mesh.parts)
		{
			sections.push_back(CreateRef<MeshSection>(primitive,
				static_cast<uint32_t>(part.offset),
				static_cast<uint32_t>(part.count), part.mi));
		}
	}

	StaticMeshDesc desc;
	desc.ibo = ibo;
	desc.primitive = primitive;
	desc.vb = vb;

	return CreateRef<StaticMesh>(desc, std::move(sections), mi);
}

void StaticMesh::SetMaterialInstance(Ref<MaterialInstance> instance)
{
	m_Material = std::move(instance);
}

Ref<MaterialInstance> StaticMesh::GetMaterial()
{
	return m_Material;
}
