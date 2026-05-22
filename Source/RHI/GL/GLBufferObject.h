#pragma once
#include "glad/glad.h"
#include "RHI/RHIDriver.h"


struct GLBufferObject : public RHI::HwBufferObject
{
	using HwBufferObject::HwBufferObject;
	GLBufferObject(uint32_t size,
			RHI::BufferObjectBinding bindingType, RHI::BufferUsage usage) noexcept
			: HwBufferObject(size), usage(usage), bindingType(bindingType) {
	}

	struct {
		GLuint id;
		union {
			GLenum binding;
			void* buffer;
		};
	} gl;
	RHI::BufferUsage usage;
	RHI::BufferObjectBinding bindingType;
	uint16_t age = 0;
};
