#pragma once
#include "../VertexBufferLayout.h"
#include "../VertexBuffer/VertexBuffer.h"

class VertexArray
{
public:
    VertexArray();
    ~VertexArray();

    void AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout);

    void Bind() const;
    void Unbind() const;

private:
    unsigned int m_RendererID;
};
