#pragma once
#include <vector>

#include "Model/Asset.h"
#include "Model/StaticMesh.h"
#include "common/Core.h"

class Material;

namespace MeshBuilder
{
    Ref<StaticMesh> BuildCube();
    Ref<StaticMesh> BuildQuad();
}
