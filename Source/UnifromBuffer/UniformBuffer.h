#pragma once
#include "Common/Core.h"

class UniformBuffer
{
public:
	UniformBuffer(uint32_t size, uint32_t binding);
	virtual ~UniformBuffer();

	virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0);
private:
	uint32 m_RendererID = 0;
};
