#include "Shader.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <glad/glad.h>

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
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char *)alloca(sizeof(float) * length);
        glGetShaderInfoLog(id, length, &length, message);

        std::cout << "Fail to compile shader!" << (type == GL_VERTEX_SHADER ? "vertex" : "pixel") << std::endl;
        std::cout << message << std::endl;

        glDeleteShader(id);
        return 0;
    }

    return id;
}

unsigned int Shader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);
	
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

void Shader::SetUniform4f(const std::string& name, const glm::vec4& value)
{
    GLCall(glUniform4f(GetUniformLocation(name), value.x, value.y, value.z, value.w));
}

int Shader::GetUniformLocation(const std::string& name)
{
    if(m_UniformLocations.find(name) != m_UniformLocations.end())
    {
        return m_UniformLocations.at(name);
    }

    GLCall(const int location = glGetUniformLocation(m_RendererID, name.c_str()));
    ASSERT(~location);

    m_UniformLocations.emplace(name, location);
    return location;
}
