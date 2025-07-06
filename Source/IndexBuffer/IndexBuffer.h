#pragma once

#include "Common/Core.h"
#include "glm/glm.hpp"

class IndexBuffer
{
public:
    IndexBuffer(const unsigned int* data, unsigned int count);
    ~IndexBuffer();

    void Bind() const;
    void Unbind() const;

    int32 GetCount() const {return static_cast<int32>(m_Count);}

private:
    uint32 m_RendererID;
    uint32 m_Count;
};
