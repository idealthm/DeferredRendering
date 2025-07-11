#include "MeshManger.h"

void MeshManger::AddMesh(Ref<StaticMesh> mesh)
{
	m_StaticMeshes.push_back(mesh);
}

void MeshManger::Shutdown()
{
	m_StaticMeshes.clear();
}
