#include "Renderer.h"
#include <iostream>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "Engine.h"
#include "RenderPipeline.h"
#include "RenderPass/RenderPass.h"
#include "Scene.h"
#include "FrameBuffer/FrameBuffer.h"
#include "Model/Texture.h"
#include "Lights/Light.h"

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

void Renderer::Init(uint32_t width, uint32_t height)
{
    m_Pipeline = CreateScope<RenderPipeline>();
}

void Renderer::Shutdown()
{
    m_Pipeline = nullptr;

    GDefaultTextures.White = nullptr;
    GDefaultTextures.Black = nullptr;
    GDefaultTextures.Gray = nullptr;
    GDefaultTextures.Normal = nullptr;
}

void Renderer::OnWindowResize(int32_t width, int32_t height)
{
    SetViewport(0, 0, width, height);
}

void Renderer::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    gEngine->GetDriver().SetViewport(x, y, width, height);
}

void Renderer::SetClearColor(const glm::vec4& color)
{
    gEngine->GetDriver().SetClearColor(color.x, color.y, color.z, color.w);
}

void Renderer::StartPass(const Ref<Scene>& scene, const Ref<RenderPass>& renderPass, const glm::u32vec2& viewportSize, RenderContext& ctx)
{
    for (uint32_t i = 0; i < renderPass->GetRenderTimes(); i++)
    {
        FBAttachmentInfo FBInfo;
        FBInfo.Width = viewportSize.x;
        FBInfo.Height = viewportSize.y;
        renderPass->Setup(FBInfo, i, ctx);
        ctx.FrameBuffer->Attach(FBInfo);
        renderPass->Execute(scene, i, ctx);
    }
}

RenderPipeline& Renderer::GetPipeline()
{
    return *m_Pipeline;
}
