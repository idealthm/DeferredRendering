#pragma once
#include <vector>

#include "VertexBuffer/VertexBufferLayout.h"
#include "common/Core.h"
#include "Model/StaticMesh.h"
#include "VertexArray/VertexArray.h"
#include "IndexBuffer/IndexBuffer.h"
#include "VertexBuffer/VertexBuffer.h"

namespace MeshBuilder
{
	enum ShapeType
	{
		Cube,
		Quad,
		Model,
	};

	Ref<StaticMesh> BuildCube(const Ref<Texture2D>& texture = {});
	Ref<StaticMesh> BuildQuad(const Ref<Texture2D>& texture = {});

	template<typename T>
	Ref<MeshSection> BuildSection(const std::vector<T>& vertices, std::vector<uint32>& indices, BufferLayout& layout)
	{
		Ref<VertexArray> vertexArray = CreateRef<VertexArray>();
		Ref<IndexBuffer> indexBuffer = CreateRef<IndexBuffer>(indices.data(), indices.size());
		Ref<VertexBuffer> vertexBuffer = CreateRef<VertexBuffer>(vertices.data(), vertices.size() * sizeof(T));
		vertexBuffer->SetLayout(layout);
		vertexArray->AddBuffer(vertexBuffer);
		vertexArray->SetIndexBuffer(indexBuffer);
		return CreateRef<MeshSection>(vertexArray, indexBuffer);
	}
}

