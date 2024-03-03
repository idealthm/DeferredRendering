#pragma once
#include <string>
#include <glm.hpp>
#include <unordered_map>

class Shader
{
public:
    Shader(const std::string& filepath);
    ~Shader();

    void Bind() const;
    void Unbind() const;

    // Set uniforms
    void SetUniform4f(const std::string& name, const glm::vec4& value);
private:
    struct ShaderProgramSource ParseShader(const std::string& filePath) const;
    static unsigned int CompileShader(unsigned int type, const std::string& source);
    static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader);
    int GetUniformLocation(const std::string& name);
private:
    std::string m_FilePath;
    unsigned int m_RendererID;

    std::unordered_map<std::string, int> m_UniformLocations;
};
