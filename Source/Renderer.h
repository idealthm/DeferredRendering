#pragma once

#define ASSERT(x) if(!(x)) __debugbreak();

#define GLCall(x) GLClearError();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))
#include <vec4.hpp>

class Shader;
class IndexBuffer;
class VertexArray;
void GLClearError();

bool GLLogCall(const char* function, const char* file, int line);

class Renderer
{
public:
    static void Clear(const glm::vec4& color);
    static void Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader);
};
