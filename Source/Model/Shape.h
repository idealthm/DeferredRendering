#pragma once

#include "Common/Core.h"
#include <memory>

class VertexBuffer;
class Shader;
class IndexBuffer;
class VertexArray;

class Shape
{
public:
    virtual ~Shape();

    virtual void Draw(Shader& shader);
protected:
    uint32 RenderID = 0;

    std::unique_ptr<VertexArray>    m_VertexArray;
    std::unique_ptr<VertexBuffer>   m_VertexBuffer;
    std::unique_ptr<IndexBuffer>    m_IndexBuffer;
};


class Cube : public Shape
{
public:
    Cube();
};

class Quad : public Shape
{
public:
    Quad();
};

class Plane : public Cube
{
public:
    Plane();
};