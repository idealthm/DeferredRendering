#pragma once
#include "common/Core.h"

class MeshSection;
class Material;

class UnitCube
{
public:
	UnitCube(const Ref<Material>& material = nullptr);
	void Draw() const;

private:
	Ref<MeshSection> m_MeshSection;
};
