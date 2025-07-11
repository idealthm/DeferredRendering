#pragma once

#include <memory>
#include <vector>
#include <glm/glm.hpp>

#include "Scene.h"

class VertexArray;
class RenderPass;
class Shader;

class Renderer
{
public:
    static Renderer& Get();

    void Init();

    void Shutdown();

    void OnWindowResize(int32 width, int32 height);

    void SetViewport(uint32 x, uint32 y, uint32 width, uint32 height);

    void Clear(const glm::vec4& color);

    void SetClearColor(const glm::vec4& color);

    void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32 count = 0);

    template<typename T, typename ...Args, std::enable_if_t<std::is_base_of_v<RenderPass, T>, int> = 0>
    std::shared_ptr<T> AddPass(Args&& ... args)
    {
        auto Ref = std::make_shared<T>(std::forward<Args>(args)...);
        m_Passes.push_back(Ref);
        return Ref;
    }

    void Draw(const Ref<Scene>& scene);
private:
    std::vector<Ref<RenderPass>> m_Passes;
};
