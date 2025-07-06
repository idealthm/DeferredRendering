#include "Renderer.h"
#include <iostream>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "IndexBuffer/IndexBuffer.h"
#include "RenderPass/RenderPass.h"
#include "Shader/Shader.h"
#include "VertexArray/VertexArray.h"


void GLClearError()
{
    while(glGetError() != GL_NO_ERROR);
}

bool GLLogCall(const char* function, const char* file, int line)
{
    while(GLenum error = glGetError())
    {
        std::cout << "[OpenGL Error] (" << error << "):" << function << " " << file << ":" << line << std::endl;
        return false;
    }
    return true;
}

Renderer& Renderer::Get()
{
    static Renderer renderer;
    return renderer;
}

void Renderer::Clear(const glm::vec4& color)
{
    // glClearColor(color.x, color.y, color.z, color.w);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::Draw()
{
    Clear(glm::vec4(0.0));

    for (const auto& pass : m_Passes)
    {
        pass->PrePass(m_Scene);

        pass->OnPass(m_Scene);

        pass->PostPass(m_Scene);
    }
}

void Renderer::SetScene(std::shared_ptr<Scene>& scene)
{
    m_Scene = scene;
}
