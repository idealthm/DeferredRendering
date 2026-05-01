#pragma once
#include "VertexBuffer/VertexBufferLayout.h"
#include "VertexBuffer/VertexBuffer.h"

class IndexBuffer;

class VertexArray
{
public:
    VertexArray();
    ~VertexArray();

    void AddBuffer(const Ref<VertexBuffer>& vb);
    void SetIndexBuffer(const Ref<IndexBuffer>& ib);

    void Bind() const;
    void Unbind() const;

private:
    uint32_t                        m_RendererID;
    uint32_t                        m_VertexBufferIndex;
    std::vector<Ref<VertexBuffer>>  m_VertexBuffers;
    Ref<IndexBuffer>                m_IndexBuffer;
};
