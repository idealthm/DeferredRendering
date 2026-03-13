#include "UniformBuffer.h"

#include "Renderer.h"
#include "glad/glad.h"

UniformBuffer::UniformBuffer(uint32_t size, uint32_t binding)
{
	GLCall(glCreateBuffers(1, &m_RendererID));
	GLCall(glBindBuffer(GL_UNIFORM_BUFFER, m_RendererID));
	GLCall(glNamedBufferData(m_RendererID, size, nullptr, GL_DYNAMIC_DRAW)); // TODO: investigate usage hint
	GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_RendererID));
}

UniformBuffer::~UniformBuffer()
{
	GLCall(glDeleteBuffers(1, &m_RendererID));
}

void UniformBuffer::Update(const void* data, uint32_t size, uint32_t offset)
{
	GLCall(glBindBuffer(GL_UNIFORM_BUFFER, m_RendererID));
	GLCall(glNamedBufferSubData(m_RendererID, offset, size, data));
}

