#include "Renderer.h"
#include <iostream>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "IndexBuffer/IndexBuffer.h"
#include "RenderPass/GBufferPass.h"
#include "RenderPass/RenderPass.h"


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

void Renderer::Init(uint32 width, uint32 height)
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

    m_GBufferPass = CreateRef<GBufferPass>(width, height);
    m_LightingPass = CreateRef<LightingPass>(width, height);
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

void Renderer::Draw(const Ref<Scene>& scene)
{
    RenderContext context {scene, m_RenderMode, 0};

    m_GBufferPass->OnPass(context);

    m_LightingPass->OnPass(context);
}

void Renderer::StartPass()
{
    
}

