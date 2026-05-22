#include "RenderTarget.h"

#include "Engine.h"
#include "Model/Texture.h"

RenderTarget::RenderTarget(RenderTargetDesc& desc)
	: m_Desc(desc)
{
	auto& color = desc.mAttachments[(uint32_t)AttachmentPoint::COLOR];
	auto& depth = desc.mAttachments[(uint32_t)AttachmentPoint::DEPTH];

	if (color.texture)
	{
		ASSERT(color.texture->GetUsage() & RHI::TextureUsage::COLOR_ATTACHMENT);
	}

	if (depth.texture)
	{
		ASSERT(depth.texture->GetUsage() & RHI::TextureUsage::DEPTH_ATTACHMENT);
	}

	uint32_t minWidth = std::numeric_limits<uint32_t>::max();
	uint32_t maxWidth = 0;
	uint32_t minHeight = std::numeric_limits<uint32_t>::max();
	uint32_t maxHeight = 0;
	uint32_t minLayerCount = std::numeric_limits<uint32_t>::max();
	uint32_t maxLayerCount = 0;
	for (auto const& attachment : desc.mAttachments) {
		if (attachment.texture) {
			const uint32_t w = attachment.texture->GetSizeX(attachment.mipLevel);
			const uint32_t h = attachment.texture->GetSizeY(attachment.mipLevel);
			const uint32_t d = attachment.texture->GetSizeZ(attachment.mipLevel);
			const uint32_t l = attachment.layerCount;
			if (l > 0) {
				ASSERT(attachment.texture->GetTarget() == RHI::SamplerType::SAMPLER_2D_ARRAY)
			}
			ASSERT(attachment.layer + l <= d);

			minWidth  = std::min(minWidth, w);
			minHeight = std::min(minHeight, h);
			minLayerCount = std::min(minLayerCount, l);
			maxWidth  = std::max(maxWidth, w);
			maxHeight = std::max(maxHeight, h);
			maxLayerCount = std::max(maxLayerCount, l);
		}
	}

	ASSERT(minWidth == maxWidth && minHeight == maxHeight && minLayerCount == maxLayerCount)

	RHI::MRT mrt{};
	RHI::TargetBufferInfo dinfo{};

	auto& driver = gEngine->GetDriver();

	auto setAttachment = [this, &driver, desc]
			(RHI::TargetBufferInfo& info, AttachmentPoint attachmentPoint) {
		Attachment const& attachment = desc.mAttachments[(size_t)attachmentPoint];
		auto t = attachment.texture;
		info.handle = t->GetHandle();
		info.level  = attachment.mipLevel;
		if (t->GetTarget() == RHI::SamplerType::SAMPLER_CUBEMAP) {
			info.layer = +attachment.face;
		} else {
			info.layer = attachment.layer;
		}
		// driver.UpdateLodRange(info.level);
	};

	for (size_t i = 0; i < RHI::MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT; i++) {
		Attachment const& attachment = desc.mAttachments[i];
		if (attachment.texture) {
			RHI::TargetBufferFlags const targetBufferBit = RHI::getTargetBufferFlagsAt(i);
			mAttachmentMask |= targetBufferBit;
			setAttachment(mrt[i], (AttachmentPoint)i);
			if (any(attachment.texture->GetUsage() & RHI::TextureUsage::SAMPLEABLE)) {
				mSampleableAttachmentsMask |= targetBufferBit;
			}
		}
	}

	Attachment const& depthAttachment = desc.mAttachments[(size_t)AttachmentPoint::DEPTH];
	if (depthAttachment.texture) {
		mAttachmentMask |= RHI::TargetBufferFlags::DEPTH;
		setAttachment(dinfo, AttachmentPoint::DEPTH);
		if (any(depthAttachment.texture->GetUsage() & RHI::TextureUsage::SAMPLEABLE)) {
			mSampleableAttachmentsMask |= RHI::TargetBufferFlags::DEPTH;
				}
	}

	m_Handle = driver.CreateRenderTarget(mAttachmentMask, desc.w, desc.h, desc.mSamples, desc.mLayerCount,
		mrt, dinfo, {});
}

RenderTarget::~RenderTarget()
{
	if (m_Handle)
		gEngine->GetDriver().DestroyRenderTarget(m_Handle);
}
