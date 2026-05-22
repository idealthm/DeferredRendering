#pragma once
#include <array>
#include "glad/glad.h"
#include "RHI/RHIDriver.h"
#include "GLHelper.h"

namespace RHI
{

struct GLTextureRef {
	GLTextureRef() = default;
	// view reference counter
	uint16_t count = 1;
	// Current per-view values of the texture (in GL we can only have a single View active at
	// a time, and this tracks that state). It's used to avoid unnecessarily change state.
	int8_t baseLevel = 127;
	int8_t maxLevel = -1;
};
	
struct GLTexture : public HwTexture {
	using HwTexture::HwTexture;
	struct GL {
		GL() noexcept : imported(false), sidecarSamples(1), reserved1(0) {}
		GLuint id = 0;          // texture or renderbuffer id
		GLenum target = 0;
		GLenum internalFormat = 0;
		GLuint sidecarRenderBufferMS = 0;  // multi-sample sidecar renderbuffer
		// texture parameters go here too
		GLfloat anisotropy = 1.0;
		int8_t baseLevel = 127;
		int8_t maxLevel = -1;
		uint8_t reserved0 = 0;
		bool imported           : 1;
		uint8_t sidecarSamples  : 3;
		uint8_t reserved1       : 4;
	} gl;
	mutable Handle<GLTextureRef> ref;
};

}
