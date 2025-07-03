#pragma once

#pragma message("当前工作目录: " __FILE__)
#include "glm/vec3.hpp"
#include <vector>
#include <glad/glad.h>

class VertexBuffer
{
public:
    VertexBuffer(const void* data, unsigned int size);
    ~VertexBuffer();

    void Bind() const;
    void Unbind() const;

private:
    unsigned int m_RendererID;
};
