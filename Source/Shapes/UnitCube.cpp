#include "UnitCube.h"
#include "MeshBuilder.h"
#include "Model/MeshSection.h"

UnitCube::UnitCube(const Ref<Material>& material)
{
	auto mesh = MeshBuilder::BuildCube(material);
	m_MeshSection = mesh->GetMeshSections()[0];
}

void UnitCube::Draw() const
{
	m_MeshSection->Draw();
}
