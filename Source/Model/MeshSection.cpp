#include "MeshSection.h"

#include "Texture.h"
#include "IndexBuffer/IndexBuffer.h"
#include "VertexArray/VertexArray.h"
#include "VertexBuffer/VertexBuffer.h"
#include "glad/glad.h"

MeshSection::MeshSection(const Ref<VertexArray>& vertexArr, const Ref<IndexBuffer>& indexBuf, const map<TextureType, vector<Ref<Texture2D>>>& textures)
	: m_VertexArray(vertexArr), m_IndexBuffer(indexBuf), m_Textures(textures)
{
}

void MeshSection::SetTexture(const MeshTextureMap& textures)
{
	m_Textures = textures;
}

void MeshSection::AddTexture(TextureType type, const Ref<Texture2D>& texture)
{
	m_Textures[type].push_back(texture);
}

void MeshSection::DeleteTexture(TextureType type, const Ref<Texture2D>& texture)
{
	auto& texArr = m_Textures[type];

	uint32 i = 0;
	for (; i < texArr.size(); i++)
		if (texArr[i] == texture) break;
	texArr.erase(texArr.begin() + i);
}

void MeshSection::Draw(Shader& shader)
{
	auto BuildUniformName = [](TextureType type, int32 Index)
	{
		std::string tail = Index ? std::to_string(Index + 1) : ""; 
		switch (type)
		{
			case TextureType::DIFFUSE: return std::string("uAlbedoMap") + tail;
			case TextureType::SPECULAR: return std::string("specular") + tail;
			case TextureType::NORMAL: return std::string("uNormalMap") + tail;
			case TextureType::HEIGHT: return std::string("height") + tail;
		}
	};

	int32 Count = 0;

	for (auto& [type, texArr] : m_Textures)
	{
		for (int i = 0; i < texArr.size(); i++)
		{
			texArr[i]->Bind(Count);
			shader.SetUniform1i(BuildUniformName(type, i), Count++);
		}
	}

	m_VertexArray->Bind();
	m_IndexBuffer->Bind();

	glDrawElements(GL_TRIANGLES, m_IndexBuffer->GetCount(), GL_UNSIGNED_INT, 0);
}
