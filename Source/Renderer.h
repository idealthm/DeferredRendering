#pragma once

#include <memory>
#include <vector>
#include <glm/glm.hpp>

#include "FrameBuffer/FrameBuffer.h"
#include "UnifromBuffer/ParamBuffer.h"

class LightingPass;
class Scene;
class ToneMapping;
class SkyLightPass;
struct LightData;
struct FBAttachmentInfo;
class UniformBuffer;
enum class EShaderType;
class TextureCube;
class Texture2D;
class ERPPass;
class ShadowPass;
class GBufferPass;
class RenderPass;
class Shader;

struct FrameData
{
    glm::mat4 m_ViewProjection;
    glm::mat4 m_Projection;
    glm::mat4 m_InvViewProjection;
    glm::mat4 m_InvProjection;
    glm::vec3 m_CameraPosition;
    int RenderMode;
};

struct RenderContext
{
    uint32 renderMode;
    uint32 usedTextureSlot;

    glm::u32vec2 viewportSize;
    float ShadowWidth, ShadowHeight;

    Ref<Texture2D> GBuffer_Position;
    Ref<Texture2D> GBuffer_Normal;
    Ref<Texture2D> GBuffer_Albedo;
    Ref<Texture2D> GBuffer_Material;
    Ref<Texture2D> GBuffer_Depth;

    Ref<Texture2D> Test;

    Ref<Texture2D> ShadowMap_Depth;

    Ref<Texture2D> LightMap_SceneColor;
    Ref<Texture2D> LightMap_SceneDepth;

    Ref<TextureCube> ERP_Cubemap;

    Ref<Texture2D> Sky_SceneColor;

    Ref<FrameBuffer> FrameBuffer;

    Ref<Texture2D> Final_SceneColor;

    Ref<ParamBuffer<FrameData>> FrameDataUB;
    Ref<ParamBuffer<LightData>> LightDataUB;
};

extern RenderContext g_ctx;

class Renderer
{
public:
    static Renderer& Get();

    void Init(uint32 width, uint32 height);

    void Shutdown();

    void OnWindowResize(int32 width, int32 height);

    void SetViewport(uint32 x, uint32 y, uint32 width, uint32 height);

    void SetClearColor(const glm::vec4& color);

    void PostRendererInit();

    void Render(Ref<Scene>& scene, const glm::u32vec2& viewportSize);

    void StartPass(const Ref<Scene>& scene, const Ref<RenderPass>& renderPass, const glm::u32vec2& viewportSize);

public:
    uint32 m_RenderMode = 0;

    Ref<GBufferPass>    m_GBufferPass;
    Ref<ShadowPass>     m_ShadowPass;
    Ref<LightingPass>   m_LightPass;
    Ref<SkyLightPass>   m_SkyLightPass;
    Ref<ToneMapping>    m_ToneMappingPass;
    Ref<ERPPass>        m_ERPPass;
};
