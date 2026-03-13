#pragma once

#include <map>
#include <vector>
#include <Shader/Shader.h>
#include "Renderer.h"

class Material;
class VertexArray;
class VertexBuffer;
class IndexBuffer;
class Texture2D;
class Shader;
using namespace std;

#define MAX_BONE_INFLUENCE 4

struct Vertex {
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoords;
    // tangent
    glm::vec3 Tangent;
    // bitangent
    glm::vec3 Bitangent;
    //bone indexes which will influence this vertex
    int m_BoneIDs[MAX_BONE_INFLUENCE];
    //weights from each bone
    float m_Weights[MAX_BONE_INFLUENCE];
};

enum class TextureType : uint8
{
    ALBEDO,
    NORMAL,
    ROUGHNESS,
    METALLIC,
    AO,
    HEIGHT,
};

extern std::array<std::string, 6> TypeToUniform;

using MeshTextureMap = std::map<TextureType, Ref<Texture2D>>;

class MeshSection {
public:
    // constructor
    MeshSection(const Ref<VertexArray>& vertexArr, const Ref<IndexBuffer>& indexBuf, Ref<Material> material = nullptr);

    void SetMaterial(const Ref<Material>& material);
    Ref<Material> GetMaterial();

    // render the mesh
    void Draw() const;

private:
    // mesh Data
    Ref<VertexArray>    m_VertexArray;
    Ref<IndexBuffer>    m_IndexBuffer;
    Ref<Material>       m_Material;
};
