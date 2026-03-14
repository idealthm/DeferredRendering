#include "Renderer.h"
#include <iostream>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "Scene.h"
#include "FrameBuffer/FrameBuffer.h"
#include "IndexBuffer/IndexBuffer.h"
#include "Model/Texture.h"
#include "RenderPass/ERPPass.h"
#include "RenderPass/GBufferPass.h"
#include "RenderPass/ShadowPass.h"
#include "RenderPass/SkyLightPass.h"
#include "RenderPass/ToneMapping.h"


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

    glEnable(GL_LINE_SMOOTH);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    PostRendererInit();
}

void Renderer::Shutdown()
{
    GDefaultTextures.White = nullptr;
    GDefaultTextures.Black = nullptr;
    GDefaultTextures.Gray = nullptr;
    GDefaultTextures.Normal = nullptr;
}

void Renderer::OnWindowResize(int32 width, int32 height)
{
    SetViewport(0, 0, width, height);
}

void Renderer::SetViewport(uint32 x, uint32 y, uint32 width, uint32 height)
{
    // glViewport(x, y, width, height);
}

void Renderer::SetClearColor(const glm::vec4& color)
{
    glClearColor(color.x, color.y, color.z, color.w);
}

void Renderer::PostRendererInit()
{
    GDefaultTextures.White  = Texture2D::Create(0xFFFFFFFF);
    GDefaultTextures.Black  = Texture2D::Create(0xFF000000);
    GDefaultTextures.Gray   = Texture2D::Create(0xFF808080);
    GDefaultTextures.Normal = Texture2D::Create(0xFFFF8080);
}

void Renderer::Render(Ref<Scene>& scene)
{
    Ref<GBufferPass> gBufferPass = CreateRef<GBufferPass>(scene->GetWidth(), scene->GetHeight());
    Ref<ShadowPass> shadowPass = CreateRef<ShadowPass>(2048.f, 2048.f);
    Ref<LightingPass> lightPass = CreateRef<LightingPass>(scene->GetWidth(), scene->GetHeight());
    Ref<SkyLightPass> skyLightPass = CreateRef<SkyLightPass>(scene->GetWidth(), scene->GetHeight());
    Ref<ToneMapping> toneMappingPass = CreateRef<ToneMapping>(scene->GetWidth(), scene->GetHeight());

    StartPass(scene, shadowPass);
    StartPass(scene, gBufferPass);
    StartPass(scene, lightPass);
    StartPass(scene, toneMappingPass);
}

void Renderer::StartPass(Ref<Scene>& scene, Ref<RenderPass> renderPass)
{
    RenderContext& ctx = scene->GetRenderContext();
    FBAttachmentInfo FBInfo;
    renderPass->Setup(ctx, FBInfo);
    BuildTextures(FBInfo);
    ctx.FrameBuffer->Attach(FBInfo);
    renderPass->Execute(scene);
}

void Renderer::BuildTextures(FBAttachmentInfo& info)
{
    auto TextureValidate = [](FBTextureAttachment& desc)
    {
        if (!desc.Texture)
            return;

        if (!desc.GetTexture() || desc.GetTexture()->GetDesc() != desc.Desc)
            *desc.Texture = CreateScope<Texture2D>(desc.Desc);
        else
            desc.GetTexture()->SetTextureParameter(desc.Desc);
    };

    TextureValidate(info.Depth);

    for (auto& desc : info.Attachments)
    {
        TextureValidate(desc);
    }
}

