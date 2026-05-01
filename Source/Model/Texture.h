#pragma once

#include <string>
#include <common/Core.h>

#include "RHI/HWTexture.h"
#include "RHI/HWSampler.h"
#include "RHI/RHITypes.h"

enum class EFBTextureFormat;

namespace
{
	RHI::TextureDesc CreateShadowMap(uint32_t size)
	{
		RHI::TextureDesc result;
		result.Width = size;
		result.Height = size;
		result.DepthOrLayers = 1;
		result.MipLevels = 1;
		result.Format = RHI::Format::Depth24Stencil8;
		result.Target = RHI::Sampler::Dim2D;
		return result;
	}

	RHI::SamplerParams ShadowMapSampler()
	{
		RHI::SamplerParams params{};
		params.wrapS = RHI::SamplerWrapMode::ClampToEdge;
		params.wrapT = RHI::SamplerWrapMode::ClampToEdge;
		params.filterMin = RHI::SamplerMinFilter::Linear;
		params.filterMag = RHI::SamplerMagFilter::Linear;
		return params;
	}

	RHI::TextureDesc CreateFinalColor(uint32_t w, uint32_t h)
	{
		RHI::TextureDesc result;
		result.Width = w;
		result.Height = h;
		result.DepthOrLayers = 1;
		result.MipLevels = 1;
		result.Format = RHI::Format::SRGBA8;
		result.Target = RHI::Sampler::Dim2D;
		return result;
	}

	RHI::SamplerParams DefaultClampSampler()
	{
		RHI::SamplerParams params{};
		params.wrapS = RHI::SamplerWrapMode::ClampToEdge;
		params.wrapT = RHI::SamplerWrapMode::ClampToEdge;
		params.filterMin = RHI::SamplerMinFilter::Linear;
		params.filterMag = RHI::SamplerMagFilter::Linear;
		return params;
	}

	RHI::SamplerParams DefaultRepeatSampler()
	{
		RHI::SamplerParams params{};
		params.wrapS = RHI::SamplerWrapMode::Repeat;
		params.wrapT = RHI::SamplerWrapMode::Repeat;
		params.filterMin = RHI::SamplerMinFilter::Linear;
		params.filterMag = RHI::SamplerMagFilter::Linear;
		return params;
	}

	RHI::SamplerParams DefaultRepeatMipmapSampler()
	{
		RHI::SamplerParams params{};
		params.wrapS = RHI::SamplerWrapMode::Repeat;
		params.wrapT = RHI::SamplerWrapMode::Repeat;
		params.filterMin = RHI::SamplerMinFilter::LinearMipmapLinear;
		params.filterMag = RHI::SamplerMagFilter::Linear;
		return params;
	}

	RHI::TextureDesc CreateGBuffer(uint32_t w, uint32_t h, RHI::Format format)
	{
		RHI::TextureDesc result;
		result.Width = w;
		result.Height = h;
		result.DepthOrLayers = 1;
		result.MipLevels = 1;
		result.Format = format;
		result.Target = RHI::Sampler::Dim2D;
		return result;
	}

	RHI::TextureDesc CreateSkybox(uint32_t w, uint32_t h)
	{
		RHI::TextureDesc result;
		result.Width = w;
		result.Height = h;
		result.DepthOrLayers = 1;
		result.MipLevels = 1;
		result.Format = RHI::Format::RGBA16F;
		result.Target = RHI::Sampler::Dim2D;
		return result;
	}

	RHI::SamplerParams SkyboxSampler()
	{
		RHI::SamplerParams params{};
		params.wrapS = RHI::SamplerWrapMode::ClampToEdge;
		params.wrapT = RHI::SamplerWrapMode::ClampToEdge;
		params.wrapR = RHI::SamplerWrapMode::ClampToEdge;
		params.filterMin = RHI::SamplerMinFilter::Linear;
		params.filterMag = RHI::SamplerMagFilter::Linear;
		return params;
	}

	RHI::TextureDesc CreateHDRBuffer(uint32_t w, uint32_t h)
	{
		RHI::TextureDesc result;
		result.Width = w;
		result.Height = h;
		result.DepthOrLayers = 1;
		result.MipLevels = 1;
		result.Format = RHI::Format::RGBA16F;
		result.Target = RHI::Sampler::Dim2D;
		return result;
	}

	RHI::TextureDesc CreateDepth(uint32_t width, uint32_t height)
	{
		RHI::TextureDesc result;
		result.Width = width;
		result.Height = height;
		result.DepthOrLayers = 1;
		result.Format = RHI::Format::Depth24Stencil8;
		result.Target = RHI::Sampler::Dim2D;
		return result;
	}

	RHI::SamplerParams DepthSampler()
	{
		RHI::SamplerParams params{};
		params.wrapS = RHI::SamplerWrapMode::ClampToEdge;
		params.wrapT = RHI::SamplerWrapMode::ClampToEdge;
		return params;
	}
}


class Texture
{
public:
	virtual ~Texture() = default;

	bool IsValid() const { return m_HWTexture != nullptr; }

	void Init(const RHI::TextureDesc& desc, const void* data = nullptr);

	void SetSampler(Ref<HWSampler> sampler) { m_Sampler = std::move(sampler); }
	Ref<HWSampler> GetSampler() const { return m_Sampler; }

	void GenerateMipmap();

	const RHI::TextureDesc& GetDesc() const { return m_Desc; }

	void Bind(uint32_t slot = 0);
	void Unbind();

	uint32_t GetSizeX() const { return m_Desc.Width; }
	uint32_t GetSizeY() const { return m_Desc.Height; }
	uint32_t GetSizeZ() const { return m_Desc.DepthOrLayers; }

	uint32_t GetRendererID();

protected:
	Ref<HWTexture> m_HWTexture;
	Ref<HWSampler> m_Sampler;
	RHI::TextureDesc m_Desc;
};


class Texture2D : public Texture
{
public:
	Texture2D(const std::string& path, bool bSRGB = false);

	Texture2D(const RHI::TextureDesc& desc, const void* data = nullptr);

public:
	static Ref<Texture2D> Create(const std::string& path, bool bSRGB = false);
	static Ref<Texture2D> Create(const uint32_t& rgba);
};


class Texture2DArray : public Texture
{
public:
	Texture2DArray(const RHI::TextureDesc& desc);
};


class Texture3D : public Texture
{
public:
	Texture3D();
	Texture3D(const RHI::TextureDesc& desc, const void* data);
};


class TextureCube : public Texture
{
public:
	enum CubeFace
	{
		Right,
		Left,
		Top,
		Bottom,
		Front,
		Back,
	};
	TextureCube(const RHI::TextureDesc& desc);

	void SetData(const void* data, uint32_t size);
	void SetFaceData(uint32_t srcName, uint32_t srcTarget, int32_t srcLevel, int32_t srcX, int32_t srcY, int32_t srcZ,
		CubeFace face);

	const RHI::TextureDesc& GetDesc() { return m_Desc; }
};


struct DefaultTextures
{
	Ref<Texture2D> White;
	Ref<Texture2D> Black;
	Ref<Texture2D> Normal;
	Ref<Texture2D> Gray;
};

template <typename T>
void CreateResource(Ref<T>& tex, const RHI::TextureDesc& desc)
{
	if (!tex || tex->GetDesc() != desc)
		tex = CreateRef<T>(desc);
}

extern DefaultTextures GDefaultTextures;
