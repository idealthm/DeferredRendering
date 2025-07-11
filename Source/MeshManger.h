#pragma once
#include <string>
#include <vector>

#include "common/Core.h"
#include "Component/ActorComponent.h"

class MeshManger
{
public:
	static MeshManger Get();

	void AddMesh(Ref<StaticMesh> mesh);
	void Shutdown();

private:
	std::vector<Ref<StaticMesh>> m_StaticMeshes;
};
