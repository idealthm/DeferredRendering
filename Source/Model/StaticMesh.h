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

	std::vector<Ref<MeshSection>>& GetMeshSections() {return m_LoadedMeshes;}
	const std::vector<Ref<MeshSection>>& GetMeshSections() const {return m_LoadedMeshes;}
private:
	StaticMeshDesc					m_Desc;
	std::vector<Ref<MeshSection>>	m_LoadedMeshes;
};