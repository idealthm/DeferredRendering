#pragma once

#include <map>
#include <vector>
#include <Shader/Shader.h>
#include "Renderer.h"

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
    DIFFUSE,
    SPECULAR,
    NORMAL,
    HEIGHT,
};

using MeshTextureMap = map<TextureType, vector<Ref<Texture2D>>>; 

class MeshSection {
public:
    // constructor
    MeshSection(const Ref<VertexArray>& vertexArr, const Ref<IndexBuffer>& indexBuf, const MeshTextureMap& textures = {});

    void SetTexture(const MeshTextureMap& textures);
    void AddTexture(TextureType type, const Ref<Texture2D>& texture);
    void DeleteTexture(TextureType type, const Ref<Texture2D>& texture);
    const MeshTextureMap& GetTextures() const { return m_Textures; }

    // render the mesh
    void Draw(Shader &shader);

private:
    // mesh Data
    Ref<VertexArray>    m_VertexArray;
    Ref<IndexBuffer>    m_IndexBuffer;
    MeshTextureMap      m_Textures;
};
