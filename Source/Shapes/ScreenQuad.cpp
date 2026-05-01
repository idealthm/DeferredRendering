#include "ScreenQuad.h"
#include "MeshBuilder.h"
#include "Model/MeshSection.h"

ScreenQuad::ScreenQuad()
{
	auto mesh = MeshBuilder::BuildQuad(nullptr);
	m_MeshSection = mesh->GetMeshSections()[0];
}

void ScreenQuad::Draw() const
{
	m_MeshSection->Draw();
}
