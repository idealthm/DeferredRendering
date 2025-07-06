#pragma once

#define ASSERT(x) if(!(x)) __debugbreak();

#ifdef _DEBUG
    #define GL_CALL_DEBUG_HEAD GLClearError();
    #define GL_CALL_DEBUG_END  ASSERT(GLLogCall(#x, __FILE__, __LINE__))
#else
    #define GL_CALL_DEBUG_HEAD 
    #define GL_CALL_DEBUG_END 
#endif

#define GLCall(x) GL_CALL_DEBUG_HEAD \
    x; \
    GL_CALL_DEBUG_END

#include <memory>
#include <vector>
#include <glm/glm.hpp>

#include "Scene.h"

class RenderPass;
class Shader;

void GLClearError();

bool GLLogCall(const char* function, const char* file, int line);

class Renderer
{
public:
    static Renderer& Get();

    void Clear(const glm::vec4& color);

    template<typename T, typename ...Args, std::enable_if_t<std::is_base_of_v<RenderPass, T>, int> = 0>
    std::shared_ptr<T> AddPass(Args&& ... args)
    {
        auto Ref = std::make_shared<T>(std::forward<Args>(args)...);
        Ref->Init(m_Scene);
        m_Passes.push_back(Ref);
        return Ref;
    }

    void Draw();
    void SetScene(std::shared_ptr<Scene>& scene);

private:
    std::vector<std::shared_ptr<RenderPass>> m_Passes;
    std::shared_ptr<Scene> m_Scene;
};
