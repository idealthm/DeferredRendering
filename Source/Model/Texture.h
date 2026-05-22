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
		result.MipLevels = 1;
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
		result.MipLevels = 1;
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
		result.MipLevels = 1;
		result.Format = format;
		result.Target = RHI::SamplerType::SAMPLER_2D;
		return result;
	}

	inline RHI::TextureDesc CreateSkybox(uint32_t w, uint32_t h)
	{
		RHI::TextureDesc result;
		result.Width = w;
		result.Height = h;
		result.DepthOrLayers = 1;
		result.MipLevels = 1;
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
		result.MipLevels = 1;
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
		result.Format = RHI::Format::Depth24Stencil8;
		result.Target = RHI::SamplerType::SAMPLER_2D;
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
		Builder& SetMipLevels(uint32_t l)      { m_Desc.MipLevels = l; return *this; }
		Builder& SetDepthOrLayers(uint32_t d)  { m_Desc.DepthOrLayers = d; return *this; }

		Ref<Texture> Build() { return CreateRef<Texture>(m_Desc); }

	private:
		RHI::TextureDesc m_Desc{};
	};

	Texture(const RHI::TextureDesc& desc);
	virtual ~Texture();

	bool IsValid() const { return !!m_HWTexture; }

	Handle<RHI::HwTexture> GetHandle() const { return m_HWTexture; }

	const RHI::TextureDesc& GetDesc() const { return m_Desc; }

	uint32_t GetSizeX(int32_t level) const { ASSERT(level < m_Desc.DepthOrLayers);return m_Desc.Width >> level; }
	uint32_t GetSizeY(int32_t level) const { ASSERT(level < m_Desc.DepthOrLayers);return m_Desc.Height >> level; }
	uint32_t GetSizeZ(int32_t level) const { ASSERT(level < m_Desc.DepthOrLayers);return m_Desc.DepthOrLayers >> level; }
	RHI::Format GetFormat() const { return m_Desc.Format; }
	RHI::TextureUsage GetUsage() const { return m_Desc.Usage; }
	RHI::SamplerType GetTarget() const { return m_Desc.Target; }

protected:
	Handle<RHI::HwTexture> m_HWTexture;
	RHI::TextureDesc  m_Desc;
};

struct DefaultTextures
{
	Ref<Texture> White;
	Ref<Texture> Black;
	Ref<Texture> Normal;
	Ref<Texture> Gray;
};

template <typename T>
void CreateResource(Ref<T>& tex, const RHI::TextureDesc& desc)
{
	if (!tex || tex->GetDesc() != desc)
		tex = CreateRef<T>(desc);
}

extern DefaultTextures GDefaultTextures;
