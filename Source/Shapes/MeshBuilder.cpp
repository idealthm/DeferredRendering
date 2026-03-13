#include "MeshBuilder.h"

#include "Model/MeshSection.h"

static std::vector<uint32> cubeIndices = {
	0, 2, 1,    0, 3, 2,    // Back
	4, 5, 6,    4, 6, 7,    // Front
	8, 9, 10,   8, 10, 11,  // Left
	12, 14, 13, 12, 15, 14, // Right
	16, 17, 18, 16, 18, 19, // Bottom
	20, 22, 21, 20, 23, 22  // Top
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

static std::vector<uint32> QuadIndices = {
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