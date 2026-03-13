#include "MeshSection.h"

#include <array>

#include "Texture.h"
#include "IndexBuffer/IndexBuffer.h"
#include "VertexArray/VertexArray.h"
#include "VertexBuffer/VertexBuffer.h"
#include "glad/glad.h"
#include "Material/Material.h"

std::array<std::string, 6> TypeToUniform = {"uAlbedoMap", "uNormalMap", "uRoughnessMap", "uMetallicMap","uAOMap", "uHeightMap"};

MeshSection::MeshSection(const Ref<VertexArray>& vertexArr, const Ref<IndexBuffer>& indexBuf, Ref<Material> material)
	: m_VertexArray(vertexArr), m_IndexBuffer(indexBuf), m_Material(material)
{
}

void MeshSection::SetMaterial(const Ref<Material>& material)
{
	m_Material = material;
}

Ref<Material> MeshSection::GetMaterial()
{
	return m_Material;
}

void MeshSection::Draw() const
{
	m_VertexArray->Bind();
	m_IndexBuffer->Bind();

	glDrawElements(GL_TRIANGLES, m_IndexBuffer->GetCount(), GL_UNSIGNED_INT, 0);
}
