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

template<> struct EnableIntegerOperators<AttachmentPoint> : public std::true_type {};
template<> struct EnableBitMaskOperators<AttachmentPoint> : public std::true_type {};


class RenderTarget
{
public:
	class Builder
	{
	public:
		Builder& texture(AttachmentPoint attachment, const Ref<Texture>& texture);
		Builder& mipLevel(AttachmentPoint attachment, uint8_t level);
		Builder& face(AttachmentPoint attachment, RHI::TextureCubemapFace face);
		Builder& layer(AttachmentPoint attachment, uint8_t layer);
		Builder& layerCount(AttachmentPoint attachment, uint8_t layerCount, uint8_t baseLayer = 0);
		
		Ref<RenderTarget> Build();

		RenderTargetDesc m_Desc{};
	};

	RenderTarget(Builder& builder);
	virtual ~RenderTarget();
	Handle<RHI::HwRenderTarget> GetHandle() const {return m_Handle;}

private:
	Handle<RHI::HwRenderTarget> m_Handle;

	RenderTargetDesc m_Desc;

	RHI::TargetBufferFlags mAttachmentMask = {};
	RHI::TargetBufferFlags mSampleableAttachmentsMask = {};
};
