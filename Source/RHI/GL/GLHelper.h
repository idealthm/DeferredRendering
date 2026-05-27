#pragma once
#include <array>
#include <cstdint>
#include <iosfwd>

#include "glad/glad.h"
#include "RHI/DriverEnums.h"


namespace RHI_Internal { // 仅用于内部实现文件，不暴露给上层
	using namespace RHI;
	uint32_t GetGLPrimitiveType(PrimitiveType type);
	uint32_t GetGLFormat(Format fmt);
	uint32_t GetGLInternalFormat(Format fmt);
	uint32_t GetGLType(Format fmt);
	uint32_t GetGLWrapMode(SamplerWrapMode mode);
	uint32_t GetGLCompareMode(SamplerCompareMode mode);
	uint32_t GetGLFilter(SamplerMinFilter filter);
	uint32_t GetGLFilter(SamplerMagFilter filter);
	uint32_t GetGLCompareFunc(SamplerCompareFunc func);
	uint32_t GetGLTextureTarget(SamplerType dim);
	uint32_t GetGLBufferBinding(BufferObjectBinding binding);
	uint32_t GetGLElementSize(ElementType type);
	uint32_t GetGLBufferUsage(BufferUsage usage);
	uint32_t GetGLCubeMapFace(uint16_t face);
	uint32_t GetGLFormat(PixelDataFormat fmt);
	uint32_t GetGLType(PixelDataType type);
	uint32_t GetTypeComponentCount(ElementType type);
	uint32_t GetTypeComponentType(ElementType type);
	uint32_t GetGLBlendFunctionMode(BlendFunction func);
	uint32_t GetGLBlendEquationMode(BlendEquation eq);
	uint32_t GetGLCullingMode(CullingMode mode);
	uint32_t GetGLStencilOperation(StencilOperation op);

	const char* getGLError(GLenum error) noexcept;
	GLenum checkGLError(std::ostream& out, const char* function, size_t line) noexcept;
	void assertGLError(const char* function, size_t line) noexcept;

	const char* getFramebufferStatus(GLenum status) noexcept;
	GLenum checkFramebufferStatus(std::ostream& out, GLenum target, const char* function, size_t line) noexcept;
	void assertFramebufferStatus(std::ostream& out, GLenum target, const char* function, size_t line) noexcept;
}

#ifdef NDEBUG
#   define CHECK_GL_ERROR(out)
#   define CHECK_GL_ERROR_NON_FATAL(out)
#   define CHECK_GL_FRAMEBUFFER_STATUS(out, target)
#else
#   define CHECK_GL_ERROR(out) { RHI_Internal::assertGLError(out, __func__, __LINE__); }
#   define CHECK_GL_ERROR_NON_FATAL(out) { RHI_Internal::checkGLError(out, __func__, __LINE__); }
#   define CHECK_GL_FRAMEBUFFER_STATUS(out, target) { RHI_Internal::assertFramebufferStatus(out, target, __func__, __LINE__); }
#endif