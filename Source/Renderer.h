#pragma once

#include <memory>
#include <vector>
#include <glm/glm.hpp>

#include "Scene.h"
#include "RenderPass/LightingPass.h"

class ShadowPass;
class GBufferPass;
class RenderPass;
class Shader;

struct RenderContext
{
    Ref<Scene> scene;

    uint32 renderMode;
    uint32 usedTextureSlot;
};

class Renderer
{
public:
    static Renderer& Get();

    void Init(uint32 width, uint32 height);

    void Shutdown();

    void OnWindowResize(int32 width, int32 height);

    void SetViewport(uint32 x, uint32 y, uint32 width, uint32 height);

    void SetClearColor(const glm::vec4& color);

    Ref<GBufferPass> GetGBufferPass() const { return m_GBufferPass;}
    Ref<ShadowPass> GetShadowPass() const { return m_ShadowPass;}
    Ref<LightingPass> GetLightingPass() const { return m_LightingPass;}

    template<typename T, typename ...Args, std::enable_if_t<std::is_base_of_v<RenderPass, T>, int> = 0>
    std::shared_ptr<T> AddPass(Args... args)
    {
        auto Ref = std::make_shared<T>(std::forward<Args>(args)...);
        m_Passes.push_back(Ref);
        return Ref;
    }

    void Draw(const Ref<Scene>& scene);

    void StartPass();

public:
    uint32 m_RenderMode = 0;

private:

    std::vector<Ref<RenderPass>> m_Passes;

    Ref<GBufferPass>    m_GBufferPass;
    Ref<ShadowPass>     m_ShadowPass;
    Ref<LightingPass>   m_LightingPass;
};
