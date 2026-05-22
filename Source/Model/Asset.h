#pragma once

#include <Common/Core.h>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Material/MaterialEnums.h"

struct Part
{
    size_t offset; // index offset (in indices, not bytes)
    size_t count;  // index count
    Ref<class MaterialInstance> mi;
};

struct Mesh
{
    size_t           offset; // base vertex offset into Asset arrays
    size_t           count;  // vertex count
    std::vector<Part> parts;
};

struct Asset
{
    std::string               file;
    std::vector<uint32_t>     indices;
    std::vector<glm::vec4>    positions;
    std::vector<glm::vec4>    normals;
    std::vector<glm::vec4>    tangents;     // xyz = tangent, w = bitangent sign
    std::vector<glm::u16vec2> texCoords0;
    std::vector<glm::u16vec2> texCoords1;
    bool                      snormUV0 = false;
    bool                      snormUV1 = false;
    std::vector<Mesh>         meshes;
};

inline void ComputeRequiredVertexAttributes(Asset& asset, const std::vector<VertexAttribute>& required)
{
    bool needsPosition = false, needsTangents = false, needsUV0 = false, needsUV1 = false;
    for (auto a : required)
    {
        switch (a)
        {
        case POSITION:  needsPosition = true; break;
        case TANGENTS:  needsTangents = true; break;
        case UV0:       needsUV0 = true;      break;
        case UV1:       needsUV1 = true;      break;
        default: break;
        }
    }

    if (needsPosition && asset.positions.empty())
        throw std::runtime_error(asset.file + ": missing required attribute POSITION");

    if (needsTangents && asset.tangents.empty() && !asset.normals.empty())
    {
        asset.tangents.resize(asset.normals.size());
        for (size_t i = 0; i < asset.normals.size(); ++i)
        {
            glm::vec3 n = glm::vec3(asset.normals[i]);
            glm::vec3 t = (std::abs(n.x) < 0.9f) ? glm::vec3(1, 0, 0) : glm::vec3(0, 1, 0);
            t = glm::normalize(t - n * glm::dot(n, t));
            glm::vec3 b = glm::cross(n, t);

            // Quaternion encode: q = quat_from_basis(t, b, n)
            glm::mat3 m(t, b, n);
            glm::quat q = glm::quat_cast(m);
            asset.tangents[i] = glm::vec4(q.x, q.y, q.z, q.w);
        }
    }
    else if (needsTangents && asset.tangents.empty())
        throw std::runtime_error(asset.file + ": missing required attribute TANGENTS (no normal data to derive from)");

    if (needsUV0 && asset.texCoords0.empty())
        throw std::runtime_error(asset.file + ": missing required attribute UV0");

    if (needsUV1 && asset.texCoords1.empty())
        throw std::runtime_error(asset.file + ": missing required attribute UV1");
}
