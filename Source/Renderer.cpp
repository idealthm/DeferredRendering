#include "Renderer.h"
#include <iostream>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "IndexBuffer/IndexBuffer.h"
#include "RenderPass/RenderPass.h"
#include "Shader/Shader.h"
#include "VertexArray/VertexArray.h"


namespace 
{
    void OpenGLMessageCallback(
        unsigned source,
        unsigned type,
        unsigned id,
        unsigned severity,
        int length,
        const char* message,
        const void* userParam)
    {
        std::cout << message << std::endl;
    }
}

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

void Renderer::Init()
{
    
#ifdef _DEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(OpenGLMessageCallback, nullptr);
		
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
#endif

    glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
}

void Renderer::Shutdown()
{
    m_Passes.empty();
}

void Renderer::OnWindowResize(int32 width, int32 height)
{
    SetViewport(0, 0, width, height);
}

void Renderer::SetViewport(uint32 x, uint32 y, uint32 width, uint32 height)
{
    glViewport(0, 0, width, height);
}

void Renderer::SetClearColor(const glm::vec4& color)
{
    glClearColor(color.x, color.y, color.z, color.w);
}

void Renderer::Clear(const glm::vec4& color)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::Draw(const Ref<Scene>& scene)
{
    Clear(glm::vec4(0.0));

    for (const auto& pass : m_Passes)
    {
        pass->PrePass(scene);

        pass->OnPass(scene);

        pass->PostPass(scene);
    }
}

