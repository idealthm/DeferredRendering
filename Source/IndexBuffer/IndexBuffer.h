#pragma once

#include "Common/Core.h"
#include "glm/glm.hpp"

class IndexBuffer
{
public:
    IndexBuffer(const uint32_t * data, uint32_t count);
    ~IndexBuffer();

    void Bind() const;
    void Unbind() const;

    int32_t GetCount() const {return static_cast<int32_t>(m_Count);}

private:
    uint32_t m_RendererID;
    uint32_t m_Count;
};
