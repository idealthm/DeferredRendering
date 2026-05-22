#pragma once

#include <memory>
#include <vector>
#include <glm/glm.hpp>

#include "UibStruct.h"
#include "FrameBuffer/FrameBuffer.h"
#include "UnifromBuffer/ParamBuffer.h"

class Scene;
class RenderPass;
class RenderPipeline;
class Texture;


struct RenderContext
{
    uint32_t renderMode;
    uint32_t usedTextureSlot;

    glm::u32vec2 viewportSize = {1600, 900};
    float ShadowWidth, ShadowHeight;

    Ref<Texture> BRDF_LUT;

    Ref<Texture> GBuffer_Position;
    Ref<Texture> GBuffer_Normal;
    Ref<Texture> GBuffer_Albedo;
    Ref<Texture> GBuffer_Material;
    Ref<Texture> GBuffer_Depth;

    Ref<Texture> Test;

    Ref<Texture> ShadowMap_Depth;

    Ref<Texture> LightMap_SceneColor;
    Ref<Texture> LightMap_SceneDepth;

    Ref<Texture> ERP_Cubemap;
    Ref<Texture> IBL_IrradianceMap;
    Ref<Texture> IBL_PreFilterMap;

    Ref<Texture> Sky_SceneColor;

    Ref<FrameBuffer> FrameBuffer;

    Ref<Texture> Final_SceneColor;

    TypedUniformBuffer<PerViewUib> FrameDataUB;
    TypedUniformBuffer<LightData> LightDataUB;
    TypedUniformBuffer<ModelData> ModelDataUB;
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
