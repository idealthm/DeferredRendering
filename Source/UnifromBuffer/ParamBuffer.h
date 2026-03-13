#pragma once
#include "UniformBuffer.h"

template<typename T>
class ParamBuffer
{
public:
	using ParamType = T;

	ParamBuffer(uint32 binding)
		: UniformBuffer(CreateRef<class UniformBuffer>(sizeof(T), binding))
	{
	}

	~ParamBuffer() = default;

	void Update()
	{
		UniformBuffer->Update(&Data, sizeof(T), 0);
	}

	T Data;

private:
	Ref<UniformBuffer> UniformBuffer;
};
