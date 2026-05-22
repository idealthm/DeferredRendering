#pragma once
#include <cstdint>

#include "RHI/DriverEnums.h"
#include "RHI/TargetBufferInfo.h"

class Texture;

namespace RHI
{
	enum class TextureCubemapFace : unsigned char;
}

enum class AttachmentPoint : uint8_t {
		COLOR0 = 0,          //!< identifies the 1st color attachment
		COLOR1 = 1,          //!< identifies the 2nd color attachment
		COLOR2 = 2,          //!< identifies the 3rd color attachment
		COLOR3 = 3,          //!< identifies the 4th color attachment
		COLOR4 = 4,          //!< identifies the 5th color attachment
		COLOR5 = 5,          //!< identifies the 6th color attachment
		COLOR6 = 6,          //!< identifies the 7th color attachment
		COLOR7 = 7,          //!< identifies the 8th color attachment
		DEPTH  = RHI::MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT,   //!< identifies the depth attachment
		COLOR  = COLOR0,     //!< identifies the 1st color attachment
};

struct Attachment {
	Ref<Texture> texture = nullptr;
	uint8_t mipLevel = 0;
	RHI::TextureCubemapFace face = RHI::TextureCubemapFace::POSITIVE_X;
	uint32_t layer = 0;
	// Indicates the number of layers used for multiview, starting from the `layer` (baseIndex).
	// This means `layer` + `layerCount` cannot exceed the number of depth for the attachment.
	uint16_t layerCount = 0;
};

struct RenderTargetDesc
{
	uint32_t w, h;
	Attachment mAttachments[RHI::MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT + 1] = {};
	uint8_t mSamples = 1;
	uint8_t mLayerCount = 1;
};


class RenderTarget
{
public:
	class Builder
	{
	public:
		Builder& SetWidth(uint32_t w)             { m_Desc.w = w; return *this; }
		Builder& SetHeight(uint32_t h)            { m_Desc.h = h; return *this; }
		Builder& SetSamples(uint8_t s)            { m_Desc.mSamples = s; return *this; }
		Builder& SetLayerCount(uint8_t l)         { m_Desc.mLayerCount = l; return *this; }
		Builder& SetColorAttachment(uint32_t index, const Attachment& a)
		{
			m_Desc.mAttachments[index] = a;
			return *this;
		}
		Builder& SetDepthAttachment(const Attachment& a)
		{
			m_Desc.mAttachments[(uint32_t)AttachmentPoint::DEPTH] = a;
			return *this;
		}
		Ref<RenderTarget> Build() { return CreateRef<RenderTarget>(m_Desc); }

	private:
		RenderTargetDesc m_Desc{};
	};

	RenderTarget(RenderTargetDesc& desc);
	virtual ~RenderTarget();

private:
	Handle<RHI::HwRenderTarget> m_Handle;

	RenderTargetDesc m_Desc;

	RHI::TargetBufferFlags mAttachmentMask = {};
	RHI::TargetBufferFlags mSampleableAttachmentsMask = {};
};
