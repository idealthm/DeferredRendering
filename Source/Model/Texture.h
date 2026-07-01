#pragma once

#include <string>
#include <common/Core.h>

#include "Common/Handle.h"
#include "RHI/DriverEnums.h"

namespace RHI
{
	struct HwTexture;
}

enum class EFBTextureFormat;

namespace TextureFactory
{
	inline RHI::TextureDesc CreateShadowMap(uint32_t size)
	{
		RHI::TextureDesc result;
		result.Width = size;
		result.Height = size;
		result.DepthOrLayers = 1;
		result.LevelCount = 1;
		result.Format = RHI::Format::Depth24Stencil8;
		result.Target = RHI::SamplerType::SAMPLER_2D;
		return result;
	}

	inline RHI::TextureDesc CreateFinalColor(uint32_t w, uint32_t h)
	{
		RHI::TextureDesc result;
		result.Width = w;
		result.Height = h;
		result.DepthOrLayers = 1;
		result.LevelCount = 1;
		result.Format = RHI::Format::SRGBA8;
		result.Target = RHI::SamplerType::SAMPLER_2D;
		return result;
	}

	inline RHI::TextureDesc CreateGBuffer(uint32_t w, uint32_t h, RHI::Format format)
	{
		RHI::TextureDesc result;
		result.Width = w;
		result.Height = h;
		result.DepthOrLayers = 1;
		result.LevelCount = 1;
		result.Format = format;
		result.Target = RHI::SamplerType::SAMPLER_2D;
		result.Usage = RHI::TextureUsage::COLOR_ATTACHMENT | RHI::TextureUsage::SAMPLEABLE;
		return result;
	}

	inline RHI::TextureDesc CreateSkybox(uint32_t w, uint32_t h)
	{
		RHI::TextureDesc result;
		result.Width = w;
		result.Height = h;
		result.DepthOrLayers = 1;
		result.LevelCount = 1;
		result.Format = RHI::Format::RGBA16F;
		result.Target = RHI::SamplerType::SAMPLER_2D;
		return result;
	}

	inline RHI::TextureDesc CreateHDRBuffer(uint32_t w, uint32_t h)
	{
		RHI::TextureDesc result;
		result.Width = w;
		result.Height = h;
		result.DepthOrLayers = 1;
		result.LevelCount = 1;
		result.Format = RHI::Format::RGBA16F;
		result.Target = RHI::SamplerType::SAMPLER_2D;
		return result;
	}

	inline RHI::TextureDesc CreateDepth(uint32_t width, uint32_t height)
	{
		RHI::TextureDesc result;
		result.Width = width;
		result.Height = height;
		result.DepthOrLayers = 1;
		result.LevelCount = 1;
		result.Format = RHI::Format::Depth24Stencil8;
		result.Target = RHI::SamplerType::SAMPLER_2D;
		result.Usage = RHI::TextureUsage::DEPTH_ATTACHMENT | RHI::TextureUsage::SAMPLEABLE;
		return result;
	}
}

class Texture
{
public:
	class Builder
	{
	public:
		Builder& SetWidth(uint32_t w)          { m_Desc.Width = w; return *this; }
		Builder& SetHeight(uint32_t h)         { m_Desc.Height = h; return *this; }
		Builder& SetFormat(RHI::Format f)      { m_Desc.Format = f; return *this; }
		Builder& SetTarget(RHI::SamplerType t) { m_Desc.Target = t; return *this; }
		Builder& SetUsage(RHI::TextureUsage u) { m_Desc.Usage = u; return *this; }
		Builder& SetMipLevels(uint32_t l)      { m_Desc.LevelCount = l; return *this; }
		Builder& SetDepthOrLayers(uint32_t d)  { m_Desc.DepthOrLayers = d; return *this; }

		Ref<Texture> Build() { return CreateRef<Texture>(m_Desc); }

	private:
		RHI::TextureDesc m_Desc{};
	};

	struct LodRange {
		// 0,0 means lod-range unset (all levels are available)
		uint8_t first = 0;  // first lod
		uint8_t last = 0;   // 1 past last lod
		bool empty() const noexcept { return first == last; }
		size_t size() const noexcept { return last - first; }
	};

	Texture(const RHI::TextureDesc& desc);
	virtual ~Texture();

	bool IsValid() const { return !!m_Handle; }

	Handle<RHI::HwTexture> GetHandle() const { return m_Handle; }

	const RHI::TextureDesc& GetDesc() const { return m_Desc; }

	uint32_t GetSizeX(int32_t level) const { ASSERT(level < m_Desc.LevelCount);return m_Desc.Width >> level; }
	uint32_t GetSizeY(int32_t level) const { ASSERT(level < m_Desc.LevelCount);return m_Desc.Height >> level; }
	uint32_t GetSizeZ(int32_t level) const { ASSERT(level < m_Desc.LevelCount);return m_Desc.DepthOrLayers >> level; }
	RHI::Format GetFormat() const { return m_Desc.Format; }
	RHI::TextureUsage GetUsage() const { return m_Desc.Usage; }
	RHI::SamplerType GetTarget() const { return m_Desc.Target; }

	void UpdateLodRange(uint8_t baseLevel, uint8_t levelCount);
	void GenerateMipmaps();

	bool TextureHandleCanMutate() const;

	bool hasAllLods(LodRange const range) const noexcept {
		return range.first == 0 && range.last == m_Desc.LevelCount;
	}

	Handle<RHI::HwTexture> GetHandleForSampling() const;
	void setHandleForSampling(Handle<RHI::HwTexture> handle) const;

protected:
	Handle<RHI::HwTexture> m_Handle;
	mutable Handle<RHI::HwTexture> m_HandleForSampling;
	LodRange m_LodRange;
	mutable LodRange m_ActiveLodRange;
	RHI::TextureDesc  m_Desc;
};

template <typename T>
void CreateResource(Ref<T>& tex, const RHI::TextureDesc& desc)
{
	if (!tex || tex->GetDesc() != desc)
		tex = CreateRef<T>(desc);
}

