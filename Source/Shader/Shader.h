#pragma once
#include <string>
#include <glm/glm.hpp>
#include <unordered_map>

#include "Common/Core.h"

class Shader
{
public:
    Shader(const std::string& filepath);
    ~Shader();

    uint32 GetRendererID() const {return m_RendererID;}

    void Bind() const;
    void Unbind() const;

    // Set uniforms
    void SetUniform3f(const std::string& name, const glm::vec3& value);
    void SetUniform4f(const std::string& name, const glm::vec4& value);
    void SetUniform1i(const std::string& name, int32 value);
    void SetUniformMatrix4f(const std::string& name, const glm::mat4& value);
private:
    struct ShaderProgramSource ParseShader(const std::string& filePath) const;
    static unsigned int CompileShader(unsigned int type, const std::string& source);
    static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader);
    int GetUniformLocation(const std::string& name);
private:
    std::string m_FilePath;
    uint32      m_RendererID;

    std::unordered_map<std::string, int> m_UniformLocations;
};
