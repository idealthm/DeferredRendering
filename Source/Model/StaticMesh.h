#pragma once
#include <string>
#include <vector>

#include "MeshSection.h"
#include "Shader/Shader.h"

namespace Util
{
	class MeshLoader;
}

class Texture2D;

struct StaticMeshDesc
{
	string importPath;
};

class StaticMesh
{
public:
	StaticMesh(const StaticMeshDesc& desc, const std::vector<Ref<MeshSection>>& meshes);
	StaticMesh(Util::MeshLoader& loader);

	void Draw(Shader& shader) const;
private:
	StaticMeshDesc					m_Desc;
	std::vector<Ref<MeshSection>>	m_LoadedMeshes;
	std::vector<Ref<Texture2D>>		m_LoadedTextures;
};