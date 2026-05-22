#pragma once
#include <vector>

#include "Model/Asset.h"
#include "Model/StaticMesh.h"
#include "common/Core.h"

class Material;

namespace MeshBuilder
{
    Ref<StaticMesh> BuildCube(const Ref<Material>& material);
    Ref<StaticMesh> BuildQuad(const Ref<Material>& material);
}
