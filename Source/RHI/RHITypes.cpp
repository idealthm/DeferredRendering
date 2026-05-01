#include "RHITypes.h"

#include "GL/glew.h"

namespace RHI_Internal
{
	using namespace RHI;

	uint32_t GetGLFormat(RHI::Format fmt)
	{
		switch (fmt)
		{
		case Format::R8: return GL_RED;
		case Format::RG8: return GL_RG;
		case Format::RGB8: return GL_RGB;
		case Format::RGBA8: return GL_RGBA;
		case Format::SRGB8: return GL_RGB;
		case Format::SRGBA8: return GL_RGBA;
		case Format::RGB16F: return GL_RGB;
		case Format::R11G11B10F: return GL_RGB;
		case Format::RG16F: return GL_RG;
		case Format::RGBA16F: return GL_RGBA;
		case Format::Depth16:
		case Format::Depth24:
		case Format::Depth32F: return GL_DEPTH_COMPONENT;
		case Format::Depth24Stencil8:
		case Format::Depth32FStencil8: return GL_DEPTH_STENCIL;
		default: return GL_RGBA;
		}
	}

	uint32_t GetGLInternalFormat(RHI::Format fmt, bool sRGB)
	{
		switch (fmt)
		{
		case Format::R8: return GL_R8;
		case Format::RG8: return GL_RG8;
		case Format::RGB8: return sRGB ? GL_SRGB8 : GL_RGB8;
		case Format::RGBA8: return sRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8;
		case Format::SRGB8: return GL_SRGB8;
		case Format::SRGBA8: return GL_SRGB8_ALPHA8;
		case Format::RGB16F: return GL_RGB16F;
		case Format::R11G11B10F: return GL_R11F_G11F_B10F;
		case Format::RG16F: return GL_RG16F;
		case Format::RGBA16F: return GL_RGBA16F;
		case Format::Depth16: return GL_DEPTH_COMPONENT16;
		case Format::Depth24: return GL_DEPTH_COMPONENT24;
		case Format::Depth32F: return GL_DEPTH_COMPONENT32F;
		case Format::Depth24Stencil8: return GL_DEPTH24_STENCIL8;
		case Format::Depth32FStencil8: return GL_DEPTH32F_STENCIL8;
		default: return sRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8;
		}
	}

	uint32_t GetGLType(RHI::Format fmt)
	{
		switch (fmt)
		{
		case Format::R8:
		case Format::RG8:
		case Format::RGB8:
		case Format::RGBA8:
		case Format::SRGB8:
		case Format::SRGBA8: return GL_UNSIGNED_BYTE;
		case Format::RGB16F:
		case Format::RG16F:
		case Format::RGBA16F:
		case Format::Depth32F: return GL_FLOAT;
		case Format::R11G11B10F: return GL_UNSIGNED_INT_10F_11F_11F_REV;
		case Format::Depth16:
		case Format::Depth24: return GL_UNSIGNED_INT;
		case Format::Depth24Stencil8: return GL_UNSIGNED_INT_24_8;
		case Format::Depth32FStencil8: return GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
		default: return GL_UNSIGNED_BYTE;
		}
	}

	uint32_t GetGLWrapMode(RHI::SamplerWrapMode mode)
	{
		switch (mode)
		{
		case RHI::SamplerWrapMode::Repeat: return GL_REPEAT;
		case RHI::SamplerWrapMode::MirroredRepeat: return GL_MIRRORED_REPEAT;
		case RHI::SamplerWrapMode::ClampToEdge: return GL_CLAMP_TO_EDGE;
		case RHI::SamplerWrapMode::ClampToBorder: return GL_CLAMP_TO_BORDER;
		case RHI::SamplerWrapMode::MirrorClampToEdge: return GL_MIRROR_CLAMP_TO_EDGE_EXT;
		default: return GL_REPEAT;
		}
	}

	uint32_t GetGLFilter(RHI::SamplerMinFilter filter)
	{
		switch (filter) {
		case SamplerMinFilter::Nearest:              return GL_NEAREST;
		case SamplerMinFilter::Linear:               return GL_LINEAR;
		case SamplerMinFilter::NearestMipmapNearest: return GL_NEAREST_MIPMAP_NEAREST;
		case SamplerMinFilter::LinearMipmapNearest:  return GL_LINEAR_MIPMAP_NEAREST;
		case SamplerMinFilter::NearestMipmapLinear:  return GL_NEAREST_MIPMAP_LINEAR;
		case SamplerMinFilter::LinearMipmapLinear:   return GL_LINEAR_MIPMAP_LINEAR;
		}
		return GL_LINEAR;
	}

	uint32_t GetGLFilter(RHI::SamplerMagFilter filter)
	{
		switch (filter) {
		case SamplerMagFilter::Nearest: return GL_NEAREST;
		case SamplerMagFilter::Linear:  return GL_LINEAR;
		}
		return GL_LINEAR;
	}

	uint32_t GetGLCompareFunc(RHI::SamplerCompareFunc func)
	{
		switch (func)
		{
		case RHI::SamplerCompareFunc::Never: return GL_NEVER;
		case RHI::SamplerCompareFunc::Less: return GL_LESS;
		case RHI::SamplerCompareFunc::Equal: return GL_EQUAL;
		case RHI::SamplerCompareFunc::LessEqual: return GL_LEQUAL;
		case RHI::SamplerCompareFunc::Greater: return GL_GREATER;
		case RHI::SamplerCompareFunc::NotEqual: return GL_NOTEQUAL;
		case RHI::SamplerCompareFunc::GreaterEqual: return GL_GEQUAL;
		case RHI::SamplerCompareFunc::Always: return GL_ALWAYS;
		default: return GL_LESS;
		}
	}

	uint32_t GetGLTextureTarget(RHI::Sampler dim)
	{
		switch (dim)
		{
		case RHI::Sampler::Dim1D: return GL_TEXTURE_1D;
		case RHI::Sampler::Dim2D: return GL_TEXTURE_2D;
		case RHI::Sampler::Dim3D: return GL_TEXTURE_3D;
		case RHI::Sampler::DimCube: return GL_TEXTURE_CUBE_MAP;
		case RHI::Sampler::Dim2DArray: return GL_TEXTURE_2D_ARRAY;
		default: return GL_TEXTURE_2D;
		}
	}
}
