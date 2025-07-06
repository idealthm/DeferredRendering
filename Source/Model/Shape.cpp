#include "Shape.h"

#include <glad/glad.h>

#include "IndexBuffer/IndexBuffer.h"
#include "Shader/Shader.h"
#include "VertexArray/VertexArray.h"

static uint32 CubeIndices[] = {
	0, 1, 2, 1, 0, 3,
	4, 5, 6, 6, 7, 4,
	8, 9, 10, 10, 11, 8,
	12, 13, 14, 13, 12, 15,
	16, 17, 18, 18, 19, 16,
	20, 21, 22, 21, 20, 23
};

static float CubeVertices[] = {
    // back face
    -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
     1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
     1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
    -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
    // front face
    -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
     1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
     1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
    -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
    // left face
    -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
    -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
    -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
    -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
    // right face
     1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
     1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
     1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
     1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
    // bottom face
    -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
     1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
     1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
    -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
    // top face
    -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
     1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
     1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
    -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
};

static uint32 QuadIndices[] = {
	0, 1, 2, 1, 2, 3
};

static float QuadVertices[] = {
 	// positions        // texture Coords
 	-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
 	-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
 	 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
 	 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
};

Shape::~Shape()
{
	glDeleteVertexArrays(1, &RenderID);
}

void Shape::Draw(Shader& shader)
{
	shader.Bind();
	m_VertexArray->Bind();
	m_IndexBuffer->Bind();

	glDrawElements(GL_TRIANGLES, m_IndexBuffer->GetCount(), GL_UNSIGNED_INT, nullptr);
}

Cube::Cube()
{
	m_VertexArray = std::make_unique<VertexArray>();
	m_VertexBuffer = std::make_unique<VertexBuffer>(CubeVertices, sizeof(CubeVertices));
	m_IndexBuffer = std::make_unique<IndexBuffer>(CubeIndices, sizeof(CubeIndices));

	VertexBufferLayout layout;
	layout.Push<float>(3);
	layout.Push<float>(3);
	layout.Push<float>(2);

	m_VertexArray->AddBuffer(*m_VertexBuffer, layout);
}

Quad::Quad()
{
	m_VertexArray = std::make_unique<VertexArray>();
	m_VertexBuffer = std::make_unique<VertexBuffer>(QuadVertices, sizeof(QuadVertices));
	m_IndexBuffer = std::make_unique<IndexBuffer>(QuadIndices, sizeof(QuadIndices));

	VertexBufferLayout layout;
	layout.Push<float>(3);
	layout.Push<float>(2);

	m_VertexArray->AddBuffer(*m_VertexBuffer, layout);
}


Plane::Plane()
	: Cube()
{
	m_VertexArray = std::make_unique<VertexArray>();
	m_VertexBuffer = std::make_unique<VertexBuffer>(CubeVertices, sizeof(CubeVertices));
	m_IndexBuffer = std::make_unique<IndexBuffer>(CubeIndices, sizeof(CubeIndices));

	VertexBufferLayout layout;
	layout.Push<float>(3);
	layout.Push<float>(3);
	layout.Push<float>(2);

	m_VertexArray->AddBuffer(*m_VertexBuffer, layout);
}