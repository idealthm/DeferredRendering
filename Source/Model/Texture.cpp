#include "Texture.h"

#include <iostream>
#include <string>

#include <glad/glad.h>

#include "Engine.h"
#include "stb_images/stb_image.h"

DefaultTextures GDefaultTextures;

// --- Texture ---

Texture::Texture(const RHI::TextureDesc& desc)
{
	m_Desc = desc;
	m_Handle = gEngine->GetDriver().CreateTexture(desc.Target, desc.LevelCount, desc.Format, 1, desc.Width, desc.Height, desc.DepthOrLayers, desc.Usage);
	m_HandleForSampling = m_Handle;
}

Texture::~Texture()
{
	gEngine->GetDriver().DestroyTexture(m_Handle);
}

void Texture::UpdateLodRange(uint8_t baseLevel, uint8_t levelCount)
{
	if (any(m_Desc.Usage & RHI::TextureUsage::SAMPLEABLE) && m_Desc.LevelCount > 1) {
		auto& range = m_LodRange;
		uint8_t const last = int8_t(baseLevel + levelCount);
		if (range.first > baseLevel || range.last < last) {
			if (range.empty()) {
				range = { baseLevel, last };
			} else {
				range.first = std::min(range.first, baseLevel);
				range.last = std::max(range.last, last);
			}
			// We defer the creation of the texture view to getHwHandleForSampling() because it
			// is a common case that by then, the view won't be needed. Creating the first view on a
			// texture has a backend cost.
		}
	}
}

void Texture::GenerateMipmaps()
{
	ASSERT(m_Desc.Target != RHI::SamplerType::SAMPLER_3D);

	if (m_Desc.LevelCount < 2 || (m_Desc.Width == 1 && m_Desc.Height == 1)) {
		return;
	}

	gEngine->GetDriver().generateMipmap(m_Handle);
	// this method shouldn't have been const
	UpdateLodRange(0, m_Desc.LevelCount);
}

bool Texture::TextureHandleCanMutate() const
{
	return any(m_Desc.Usage & RHI::TextureUsage::SAMPLEABLE) && m_Desc.LevelCount > 1;
}

Handle<RHI::HwTexture> Texture::GetHandleForSampling() const
{
	auto const& range = m_LodRange;
	auto& activeRange = m_ActiveLodRange;
	bool const lodRangeChanged = activeRange.first != range.first || activeRange.last != range.last;
	if (UTILS_UNLIKELY(lodRangeChanged)) {
		activeRange = range;
		if (range.empty() || hasAllLods(range)) {
			setHandleForSampling(m_Handle);
		} else {
			setHandleForSampling(gEngine->GetDriver().CreateTextureView(
				m_Handle, range.first, range.size()));
		}
	}
	return m_HandleForSampling;
}

void Texture::setHandleForSampling(Handle<RHI::HwTexture> handle) const
{
	if (m_HandleForSampling && m_HandleForSampling != m_Handle) {
		gEngine->GetDriver().DestroyTexture(m_HandleForSampling);
	}
	m_HandleForSampling = handle;
}

