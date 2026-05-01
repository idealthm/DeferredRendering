#pragma once
#include "Common/Core.h"

class UniformBuffer
{
public:
	UniformBuffer(uint32_t size, uint32_t binding);
	virtual ~UniformBuffer();

	virtual void Update(const void* data, uint32_t size, uint32_t offset = 0);
private:
	uint32_t m_RendererID = 0;
};
