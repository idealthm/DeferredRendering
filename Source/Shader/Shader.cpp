#include "Shader.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>

#include "Renderer.h"
#include "ShaderPreprocessor/ShaderLoader.h"

#define SET_UNIFORM(X) if (int location = GetUniformLocation(name); ~location) {GLCall(X);}

struct ShaderProgramSource
{
    std::string VertexSource;
    std::string FragmentSource;
};

Shader::Shader(const std::string& filePath, uint32_t freeSlot, struct DUI* dui)
    : m_FreeSlotIndex(4)
{
    m_RendererID = ShaderLoader::CreateShader(filePath, dui);
}

Shader::~Shader()
{
    glDeleteProgram(m_RendererID);
}

void Shader::Bind() const
{
    GLCall(glUseProgram(m_RendererID));
}

void Shader::Unbind() const
{
    GLCall(glUseProgram(0));
}

void Shader::SetUniform1f(const std::string& name, const float& value)
{
    SET_UNIFORM(glUniform1f(location, value));
}

void Shader::SetUniform2f(const std::string& name, const glm::vec2& value)
{
    SET_UNIFORM(glUniform2f(location, value.x, value.y));
}

void Shader::SetUniform3f(const std::string& name, const glm::vec3& value)
{
    SET_UNIFORM(glUniform3f(location, value.x, value.y, value.z))
}

void Shader::SetUniform4f(const std::string& name, const glm::vec4& value)
{
    SET_UNIFORM(glUniform4f(location, value.x, value.y, value.z, value.w));
}

void Shader::SetUniform1i(const std::string& name, int32_t value)
{
    SET_UNIFORM(glUniform1i(location, value));
}

void Shader::SetUniformMatrix4f(const std::string& name, const glm::mat4& value)
{
    SET_UNIFORM(glUniformMatrix4fv(location, 1, false, glm::value_ptr(value)));
}

int Shader::GetUniformLocation(const std::string& name)
{
    if(m_UniformLocations.find(name) != m_UniformLocations.end())
    {
        return m_UniformLocations.at(name);
    }

    GLCall(const int location = glGetUniformLocation(m_RendererID, name.c_str()));

    m_UniformLocations.emplace(name, location);
    return location;
}
