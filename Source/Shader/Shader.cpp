#include "Shader.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>

#include "Renderer.h"

struct ShaderProgramSource
{
    std::string VertexSource;
    std::string FragmentSource;
};

ShaderProgramSource Shader::ParseShader(const std::string& filepath) const
{
    std::ifstream stream(filepath);

    enum class ShaderType
    {
        None = -1,
        Vertex,
        Fragment,
    };

    std::string line;
    std::stringstream ss[2];

    ShaderType type = ShaderType::None;

    while(getline(stream, line))
    {
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
                type = ShaderType::Vertex;
            else if (line.find("fragment") != std::string::npos)
                type = ShaderType::Fragment;
        }
        else
        {
            ss[(int)type] << line << '\n';
        }
    }
    return {ss[0].str(), ss[1].str()};
}

unsigned int Shader::CompileShader(unsigned int type, const std::string& source)
{
    GLCall(unsigned int id = glCreateShader(type));
    const char* src = source.c_str();
    GLCall(glShaderSource(id, 1, &src, nullptr));
    GLCall(glCompileShader(id));

    int result;
    GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
    if (result == GL_FALSE)
    {
        int length;
        GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
        char* message = (char *)alloca(sizeof(float) * length);
        GLCall(glGetShaderInfoLog(id, length, &length, message));

        std::cout << "Fail to compile shader!" << (type == GL_VERTEX_SHADER ? "vertex" : "pixel") << std::endl;
        std::cout << message << std::endl;

        GLCall(glDeleteShader(id));
        return 0;
    }

    return id;
}

unsigned int Shader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    GLCall(glAttachShader(program, vs));
    GLCall(glAttachShader(program, fs));
    GLCall(glLinkProgram(program));
    GLCall(glValidateProgram(program));

    GLCall(glDeleteShader(vs));
    GLCall(glDeleteShader(fs));
	
    return program;
}

Shader::Shader(const std::string& filepath)
    : m_FilePath(filepath), m_RendererID(0)
{
    const ShaderProgramSource source = ParseShader(m_FilePath);
    m_RendererID = CreateShader(source.VertexSource, source.FragmentSource);
}

Shader::~Shader()
{
    GLCall(glDeleteProgram(m_RendererID));
}

void Shader::Bind() const
{
    GLCall(glUseProgram(m_RendererID));
}

void Shader::Unbind() const
{
    GLCall(glUseProgram(0));
}

void Shader::SetUniform3f(const std::string& name, const glm::vec3& value)
{
    GLCall(glUniform3f(GetUniformLocation(name), value.x, value.y, value.z));
}

void Shader::SetUniform4f(const std::string& name, const glm::vec4& value)
{
    GLCall(glUniform4f(GetUniformLocation(name), value.x, value.y, value.z, value.w));
}

void Shader::SetUniform1i(const std::string& name, int32 value)
{
    GLCall(glUniform1i(GetUniformLocation(name), value));
}

void Shader::SetUniformMatrix4f(const std::string& name, const glm::mat4& value)
{
    GLCall(glUniformMatrix4fv(GetUniformLocation(name), 1, false, glm::value_ptr(value)));
}

int Shader::GetUniformLocation(const std::string& name)
{
    if(m_UniformLocations.find(name) != m_UniformLocations.end())
    {
        return m_UniformLocations.at(name);
    }

    GLCall(const int location = glGetUniformLocation(m_RendererID, name.c_str()));
    ASSERT(location >= 0);

    m_UniformLocations.emplace(name, location);
    return location;
}
