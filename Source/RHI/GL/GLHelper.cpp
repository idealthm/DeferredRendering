#include "GLHelper.h"

#include "Common/Core.h"
#include "glad/glad.h"

namespace RHI_Internal
{
	using namespace RHI;

	uint32_t GetGLPrimitiveType(RHI::PrimitiveType type)
	{
		switch (type)
		{
		case PrimitiveType::LINES: return GL_LINES;
		case PrimitiveType::LINE_STRIP: return GL_LINE_STRIP;
		case PrimitiveType::POINTS: return GL_POINTS;
		case PrimitiveType::TRIANGLES: return GL_TRIANGLES;
		case PrimitiveType::TRIANGLE_STRIP: return GL_TRIANGLE_STRIP;
		}
		ASSERT(0);
		return GL_TRIANGLES;
	}

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

	uint32_t GetGLInternalFormat(RHI::Format fmt)
	{
		switch (fmt)
		{
		case Format::R8: return GL_R8;
		case Format::RG8: return GL_RG8;
		case Format::RGB8: return GL_RGB8;
		case Format::RGBA8: return GL_RGBA8;
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
		default: return GL_RGBA8;
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
		case RHI::SamplerWrapMode::MirrorClampToEdge: return GL_MIRROR_CLAMP_TO_EDGE;
		default: return GL_REPEAT;
		}
	}

	uint32_t GetGLCompareMode(SamplerCompareMode mode)
	{
		switch (mode)
		{
			case SamplerCompareMode::COMPARE_TO_TEXTURE: return GL_COMPARE_REF_TO_TEXTURE;
			case SamplerCompareMode::NONE: return GL_NONE;
		}
		ASSERT(0);
		return GL_NONE;
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


	uint32_t GetGLTextureTarget(SamplerType dim)
	{
		switch (dim)
		{
		case SamplerType::SAMPLER_2D: return GL_TEXTURE_2D;
		case SamplerType::SAMPLER_2D_ARRAY: return GL_TEXTURE_2D_ARRAY;
		case SamplerType::SAMPLER_CUBEMAP: return GL_TEXTURE_CUBE_MAP;
		case SamplerType::SAMPLER_3D: return GL_TEXTURE_3D;
		case SamplerType::SAMPLER_CUBEMAP_ARRAY: return GL_TEXTURE_CUBE_MAP_ARRAY;
		default: return GL_TEXTURE_2D;
		}
	}

	uint32_t GetGLBufferBinding(BufferObjectBinding binding)
	{
		switch (binding)
		{
		case BufferObjectBinding::UNIFORM: return GL_UNIFORM_BUFFER;
		case BufferObjectBinding::SHADER_STORAGE: return GL_SHADER_STORAGE_BUFFER;
		case BufferObjectBinding::VERTEX: return GL_ARRAY_BUFFER;
		}
	}

	uint32_t GetGLElementSize(ElementType type)
	{
		switch (type)
		{
		case ElementType::BYTE: return sizeof(int8_t);
		case ElementType::BYTE2: return sizeof(int8_t) * 2;
		case ElementType::BYTE3: return sizeof(int8_t) * 3;
		case ElementType::BYTE4: return sizeof(int8_t) * 4;
		case ElementType::UBYTE: return sizeof(uint8_t);
		case ElementType::UBYTE2: return sizeof(uint8_t) * 2;
		case ElementType::UBYTE3: return sizeof(uint8_t) * 3;
		case ElementType::UBYTE4: return sizeof(uint8_t) * 4;
		case ElementType::SHORT: return sizeof(int16_t);
		case ElementType::SHORT2: return sizeof(int16_t) * 2;
		case ElementType::SHORT3: return sizeof(int16_t) * 3;
		case ElementType::SHORT4: return sizeof(int16_t) * 4;
		case ElementType::USHORT: return sizeof(uint16_t);
		case ElementType::USHORT2: return sizeof(uint16_t) * 2;
		case ElementType::USHORT3: return sizeof(uint16_t) * 3;
		case ElementType::USHORT4: return sizeof(uint16_t) * 4;
		case ElementType::INT: return sizeof(int32_t);
		case ElementType::UINT: return sizeof(uint32_t);
		case ElementType::FLOAT: return sizeof(float);
		case ElementType::FLOAT2: return sizeof(float) * 2;
		case ElementType::FLOAT3: return sizeof(float) * 3;
		case ElementType::FLOAT4: return sizeof(float) * 4;
		case ElementType::HALF: return sizeof(uint16_t);
		case ElementType::HALF2: return sizeof(uint16_t) * 2;
		case ElementType::HALF3: return sizeof(uint16_t) * 3;
		case ElementType::HALF4: return sizeof(uint16_t) * 4;
		default: 
			ASSERT(false);
			return 0;
		}
	}

	uint32_t GetGLBufferUsage(BufferUsage usage)
	{
		switch (usage)
		{
			case BufferUsage::STATIC: return GL_STATIC_DRAW;
			case BufferUsage::DYNAMIC: return GL_DYNAMIC_DRAW;
		}
		ASSERT(false);
		return GL_STATIC_DRAW;
	}

	uint32_t GetGLCubeMapFace(uint16_t face)
	{
		ASSERT(face <= 5);
		return GL_TEXTURE_CUBE_MAP_POSITIVE_X + face;
	}

	uint32_t GetGLFormat(PixelDataFormat fmt)
	{
		using PixelDataFormat = PixelDataFormat;
		switch (fmt) {
			case PixelDataFormat::RGB:              return GL_RGB;
			case PixelDataFormat::RGBA:             return GL_RGBA;
			case PixelDataFormat::UNUSED:           return GL_RGBA; // should never happen (used to be rgbm)
			case PixelDataFormat::DEPTH_COMPONENT:  return GL_DEPTH_COMPONENT;
			case PixelDataFormat::ALPHA:            return GL_ALPHA;
			case PixelDataFormat::DEPTH_STENCIL:    return GL_DEPTH_STENCIL;
			case PixelDataFormat::R:                return GL_RED;
			case PixelDataFormat::R_INTEGER:        return GL_RED_INTEGER;
			case PixelDataFormat::RG:               return GL_RG;
			case PixelDataFormat::RG_INTEGER:       return GL_RG_INTEGER;
			case PixelDataFormat::RGB_INTEGER:      return GL_RGB_INTEGER;
			case PixelDataFormat::RGBA_INTEGER:     return GL_RGBA_INTEGER;
			default: return GL_NONE;
		}
	}

	uint32_t GetGLType(PixelDataType type)
	{
		using PixelDataType = PixelDataType;
		switch (type) {
		case PixelDataType::UBYTE:                return GL_UNSIGNED_BYTE;
		case PixelDataType::BYTE:                 return GL_BYTE;
		case PixelDataType::USHORT:               return GL_UNSIGNED_SHORT;
		case PixelDataType::SHORT:                return GL_SHORT;
		case PixelDataType::UINT:                 return GL_UNSIGNED_INT;
		case PixelDataType::INT:                  return GL_INT;
		case PixelDataType::FLOAT:                return GL_FLOAT;
		case PixelDataType::USHORT_565:           return GL_UNSIGNED_SHORT_5_6_5;
		case PixelDataType::HALF:                 return GL_HALF_FLOAT;
		case PixelDataType::UINT_10F_11F_11F_REV: return GL_UNSIGNED_INT_10F_11F_11F_REV;
		case PixelDataType::UINT_2_10_10_10_REV:  return GL_UNSIGNED_INT_2_10_10_10_REV;
		case PixelDataType::COMPRESSED:           return 0; // should never happen
		default: return GL_NONE;
		}
    }

	uint32_t GetTypeComponentCount(ElementType type)
	{
		using ElementType = ElementType;
		switch (type) {
		case ElementType::BYTE:
		case ElementType::UBYTE:
		case ElementType::SHORT:
		case ElementType::USHORT:
		case ElementType::INT:
		case ElementType::UINT:
		case ElementType::FLOAT:
		case ElementType::HALF:
			return 1;
		case ElementType::FLOAT2:
		case ElementType::HALF2:
		case ElementType::BYTE2:
		case ElementType::UBYTE2:
		case ElementType::SHORT2:
		case ElementType::USHORT2:
			return 2;
		case ElementType::FLOAT3:
		case ElementType::HALF3:
		case ElementType::BYTE3:
		case ElementType::UBYTE3:
		case ElementType::SHORT3:
		case ElementType::USHORT3:
			return 3;
		case ElementType::FLOAT4:
		case ElementType::HALF4:
		case ElementType::BYTE4:
		case ElementType::UBYTE4:
		case ElementType::SHORT4:
		case ElementType::USHORT4:
			return 4;
		}
	}

	uint32_t GetTypeComponentType(ElementType type)
	{
		using ElementType = ElementType;
		switch (type) {
		case ElementType::BYTE:
		case ElementType::BYTE2:
		case ElementType::BYTE3:
		case ElementType::BYTE4:
			return GL_BYTE;
		case ElementType::UBYTE:
		case ElementType::UBYTE2:
		case ElementType::UBYTE3:
		case ElementType::UBYTE4:
			return GL_UNSIGNED_BYTE;
		case ElementType::SHORT:
		case ElementType::SHORT2:
		case ElementType::SHORT3:
		case ElementType::SHORT4:
			return GL_SHORT;
		case ElementType::USHORT:
		case ElementType::USHORT2:
		case ElementType::USHORT3:
		case ElementType::USHORT4:
			return GL_UNSIGNED_SHORT;
		case ElementType::INT:
			return GL_INT;
		case ElementType::UINT:
			return GL_UNSIGNED_INT;
		case ElementType::FLOAT:
		case ElementType::FLOAT2:
		case ElementType::FLOAT3:
		case ElementType::FLOAT4:
			return GL_FLOAT;
		case ElementType::HALF:
		case ElementType::HALF2:
		case ElementType::HALF3:
		case ElementType::HALF4:
			// on ES2 we should never end-up here
			return GL_HALF_FLOAT;
		}
	}

	uint32_t GetGLStencilOperation(StencilOperation op)
	{
		switch (op)
		{
		case StencilOperation::KEEP: return GL_KEEP;
		case StencilOperation::ZERO: return GL_ZERO;
		case StencilOperation::REPLACE: return GL_REPLACE;
		case StencilOperation::INCR: return GL_INCR;
		case StencilOperation::INCR_WRAP: return GL_INCR_WRAP;
		case StencilOperation::DECR: return GL_DECR;
		case StencilOperation::DECR_WRAP: return GL_DECR_WRAP;
		case StencilOperation::INVERT: return GL_INVERT;
		default: return GL_KEEP;
		}
	}

	uint32_t GetGLBlendFunctionMode(BlendFunction func)
	{
		switch (func)
		{
		case BlendFunction::ZERO: return GL_ZERO;
		case BlendFunction::ONE: return GL_ONE;
		case BlendFunction::SRC_COLOR: return GL_SRC_COLOR;
		case BlendFunction::ONE_MINUS_SRC_COLOR: return GL_ONE_MINUS_SRC_COLOR;
		case BlendFunction::DST_COLOR: return GL_DST_COLOR;
		case BlendFunction::ONE_MINUS_DST_COLOR: return GL_ONE_MINUS_DST_COLOR;
		case BlendFunction::SRC_ALPHA: return GL_SRC_ALPHA;
		case BlendFunction::ONE_MINUS_SRC_ALPHA: return GL_ONE_MINUS_SRC_ALPHA;
		case BlendFunction::DST_ALPHA: return GL_DST_ALPHA;
		case BlendFunction::ONE_MINUS_DST_ALPHA: return GL_ONE_MINUS_DST_ALPHA;
		case BlendFunction::SRC_ALPHA_SATURATE: return GL_SRC_ALPHA_SATURATE;
		default: return GL_ONE;
		}
	}

	uint32_t GetGLBlendEquationMode(BlendEquation eq)
	{
		switch (eq)
		{
		case BlendEquation::ADD: return GL_FUNC_ADD;
		case BlendEquation::SUBTRACT: return GL_FUNC_SUBTRACT;
		case BlendEquation::REVERSE_SUBTRACT: return GL_FUNC_REVERSE_SUBTRACT;
		case BlendEquation::MIN: return GL_MIN;
		case BlendEquation::MAX: return GL_MAX;
		default: return GL_FUNC_ADD;
		}
	}

	uint32_t GetGLCullingMode(CullingMode mode)
	{
		switch (mode)
		{
		case CullingMode::NONE: return 0;
		case CullingMode::FRONT: return GL_FRONT;
		case CullingMode::BACK: return GL_BACK;
		case CullingMode::FRONT_AND_BACK: return GL_FRONT_AND_BACK;
		default: return GL_BACK;
		}
	}
}
