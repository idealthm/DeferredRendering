#include "StaticMesh.h"

#include "Texture.h"
#include "Util.h"


StaticMesh::StaticMesh(const StaticMeshDesc& desc, const std::vector<Ref<MeshSection>>& meshes)
	: m_Desc(desc), m_LoadedMeshes(meshes)
{
	for (auto& mesh : m_LoadedMeshes)
	{
		for (auto& [type, texArr] : mesh->GetTextures())
		{
			for (auto& tex : texArr)
				m_LoadedTextures.push_back(tex);
		}
	}
}

StaticMesh::StaticMesh(Util::MeshLoader& loader)
	: StaticMesh(StaticMeshDesc{loader.m_Path}, loader.m_LoadedMeshes)
{
}

void StaticMesh::Draw(Shader& shader) const
{
	for (uint32 i = 0; i < m_LoadedMeshes.size(); i++)
		m_LoadedMeshes[i]->Draw(shader);
}

