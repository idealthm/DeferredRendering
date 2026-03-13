#include "StaticMesh.h"

#include "Texture.h"
#include "Util.h"


StaticMesh::StaticMesh(const StaticMeshDesc& desc, const std::vector<Ref<MeshSection>>& meshes)
	: m_Desc(desc), m_LoadedMeshes(meshes)
{
}
