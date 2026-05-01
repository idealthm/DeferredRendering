#pragma once
#include <vector>

#include "Common/Core.h"

namespace RHI
{
	enum class Format : uint8_t {
		Unknown = 0,
		// 颜色格式
		R8,
		RG8,
		RGB8,
		RGBA8,
		SRGB8,
		SRGBA8,
		RGB16F,
		R11G11B10F,
		RG16F,
		RGBA16F,
		// 深度/模板格式
		Depth16,
		Depth24,
		Depth32F,
		Depth24Stencil8,
		Depth32FStencil8
	};

	enum class Sampler : uint8_t {
		Dim1D,
		Dim2D,
		Dim3D,
		DimCube,
		Dim2DArray
	};

	enum class SamplerMinFilter : uint8_t {
		Nearest,
		Linear,
		NearestMipmapNearest,
		LinearMipmapNearest,
		NearestMipmapLinear,
		LinearMipmapLinear
	};

	enum class SamplerMagFilter : uint8_t {
		Nearest,
		Linear,
	};

	enum class SamplerWrapMode : uint8_t {
		Repeat,
		MirroredRepeat,
		ClampToEdge,
		ClampToBorder,
		MirrorClampToEdge
	};

	enum class SamplerCompareMode : uint8_t {
		NONE = 0,
		COMPARE_TO_TEXTURE = 1
	};

	enum class SamplerCompareFunc : uint8_t {
		Never,
		Less,
		Equal,
		LessEqual,
		Greater,
		NotEqual,
		GreaterEqual,
		Always
	};

	// 帧缓冲附着点类型
	enum class AttachmentType : uint8_t {
		Color,
		Depth,
		Stencil,
		DepthStencil
	};

	// 帧缓冲加载/存储操作 (对应 Vulkan Attachment Load/Store Op)
	enum class LoadAction : uint8_t {
		Load,       // 保留显存原有内容
		Clear,      // 清除
		DontCare    // 不关心 (Vulkan 专用，OpenGL 可映射为 Load)
	};

	enum class StoreAction : uint8_t {
		Store,      // 保留结果到显存
		Discard     // 丢弃结果 (移动端/PC 性能优化)
	};

	struct TextureDesc {
		uint32_t	Width = 1;
		uint32_t	Height = 1;
		uint32_t	DepthOrLayers = 1;   // 3D纹理的深度 或 数组纹理的层数
		uint32_t	MipLevels = 1;
		Format  	Format = Format::RGBA8;
		Sampler		Target = Sampler::Dim2D;

		bool operator==(const TextureDesc& other) const
		{
			return Width == other.Width && Height == other.Height && DepthOrLayers == other.DepthOrLayers
				&& MipLevels == other.MipLevels && Format == other.Format && Target == other.Target;
		}
		bool operator!=(const TextureDesc& other) const { return !(*this == other); }
	};

	struct FramebufferAttachmentDesc {
		AttachmentType  Type = AttachmentType::Color;
		LoadAction      LoadAction = LoadAction::Clear;
		StoreAction     StoreAction = StoreAction::Store;
    
		// 如果是数组纹理/立方体，指定具体层级或面
		uint32_t            LayerIndex = 0;
		uint32_t            MipLevel = 0;
    
		// 清除值 (根据类型不同取不同分量)
		union {
			float           ClearColor[4];
			struct {
				float       ClearDepth;
				uint32_t    ClearStencil;
			};
		};
	};

	struct FramebufferDesc {
		uint32_t                                Width = 0;
		uint32_t                                Height = 0;
		std::vector<FramebufferAttachmentDesc> Attachments;
	};

	
struct SamplerParams { // NOLINT
    SamplerMagFilter filterMag      : 1;    //!< magnification filter (NEAREST)
    SamplerMinFilter filterMin      : 3;    //!< minification filter  (NEAREST)
    SamplerWrapMode wrapS           : 2;    //!< s-coordinate wrap mode (CLAMP_TO_EDGE)
    SamplerWrapMode wrapT           : 2;    //!< t-coordinate wrap mode (CLAMP_TO_EDGE)

    SamplerWrapMode wrapR           : 2;    //!< r-coordinate wrap mode (CLAMP_TO_EDGE)
    uint8_t anisotropyLog2          : 3;    //!< anisotropy level (0)
    SamplerCompareMode compareMode  : 1;    //!< sampler compare mode (NONE)
    uint8_t padding0                : 2;    //!< reserved. must be 0.

    SamplerCompareFunc compareFunc  : 3;    //!< sampler comparison function (LE)
    uint8_t padding1                : 5;    //!< reserved. must be 0.
    uint8_t padding2                : 8;    //!< reserved. must be 0.

    struct Hasher {
        size_t operator()(SamplerParams p) const noexcept {
            // we don't use std::hash<> here, so we don't have to include <functional>
            return *reinterpret_cast<uint32_t const*>(reinterpret_cast<char const*>(&p));
        }
    };

    struct EqualTo {
        bool operator()(SamplerParams lhs, SamplerParams rhs) const noexcept {
            auto* pLhs = reinterpret_cast<uint32_t const*>(reinterpret_cast<char const*>(&lhs));
            auto* pRhs = reinterpret_cast<uint32_t const*>(reinterpret_cast<char const*>(&rhs));
            return *pLhs == *pRhs;
        }
    };

    struct LessThan {
        bool operator()(SamplerParams lhs, SamplerParams rhs) const noexcept {
            auto* pLhs = reinterpret_cast<uint32_t const*>(reinterpret_cast<char const*>(&lhs));
            auto* pRhs = reinterpret_cast<uint32_t const*>(reinterpret_cast<char const*>(&rhs));
            return *pLhs == *pRhs;
        }
    };

private:
    friend inline bool operator == (SamplerParams lhs, SamplerParams rhs) noexcept {
        return SamplerParams::EqualTo{}(lhs, rhs);
    }
    friend inline bool operator != (SamplerParams lhs, SamplerParams rhs) noexcept {
        return  !SamplerParams::EqualTo{}(lhs, rhs);
    }
    friend inline bool operator < (SamplerParams lhs, SamplerParams rhs) noexcept {
        return SamplerParams::LessThan{}(lhs, rhs);
    }
};
static_assert(sizeof(SamplerParams) == 4);
}

namespace RHI_Internal { // 仅用于内部实现文件，不暴露给上层
	using namespace RHI;
	uint32_t    GetGLFormat(Format fmt);
	uint32_t    GetGLInternalFormat(Format fmt, bool sRGB);
	uint32_t    GetGLType(Format fmt);
	uint32_t    GetGLWrapMode(SamplerWrapMode mode);
	uint32_t    GetGLFilter(SamplerMinFilter filter);
	uint32_t    GetGLFilter(SamplerMagFilter filter);
	uint32_t    GetGLCompareFunc(SamplerCompareFunc func);
	uint32_t    GetGLTextureTarget(Sampler dim);
}
