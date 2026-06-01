#include "MeshBuilder.h"

#include "Engine.h"
#include "Material/Material.h"
#include "Material/MaterialInstance.h"
#include "Model/MeshSection.h"

static std::vector<uint32_t> cubeIndices = {
    0, 1, 2,
    2, 3, 0,
    // Front face
    4, 5, 6,
    6, 7, 4,
    // Left face
    8, 9, 10,
    10, 11, 8,
    // Right face
    12, 13, 14,
    14, 15, 12,
    // Bottom face
    16, 17, 18,
    18, 19, 16,
    // Top face (fixed winding)
    20, 22, 21,
    22, 20, 23
};

// Packed cube data: pos(3) normal(3) uv(2) tangent(3) = 11 floats per vertex
static float cubeRaw[] = {
    // Back face (0-3)
    -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f, -1.0f,  0.0f,  0.0f,
     1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f, -1.0f,  0.0f,  0.0f,
     1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f, -1.0f,  0.0f,  0.0f,
    -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f, -1.0f,  0.0f,  0.0f,

    // Front face (4-7)
    -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,  1.0f,  0.0f,  0.0f,
     1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,  1.0f,  0.0f,  0.0f,
     1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,  1.0f,  0.0f,  0.0f,
    -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,  1.0f,  0.0f,  0.0f,

    // Left face (8-11)
    -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,  0.0f,  0.0f, -1.0f,
    -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,  0.0f,  0.0f, -1.0f,
    -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,  0.0f,  0.0f, -1.0f,
    -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,  0.0f,  0.0f, -1.0f,

    // Right face (12-15)
     1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,  0.0f,  0.0f,  1.0f,
     1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,  0.0f,  0.0f,  1.0f,
     1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,  0.0f,  0.0f,  1.0f,
     1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,  0.0f,  0.0f,  1.0f,

    // Bottom face (16-19)
    -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,  1.0f,  0.0f,  0.0f,
     1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,  1.0f,  0.0f,  0.0f,
     1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,  1.0f,  0.0f,  0.0f,
    -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,  1.0f,  0.0f,  0.0f,

    // Top face (fixed winding) (20-23)
    -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,  1.0f,  0.0f,  0.0f,
     1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,  1.0f,  0.0f,  0.0f,
     1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,  1.0f,  0.0f,  0.0f,
    -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,  1.0f,  0.0f,  0.0f
};

static Asset BuildAssetFromInterleaved(const float* raw, size_t vertexCount,
                                        const std::vector<uint32_t>& indices, int strideFloats)
{
    Asset asset;
    asset.indices = indices;
    asset.positions.reserve(vertexCount);
    asset.normals.reserve(vertexCount);
    asset.tangents.reserve(vertexCount);
    asset.texCoords0.reserve(vertexCount);
    asset.snormUV0 = true;

    for (size_t i = 0; i < vertexCount; i++)
    {
        const float* v = raw + i * strideFloats;
        asset.positions.push_back({v[0], v[1], v[2], 1.0f});
        asset.normals.push_back({v[3], v[4], v[5], 0.0f});
        float u = glm::clamp(v[6], 0.0f, 1.0f);
        float vv = glm::clamp(v[7], 0.0f, 1.0f);
        asset.texCoords0.push_back({
            static_cast<uint16_t>(u * 65535.0f),
            static_cast<uint16_t>(vv * 65535.0f)
        });
        asset.tangents.push_back({v[8], v[9], v[10], 1.0f});
    }

    Mesh mesh;
    mesh.offset = 0;
    mesh.count = vertexCount;
    Part part;
    part.offset = 0;
    part.count = indices.size();
    mesh.parts.push_back(part);
    asset.meshes.push_back(mesh);

    return asset;
}

Ref<StaticMesh> MeshBuilder::BuildCube()
{
    constexpr int stride = 11; // pos(3)+normal(3)+uv(2)+tangent(3)
    constexpr size_t vertexCount = 24;
    Asset asset = BuildAssetFromInterleaved(cubeRaw, vertexCount, cubeIndices, stride);

    return StaticMesh::Create(asset, CreateRef<MaterialInstance>(gEngine->GetDefaultShapeMaterial()));
}

static std::vector<uint32_t> QuadIndices = { 0, 1, 2, 1, 3, 2 };

static float QuadRaw[] = {
    -1.0f,  1.0f,
    -1.0f, -1.0f,
     1.0f,  1.0f,
     1.0f, -1.0f,
};

Ref<StaticMesh> MeshBuilder::BuildQuad()
{
    Asset asset;
    asset.indices = QuadIndices;

    constexpr size_t vertexCount = 4;
    for (size_t i = 0; i < vertexCount; i++)
    {
        asset.positions.push_back({QuadRaw[i * 2], QuadRaw[i * 2 + 1], 0.0f, 1.0f});
        asset.normals.push_back({0.0f, 0.0f, 1.0f, 0.0f});
        asset.tangents.push_back({1.0f, 0.0f, 0.0f, 1.0f});
        asset.texCoords0.push_back({
            static_cast<uint16_t>(QuadRaw[i * 2] * 0.5f * 65535.0f + 32767.0f),
            static_cast<uint16_t>(QuadRaw[i * 2 + 1] * 0.5f * 65535.0f + 32767.0f)
        });
    }
    asset.snormUV0 = true;

    Mesh mesh;
    mesh.offset = 0;
    mesh.count = vertexCount;
    Part part;
    part.offset = 0;
    part.count = static_cast<uint32_t>(QuadIndices.size());
    mesh.parts.push_back(part);
    asset.meshes.push_back(mesh);

    return StaticMesh::Create(asset, CreateRef<MaterialInstance>(gEngine->GetDefaultShapeMaterial()));
}
