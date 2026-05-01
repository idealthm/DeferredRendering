#include "MeshBuilder.h"

#include "Model/MeshSection.h"

static std::vector<uint32_t> cubeIndices = {
	0, 1, 2,  // 第一个三角形
   2, 3, 0,  // 第二个三角形
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
   // Top face
   20, 21, 22,
   22, 23, 20
};

static std::vector<float> cubeVertices = {
    // Position(3)         Normal(3)            UV(2)       Tangent(3)
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

    // Top face (20-23)
    -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,  1.0f,  0.0f,  0.0f,
     1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,  1.0f,  0.0f,  0.0f,
     1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,  1.0f,  0.0f,  0.0f,
    -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,  1.0f,  0.0f,  0.0f
};

static std::vector<uint32_t> QuadIndices = {
	0, 1, 2, 1, 2, 3
};

static std::vector<float> QuadVertices = {
 	// positions        // texture Coords
 	-1.0f,  1.0f,
 	-1.0f, -1.0f,
 	 1.0f,  1.0f,
 	 1.0f, -1.0f,
};

Ref<StaticMesh> MeshBuilder::BuildCube(const Ref<Material>& material)
{
	BufferLayout layout = {
		{ShaderDataType::Float3, "aPosition"},
		{ShaderDataType::Float3, "aNormal"},
		{ShaderDataType::Float2, "aTexCoords"},
		{ShaderDataType::Float3, "aTangent"},
	};

	auto section = BuildSection(cubeVertices, cubeIndices, layout);
	section->SetMaterial(material);
	return CreateRef<StaticMesh>(StaticMeshDesc{}, std::vector<Ref<MeshSection>>{section});
}

Ref<StaticMesh> MeshBuilder::BuildQuad(const Ref<Material>& material)
{
	BufferLayout layout = {
		{ShaderDataType::Float2, "aPosition"},
	};

	auto section = BuildSection(QuadVertices, QuadIndices, layout);
	section->SetMaterial(material);
	return CreateRef<StaticMesh>(StaticMeshDesc{}, std::vector<Ref<MeshSection>>{section});
}