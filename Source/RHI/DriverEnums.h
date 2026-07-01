#pragma once
#include <array>
#include <vector>
#include <glm/glm.hpp>
#include <Common/Utils/BitmaskEnum.h>

#include "Common/Utils/PoolAllocator.h"

static constexpr size_t MAX_VERTEX_ATTRIBUTE_COUNT  = 16;
static constexpr size_t MAX_SAMPLER_COUNT           = 62;   // Maximum needed at feature level 3.
static constexpr size_t MAX_VERTEX_BUFFER_COUNT     = 16;   // Max number of bound buffer objects.
static constexpr size_t MAX_SSBO_COUNT              = 4;    // This is guaranteed by OpenGL ES.
static constexpr size_t MAX_DESCRIPTOR_SET_COUNT    = 4;    // This is guaranteed by Vulkan.
static constexpr size_t MAX_DESCRIPTOR_COUNT        = 64;   // per set

static constexpr uint8_t MIN_SUPPORTED_RENDER_TARGET_COUNT = 4u;

static constexpr uint8_t MAX_SUPPORTED_RENDER_TARGET_COUNT = 8u;

using descriptor_set_t = uint8_t;
using descriptor_binding_t = uint8_t;

namespace RHI
{
	enum class SamplerType : uint8_t {
		SAMPLER_2D,
		SAMPLER_2D_ARRAY,
		SAMPLER_CUBEMAP,
		SAMPLER_3D,
		SAMPLER_CUBEMAP_ARRAY,
	};

	//! Texture sampler format
	enum class SamplerFormat : uint8_t {
		INT = 0,        //!< signed integer sampler
		UINT = 1,       //!< unsigned integer sampler
		FLOAT = 2,      //!< float sampler
		SHADOW = 3      //!< shadow sampler (PCF)
	};

	enum class PrimitiveType : uint8_t {
		// don't change the enums values (made to match GL)
		POINTS         = 0,    //!< points
		LINES          = 1,    //!< lines
		LINE_STRIP     = 3,    //!< line strip
		TRIANGLES      = 4,    //!< triangles
		TRIANGLE_STRIP = 5     //!< triangle strip
	};

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

	enum class TextureUsage : uint16_t {
		NONE                = 0x0000,
		COLOR_ATTACHMENT    = 0x0001,            //!< Texture can be used as a color attachment
		DEPTH_ATTACHMENT    = 0x0002,            //!< Texture can be used as a depth attachment
		STENCIL_ATTACHMENT  = 0x0004,            //!< Texture can be used as a stencil attachment
		UPLOADABLE          = 0x0008,            //!< Data can be uploaded into this texture (default)
		SAMPLEABLE          = 0x0010,            //!< Texture can be sampled (default)
		SUBPASS_INPUT       = 0x0020,            //!< Texture can be used as a subpass input
		BLIT_SRC            = 0x0040,            //!< Texture can be used the source of a blit()
		BLIT_DST            = 0x0080,            //!< Texture can be used the destination of a blit()
		PROTECTED           = 0x0100,            //!< Texture can be used for protected content
		DEFAULT             = UPLOADABLE | SAMPLEABLE,   //!< Default texture usage
		ALL_ATTACHMENTS     = COLOR_ATTACHMENT | DEPTH_ATTACHMENT | STENCIL_ATTACHMENT | SUBPASS_INPUT,   //!< Mask of all attachments
	};

	enum class PixelDataFormat : uint8_t {
		R,                  //!< One Red channel, float
		R_INTEGER,          //!< One Red channel, integer
		RG,                 //!< Two Red and Green channels, float
		RG_INTEGER,         //!< Two Red and Green channels, integer
		RGB,                //!< Three Red, Green and Blue channels, float
		RGB_INTEGER,        //!< Three Red, Green and Blue channels, integer
		RGBA,               //!< Four Red, Green, Blue and Alpha channels, float
		RGBA_INTEGER,       //!< Four Red, Green, Blue and Alpha channels, integer
		UNUSED,             // used to be rgbm
		DEPTH_COMPONENT,    //!< Depth, 16-bit or 24-bits usually
		DEPTH_STENCIL,      //!< Two Depth (24-bits) + Stencil (8-bits) channels
		ALPHA               //! One Alpha channel, float
	};

	enum class CullingMode : uint8_t {
		NONE,               //!< No culling, front and back faces are visible
		FRONT,              //!< Front face culling, only back faces are visible
		BACK,               //!< Back face culling, only front faces are visible
		FRONT_AND_BACK      //!< Front and Back, geometry is not visible
	};

	enum class BlendEquation : uint8_t {
		ADD,                    //!< the fragment is added to the color buffer
		SUBTRACT,               //!< the fragment is subtracted from the color buffer
		REVERSE_SUBTRACT,       //!< the color buffer is subtracted from the fragment
		MIN,                    //!< the min between the fragment and color buffer
		MAX                     //!< the max between the fragment and color buffer
	};

	enum class BlendFunction : uint8_t {
		ZERO,                   //!< f(src, dst) = 0
		ONE,                    //!< f(src, dst) = 1
		SRC_COLOR,              //!< f(src, dst) = src
		ONE_MINUS_SRC_COLOR,    //!< f(src, dst) = 1-src
		DST_COLOR,              //!< f(src, dst) = dst
		ONE_MINUS_DST_COLOR,    //!< f(src, dst) = 1-dst
		SRC_ALPHA,              //!< f(src, dst) = src.a
		ONE_MINUS_SRC_ALPHA,    //!< f(src, dst) = 1-src.a
		DST_ALPHA,              //!< f(src, dst) = dst.a
		ONE_MINUS_DST_ALPHA,    //!< f(src, dst) = 1-dst.a
		SRC_ALPHA_SATURATE      //!< f(src, dst) = (1,1,1) * min(src.a, 1 - dst.a), 1
	};

	//! Pixel Data Type
	enum class PixelDataType : uint8_t {
		UBYTE,                //!< unsigned byte
		BYTE,                 //!< signed byte
		USHORT,               //!< unsigned short (16-bit)
		SHORT,                //!< signed short (16-bit)
		UINT,                 //!< unsigned int (32-bit)
		INT,                  //!< signed int (32-bit)
		HALF,                 //!< half-float (16-bit float)
		FLOAT,                //!< float (32-bits float)
		COMPRESSED,           //!< compressed pixels, @see CompressedPixelDataType
		UINT_10F_11F_11F_REV, //!< three low precision floating-point numbers
		USHORT_565,           //!< unsigned int (16-bit), encodes 3 RGB channels
		UINT_2_10_10_10_REV,  //!< unsigned normalized 10 bits RGB, 2 bits alpha
	};

	enum class TextureCubemapFace : uint8_t {
		// don't change the enums values
		POSITIVE_X = 0, //!< +x face
		NEGATIVE_X = 1, //!< -x face
		POSITIVE_Y = 2, //!< +y face
		NEGATIVE_Y = 3, //!< -y face
		POSITIVE_Z = 4, //!< +z face
		NEGATIVE_Z = 5, //!< -z face
	};

	struct TextureDesc {
		uint32_t	Width = 1;
		uint32_t	Height = 1;
		uint8_t		DepthOrLayers = 1;   // 3D纹理的深度 或 数组纹理的层数
		uint8_t		LevelCount = 1;
		Format  	Format = Format::RGBA8;
		SamplerType		Target = SamplerType::SAMPLER_2D;
		TextureUsage Usage = TextureUsage::DEFAULT;

		bool operator==(const TextureDesc& other) const
		{
			return Width == other.Width && Height == other.Height && DepthOrLayers == other.DepthOrLayers
				&& LevelCount == other.LevelCount && Format == other.Format && Target == other.Target;
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
            return *pLhs < *pRhs;
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

	
enum class ElementType : uint8_t {
    BYTE,
    BYTE2,
    BYTE3,
    BYTE4,
    UBYTE,
    UBYTE2,
    UBYTE3,
    UBYTE4,
    SHORT,
    SHORT2,
    SHORT3,
    SHORT4,
    USHORT,
    USHORT2,
    USHORT3,
    USHORT4,
    INT,
    UINT,
    FLOAT,
    FLOAT2,
    FLOAT3,
    FLOAT4,
    HALF,
    HALF2,
    HALF3,
    HALF4,
};

enum class TargetBufferFlags : uint32_t {
    NONE = 0x0u,                            //!< No buffer selected.
    COLOR0 = 0x00000001u,                   //!< Color buffer selected.
    COLOR1 = 0x00000002u,                   //!< Color buffer selected.
    COLOR2 = 0x00000004u,                   //!< Color buffer selected.
    COLOR3 = 0x00000008u,                   //!< Color buffer selected.
    COLOR4 = 0x00000010u,                   //!< Color buffer selected.
    COLOR5 = 0x00000020u,                   //!< Color buffer selected.
    COLOR6 = 0x00000040u,                   //!< Color buffer selected.
    COLOR7 = 0x00000080u,                   //!< Color buffer selected.

    COLOR = COLOR0,                         //!< \deprecated
    COLOR_ALL = COLOR0 | COLOR1 | COLOR2 | COLOR3 | COLOR4 | COLOR5 | COLOR6 | COLOR7,
    DEPTH   = 0x10000000u,                  //!< Depth buffer selected.
    STENCIL = 0x20000000u,                  //!< Stencil buffer selected.
    DEPTH_AND_STENCIL = DEPTH | STENCIL,    //!< depth and stencil buffer selected.
    ALL = COLOR_ALL | DEPTH | STENCIL       //!< Color, depth and stencil buffer selected.
};

struct Attribute {
    //! attribute is normalized (remapped between 0 and 1)
    static constexpr uint8_t FLAG_NORMALIZED     = 0x1;
    //! attribute is an integer
    static constexpr uint8_t FLAG_INTEGER_TARGET = 0x2;
    static constexpr uint8_t BUFFER_UNUSED = 0xFF;
    uint32_t offset = 0;                    //!< attribute offset in bytes
    uint8_t stride = 0;                     //!< attribute stride in bytes
    uint8_t buffer = BUFFER_UNUSED;         //!< attribute buffer index
    ElementType type = ElementType::BYTE;   //!< attribute element type
    uint8_t flags = 0x0;                    //!< attribute flags
};

enum class DescriptorType : uint8_t {
    UNIFORM_BUFFER,
    SHADER_STORAGE_BUFFER,
    SAMPLER,
    INPUT_ATTACHMENT,
    SAMPLER_EXTERNAL
};

enum class DescriptorFlags : uint8_t {
    NONE = 0x00,
    DYNAMIC_OFFSET = 0x01
};

enum class BufferObjectBinding : uint8_t {
    VERTEX,
    UNIFORM,
    SHADER_STORAGE
};

enum class BufferUsage : uint8_t {
    STATIC,      //!< content modified once, used many times
    DYNAMIC,     //!< content modified frequently, used many times
};

enum class ShaderStageFlags : uint8_t {
    NONE        =    0,
    VERTEX      =    0x1,
    FRAGMENT    =    0x2,
    COMPUTE     =    0x4,
    ALL_SHADER_STAGE_FLAGS = VERTEX | FRAGMENT | COMPUTE
};

struct DescriptorSetLayoutBinding {
    DescriptorType type;
    ShaderStageFlags stageFlags;
    uint8_t binding;
    DescriptorFlags flags = DescriptorFlags::NONE;
    uint16_t count = 0;

    friend inline bool operator==(
            DescriptorSetLayoutBinding const& lhs,
            DescriptorSetLayoutBinding const& rhs) noexcept {
        return lhs.type == rhs.type &&
               lhs.flags == rhs.flags &&
               lhs.count == rhs.count &&
               lhs.stageFlags == rhs.stageFlags;
    }
};

	
//! Raster state descriptor
struct RasterState {

    RasterState() noexcept { // NOLINT
        static_assert(sizeof(RasterState) == sizeof(uint32_t),
                "RasterState size not what was intended");
        culling = CullingMode::BACK;
        blendEquationRGB = BlendEquation::ADD;
        blendEquationAlpha = BlendEquation::ADD;
        blendFunctionSrcRGB = BlendFunction::ONE;
        blendFunctionSrcAlpha = BlendFunction::ONE;
        blendFunctionDstRGB = BlendFunction::ZERO;
        blendFunctionDstAlpha = BlendFunction::ZERO;
        colorWrite = true;
        depthWrite = true;
        depthFunc = SamplerCompareFunc::Less;
    }

    bool operator == (RasterState rhs) const noexcept { return u == rhs.u; }
    bool operator != (RasterState rhs) const noexcept { return u != rhs.u; }

    void disableBlending() noexcept {
        blendEquationRGB = BlendEquation::ADD;
        blendEquationAlpha = BlendEquation::ADD;
        blendFunctionSrcRGB = BlendFunction::ONE;
        blendFunctionSrcAlpha = BlendFunction::ONE;
        blendFunctionDstRGB = BlendFunction::ZERO;
        blendFunctionDstAlpha = BlendFunction::ZERO;
    }

    // note: clang reduces this entire function to a simple load/mask/compare
    bool hasBlending() const noexcept {
        // This is used to decide if blending needs to be enabled in the h/w
        return !(blendEquationRGB == BlendEquation::ADD &&
                 blendEquationAlpha == BlendEquation::ADD &&
                 blendFunctionSrcRGB == BlendFunction::ONE &&
                 blendFunctionSrcAlpha == BlendFunction::ONE &&
                 blendFunctionDstRGB == BlendFunction::ZERO &&
                 blendFunctionDstAlpha == BlendFunction::ZERO);
    }

    union {
        struct {
            //! culling mode
            CullingMode culling                         : 2;        //  2

            //! blend equation for the red, green and blue components
            BlendEquation blendEquationRGB              : 3;        //  5
            //! blend equation for the alpha component
            BlendEquation blendEquationAlpha            : 3;        //  8

            //! blending function for the source color
            BlendFunction blendFunctionSrcRGB           : 4;        // 12
            //! blending function for the source alpha
            BlendFunction blendFunctionSrcAlpha         : 4;        // 16
            //! blending function for the destination color
            BlendFunction blendFunctionDstRGB           : 4;        // 20
            //! blending function for the destination alpha
            BlendFunction blendFunctionDstAlpha         : 4;        // 24

            //! Whether depth-buffer writes are enabled
            bool depthWrite                             : 1;        // 25
            //! Depth test function
            SamplerCompareFunc depthFunc                : 3;        // 28

            //! Whether color-buffer writes are enabled
            bool colorWrite                             : 1;        // 29

            //! use alpha-channel as coverage mask for anti-aliasing
            bool alphaToCoverage                        : 1;        // 30

            //! whether front face winding direction must be inverted
            bool inverseFrontFaces                      : 1;        // 31

            //! padding, must be 0
            bool depthClamp                             : 1;        // 32
        };
        uint32_t u = 0;
    };
};

struct PolygonOffset {
	float slope = 0;        // factor in GL-speak
	float constant = 0;     // units in GL-speak
};
	
//! stencil faces
enum class StencilFace : uint8_t {
	FRONT               = 0x1,              //!< Update stencil state for front-facing polygons.
	BACK                = 0x2,              //!< Update stencil state for back-facing polygons.
	FRONT_AND_BACK      = FRONT | BACK,     //!< Update stencil state for all polygons.
};

enum class StencilOperation : uint8_t {
	KEEP,                   //!< Keeps the current value.
	ZERO,                   //!< Sets the value to 0.
	REPLACE,                //!< Sets the value to the stencil reference value.
	INCR,                   //!< Increments the current value. Clamps to the maximum representable unsigned value.
	INCR_WRAP,              //!< Increments the current value. Wraps value to zero when incrementing the maximum representable unsigned value.
	DECR,                   //!< Decrements the current value. Clamps to 0.
	DECR_WRAP,              //!< Decrements the current value. Wraps value to the maximum representable unsigned value when decrementing a value of zero.
	INVERT,                 //!< Bitwise inverts the current value.
};
	
struct StencilState {

    struct StencilOperations {
        //! Stencil test function
        SamplerCompareFunc stencilFunc                  : 3;                    // 3

        //! Stencil operation when stencil test fails
        StencilOperation stencilOpStencilFail           : 3;                    // 6

        uint8_t padding0                                : 2;                    // 8

        //! Stencil operation when stencil test passes but depth test fails
        StencilOperation stencilOpDepthFail             : 3;                    // 11

        //! Stencil operation when both stencil and depth test pass
        StencilOperation stencilOpDepthStencilPass      : 3;                    // 14

        uint8_t padding1                                : 2;                    // 16

        //! Reference value for stencil comparison tests and updates
        uint8_t ref;                                                            // 24

        //! Masks the bits of the stencil values participating in the stencil comparison test.
        uint8_t readMask;                                                       // 32

        //! Masks the bits of the stencil values updated by the stencil test.
        uint8_t writeMask;                                                      // 40
    };

    //! Stencil operations for front-facing polygons
    StencilOperations front = {
            SamplerCompareFunc::Always,
            StencilOperation::KEEP,
            0,
            StencilOperation::KEEP,
            StencilOperation::KEEP,
            0,
            0,
            0xff,
            0xff };

    //! Stencil operations for back-facing polygons
    StencilOperations back  = {
            SamplerCompareFunc::Always,
            StencilOperation::KEEP,
            0,
            StencilOperation::KEEP,
            StencilOperation::KEEP,
            0,
            0,
            0xff,
            0xff };

    //! Whether stencil-buffer writes are enabled
    bool stencilWrite = false;

    uint8_t padding = 0;
};


	
struct RenderPassFlags {
    /**
     * bitmask indicating which buffers to clear at the beginning of a render pass.
     * This implies discard.
     */
    TargetBufferFlags clear;

    /**
     * bitmask indicating which buffers to discard at the beginning of a render pass.
     * Discarded buffers have uninitialized content, they must be entirely drawn over or cleared.
     */
    TargetBufferFlags discardStart;

    /**
     * bitmask indicating which buffers to discard at the end of a render pass.
     * Discarded buffers' content becomes invalid, they must not be read from again.
     */
    TargetBufferFlags discardEnd;
};


struct Viewport {
	int32_t left;       //!< left coordinate in window space.
	int32_t bottom;     //!< bottom coordinate in window space.
	uint32_t width;     //!< width in pixels
	uint32_t height;    //!< height in pixels
	//! get the right coordinate in window space of the viewport
	int32_t right() const noexcept { return left + int32_t(width); }
	//! get the top coordinate in window space of the viewport
	int32_t top() const noexcept { return bottom + int32_t(height); }

	friend bool operator==(Viewport const& lhs, Viewport const& rhs) noexcept {
		// clang can do this branchless with xor/or
		return lhs.left == rhs.left && lhs.bottom == rhs.bottom &&
			   lhs.width == rhs.width && lhs.height == rhs.height;
	}

	friend bool operator!=(Viewport const& lhs, Viewport const& rhs) noexcept {
		// clang is being dumb and uses branches
		return bool(((lhs.left ^ rhs.left) | (lhs.bottom ^ rhs.bottom)) |
					((lhs.width ^ rhs.width) | (lhs.height ^ rhs.height)));
	}
};

/**
 * Specifies the mapping of the near and far clipping plane to window coordinates.
 */
struct DepthRange {
	float near = 0.0f;    //!< mapping of the near plane to window coordinates.
	float far = 1.0f;     //!< mapping of the far plane to window coordinates.
};
	
/**
 * Parameters of a render pass.
 */
struct RenderPassParams {
    RenderPassFlags flags{};    //!< operations performed on the buffers for this pass

    Viewport viewport{};        //!< viewport for this pass
    DepthRange depthRange{};    //!< depth range for this pass

    //! Color to use to clear the COLOR buffer. RenderPassFlags::clear must be set.
    glm::vec4 clearColor = {};

    //! Depth value to clear the depth buffer with
    double clearDepth = 0.0;

    //! Stencil value to clear the stencil buffer with
    uint32_t clearStencil = 0;

    /**
     * The subpass mask specifies which color attachments are designated for read-back in the second
     * subpass. If this is zero, the render pass has only one subpass. The least significant bit
     * specifies that the first color attachment in the render target is a subpass input.
     *
     * For now only 2 subpasses are supported, so only the lower 8 bits are used, one for each color
     * attachment (see MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT).
     */
    uint16_t subpassMask = 0;

    /**
     * This mask makes a promise to the backend about read-only usage of the depth attachment (bit
     * 0) and the stencil attachment (bit 1). Some backends need to know if writes are disabled in
     * order to allow sampling from the depth attachment.
     */
    uint16_t readOnlyDepthStencil = 0;

    static constexpr uint16_t READONLY_DEPTH = 1 << 0;
    static constexpr uint16_t READONLY_STENCIL = 1 << 1;
};

struct DescriptorSetLayout {
    std::vector<DescriptorSetLayoutBinding> bindings;
};

static constexpr bool isDepthFormat(Format format) noexcept {
    switch (format) {
    case Format::Depth16:
    case Format::Depth24:
    case Format::Depth32F:
    case Format::Depth24Stencil8:
    case Format::Depth32FStencil8:
        return true;
    default:
        return false;
    }
}

inline constexpr TargetBufferFlags getTargetBufferFlagsAt(size_t index) noexcept {
	if (index == 0u) return TargetBufferFlags::COLOR0;
	if (index == 1u) return TargetBufferFlags::COLOR1;
	if (index == 2u) return TargetBufferFlags::COLOR2;
	if (index == 3u) return TargetBufferFlags::COLOR3;
	if (index == 4u) return TargetBufferFlags::COLOR4;
	if (index == 5u) return TargetBufferFlags::COLOR5;
	if (index == 6u) return TargetBufferFlags::COLOR6;
	if (index == 7u) return TargetBufferFlags::COLOR7;
	if (index == 8u) return TargetBufferFlags::DEPTH;
	if (index == 9u) return TargetBufferFlags::STENCIL;
	return TargetBufferFlags::NONE;
}

using AttributeArray = std::array<Attribute, MAX_VERTEX_ATTRIBUTE_COUNT>;

}
template<> struct EnableBitMaskOperators<RHI::TextureUsage> : public std::true_type { };
template<> struct EnableIntegerOperators<RHI::TextureUsage> : public std::true_type { };

template<> struct EnableBitMaskOperators<RHI::ShaderStageFlags> : public std::true_type { };
template<> struct EnableIntegerOperators<RHI::ShaderStageFlags> : public std::true_type { };

template<> struct EnableBitMaskOperators<RHI::TargetBufferFlags> : public std::true_type { };
template<> struct EnableIntegerOperators<RHI::TargetBufferFlags> : public std::true_type { };

template<> struct EnableBitMaskOperators<RHI::TextureCubemapFace> : public std::true_type { };
template<> struct EnableIntegerOperators<RHI::TextureCubemapFace> : public std::true_type { };

