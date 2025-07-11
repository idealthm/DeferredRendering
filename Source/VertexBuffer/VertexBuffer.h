#pragma once

#include "VertexBufferLayout.h"

class VertexBuffer
{
public:
    VertexBuffer(const void* data, unsigned int size);
    ~VertexBuffer();

    void Bind() const;
    void Unbind() const;

    void SetLayout(const BufferLayout& Layout);
    BufferLayout& GetLayout() {return m_Layout;}

private:
    unsigned int m_RendererID;
    BufferLayout m_Layout;
};
