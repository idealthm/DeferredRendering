#pragma once

#include <memory>
#include <vector>
#include <glm/glm.hpp>

#include "FrameBuffer/FrameBuffer.h"
#include "UnifromBuffer/ParamBuffer.h"

class Scene;
class RenderPass;
class RenderPipeline;
class Texture2D;
class TextureCube;
struct LightData;

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
    uint32_t renderMode;
    uint32_t usedTextureSlot;

    glm::u32vec2 viewportSize = {1600, 900};
    float ShadowWidth, ShadowHeight;

    Ref<Texture2D> BRDF_LUT;

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
    Ref<TextureCube> IBL_IrradianceMap;
    Ref<TextureCube> IBL_PreFilterMap;

    Ref<Texture2D> Sky_SceneColor;

    Ref<FrameBuffer> FrameBuffer;

    Ref<Texture2D> Final_SceneColor;

    Ref<ParamBuffer<FrameData>> FrameDataUB;
    Ref<ParamBuffer<LightData>> LightDataUB;
};

class Renderer
{
public:
    static Renderer& Get();

    void Init(uint32_t width, uint32_t height);

    void Shutdown();

    void OnWindowResize(int32_t width, int32_t height);

    void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

    void SetClearColor(const glm::vec4& color);

    void StartPass(const Ref<Scene>& scene, const Ref<RenderPass>& renderPass, const glm::u32vec2& viewportSize, RenderContext& ctx);

    RenderPipeline& GetPipeline();

private:
    Scope<RenderPipeline> m_Pipeline;
};
