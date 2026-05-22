#include "StaticMesh.h"

#include "Texture.h"
#include "RHI/VertexBuffer.h"
#include "RHI/IndexBuffer.h"
#include "RHI/RenderPrimitive.h"
#include "RHI/BufferLayout.h"

StaticMesh::StaticMesh(const StaticMeshDesc& desc, std::vector<Ref<MeshSection>> sections,
                       Ref<RHI::RenderPrimitive> primitive, Ref<RHI::VertexBuffer> vb, Ref<RHI::IndexBuffer> ibo)
	: m_Desc(desc), m_LoadedMeshes(std::move(sections)),
	  m_RenderPrimitive(std::move(primitive)), m_VertexBuffer(std::move(vb)), m_IndexBuffer(std::move(ibo))
{
}

Ref<StaticMesh> StaticMesh::Create(const Asset& asset)
{
	using namespace RHI;

	// 1. Build BufferLayout descriptions for each attribute stream
	std::vector<BufferLayout> layouts;

	// Slot 0: Positions (Float4)
	layouts.push_back(BufferLayout{{ ShaderDataType::Float4, "aPosition" }});

	// Slot 1: Normals (Float4)
	layouts.push_back(BufferLayout{{ ShaderDataType::Float4, "aNormal" }});

	// Slot 2: Tangents (Float4)
	layouts.push_back(BufferLayout{{ ShaderDataType::Float4, "aTangent" }});

	// Slot 3: TexCoords0 (UShort2, potentially normalized)
	if (!asset.texCoords0.empty())
	{
		layouts.push_back(BufferLayout{{ ShaderDataType::UShort2, "aTexCoord0", asset.snormUV0 }});
	}

	// Slot 4: TexCoords1 (UShort2, potentially normalized)
	if (!asset.texCoords1.empty())
	{
		layouts.push_back(BufferLayout{{ ShaderDataType::UShort2, "aTexCoord1", asset.snormUV1 }});
	}

	// 2. Create VertexBuffer with all buffer layouts
	VertexBufferDesc vbDesc;
	vbDesc.vertexCount = asset.positions.size();
	vbDesc.bufferLayouts = std::move(layouts);
	auto vb = CreateRef<VertexBuffer>(vbDesc);

	// 3. Upload vertex data per buffer slot
	vb->SetData(0, asset.positions.data(),
	            asset.positions.size() * sizeof(glm::vec4));
	vb->SetData(1, asset.normals.data(),
	            asset.normals.size() * sizeof(glm::vec4));
	vb->SetData(2, asset.tangents.data(),
	            asset.tangents.size() * sizeof(glm::vec4));

	uint8_t nextSlot = 3;
	if (!asset.texCoords0.empty())
	{
		vb->SetData(nextSlot++, asset.texCoords0.data(),
		            asset.texCoords0.size() * sizeof(glm::u16vec2));
	}
	if (!asset.texCoords1.empty())
	{
		vb->SetData(nextSlot++, asset.texCoords1.data(),
		            asset.texCoords1.size() * sizeof(glm::u16vec2));
	}

	// 4. Create IndexBuffer
	IndexBufferDesc ibDesc;
	ibDesc.elementType = ElementType::UINT;
	ibDesc.indexCount = asset.indices.size();
	auto ibo = CreateRef<IndexBuffer>(ibDesc);
	ibo->SetData(asset.indices.data(),
	             asset.indices.size() * sizeof(uint32_t));

	// 5. Create RenderPrimitive
	RenderPrimitiveDesc rpDesc;
	rpDesc.vertexBuffer = vb;
	rpDesc.indexBuffer = ibo;
	rpDesc.primitiveType = PrimitiveType::TRIANGLES;
	auto primitive = CreateRef<RenderPrimitive>(rpDesc);

	// 6. Build MeshSections from Asset parts
	std::vector<Ref<MeshSection>> sections;
	for (const auto& mesh : asset.meshes)
	{
		for (const auto& part : mesh.parts)
		{
			sections.push_back(CreateRef<MeshSection>(primitive,
				static_cast<uint32_t>(part.offset),
				static_cast<uint32_t>(part.count)));
		}
	}

	return CreateRef<StaticMesh>(StaticMeshDesc{asset.file}, std::move(sections),
	                             std::move(primitive), std::move(vb), std::move(ibo));
}
