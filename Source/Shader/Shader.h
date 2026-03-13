#pragma once
#include <bitset>
#include <string>
#include <glm/glm.hpp>
#include <unordered_map>

#include "Common/Core.h"
#include "ShaderPreprocessor/ShaderDefines.h"


class Texture;
class Shader;


enum class EShaderType
{
    Shader_PBR,				// Accept albedo, normal, roughness, metallic, output albedo, position, normal, material.rmao 
    Shader_ShadowMap,		// output depth.
    Shader_SkyBox,			// Accept Depth, output SceneColor,
    Shader_Cubemap,			// Accept HDR image, output CubeMap,
};

class Shader
{
    friend struct SlotSnapshot;
public:
    Shader(const std::string& FilePath, uint32 freeSlot, DUI* dui=nullptr);
    ~Shader();

    uint32 GetRendererID() const {return m_RendererID;}

    void Bind() const;
    void Unbind() const;

    // Set uniforms
    void SetUniform1f(const std::string& name, const float& value);
    void SetUniform2f(const std::string& name, const glm::vec2& value);
    void SetUniform3f(const std::string& name, const glm::vec3& value);
    void SetUniform4f(const std::string& name, const glm::vec4& value);
    void SetUniform1i(const std::string& name, int32 value);
    void SetUniformMatrix4f(const std::string& name, const glm::mat4& value);

    uint32 GetFreeSlotIndex() const {return m_FreeSlotIndex;}
private:
    int GetUniformLocation(const std::string& name);

private:
    uint64      m_BuildHash;
    uint32      m_RendererID;
    uint32      m_FreeSlotIndex;

    std::unordered_map<std::string, int> m_UniformLocations;
};
