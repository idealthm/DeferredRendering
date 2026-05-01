#include "Texture.h"

#include <iostream>
#include <string>

#include <glad/glad.h>

#include "RHI/SamplerPool.h"
#include "stb_images/stb_image.h"

namespace
{
	class FScopedFileLoader
	{
	public:
		FScopedFileLoader(const char* path, bool bSRGB);

		~FScopedFileLoader();

		operator bool() const { return !!data; }
		void* data;
		RHI::TextureDesc desc;
	};

	FScopedFileLoader::FScopedFileLoader(const char* path, bool bSRGB)
	{
		int32_t width, height, nrComponents;
		bool bfloat = stbi_is_hdr(path);
		if (bfloat)
		{
			data = stbi_loadf(path, &width, &height, &nrComponents, 0);
		}
		else
		{
			data = stbi_load(path, &width, &height, &nrComponents, 0);
		}

		desc.Width = width;
		desc.Height = height;
		desc.DepthOrLayers = 1;
		desc.MipLevels = 1;
		desc.Target = RHI::Sampler::Dim2D;

		switch (nrComponents) {
		case 1: desc.Format = RHI::Format::R8; break;
		case 2:
			desc.Format = bfloat ? RHI::Format::RG16F : RHI::Format::RG8;
			break;
		case 3:
			desc.Format = bSRGB ? RHI::Format::SRGB8 : (bfloat ? RHI::Format::RGB16F : RHI::Format::RGB8);
			break;
		case 4:
			desc.Format = bSRGB ? RHI::Format::SRGBA8 : (bfloat ? RHI::Format::RGBA16F : RHI::Format::RGBA8);
			break;
		}
	}

	FScopedFileLoader::~FScopedFileLoader()
	{
		stbi_image_free(data);
	}
}

DefaultTextures GDefaultTextures;

// --- Texture ---

void Texture::Init(const RHI::TextureDesc& desc, const void* data)
{
	m_Desc = desc;
	m_HWTexture = HWTexture::Create(desc, data);
}

void Texture::GenerateMipmap()
{
	if (m_HWTexture)
		m_HWTexture->GenerateMipmap();
}

void Texture::Bind(uint32_t slot)
{
	if (m_HWTexture)
		m_HWTexture->Bind(slot);
	if (m_Sampler)
		m_Sampler->Bind(slot);
}

void Texture::Unbind()
{
	if (m_HWTexture)
		m_HWTexture->Unbind();
}

uint32_t Texture::GetRendererID()
{
	if (m_HWTexture)
		return m_HWTexture->GetRendererID();
	return 0xFFFFFFFF;
}

// --- Texture2D ---

Texture2D::Texture2D(const std::string& path, bool bSRGB)
{
	FScopedFileLoader fileLoader(path.c_str(), bSRGB);
	if (fileLoader)
	{
		Init(fileLoader.desc, fileLoader.data);
		GenerateMipmap();
		SetSampler(SamplerPool::Get().GetOrCreate(DefaultRepeatMipmapSampler()));
	}
}

Texture2D::Texture2D(const RHI::TextureDesc& desc, const void* data)
{
	Init(desc, data);
	SetSampler(SamplerPool::Get().GetOrCreate(DefaultRepeatSampler()));
}

Ref<Texture2D> Texture2D::Create(const std::string& path, bool bSRGB)
{
	FScopedFileLoader fileLoader(path.c_str(), bSRGB);
	if (fileLoader)
	{
		auto tex = CreateRef<Texture2D>(fileLoader.desc, fileLoader.data);
		tex->GenerateMipmap();
		tex->SetSampler(SamplerPool::Get().GetOrCreate(DefaultRepeatMipmapSampler()));
		return tex;
	}

	std::cout << "Texture failed to load at path: " << path << std::endl;
	return {};
}

Ref<Texture2D> Texture2D::Create(const uint32_t& rgba)
{
	RHI::TextureDesc desc;
	desc.Width = 1;
	desc.Height = 1;
	desc.DepthOrLayers = 1;
	desc.MipLevels = 1;
	desc.Format = RHI::Format::RGBA8;
	desc.Target = RHI::Sampler::Dim2D;
	auto tex = CreateRef<Texture2D>(desc, &rgba);
	tex->SetSampler(SamplerPool::Get().GetOrCreate(DefaultRepeatSampler()));
	return tex;
}


// --- Texture2DArray ---

Texture2DArray::Texture2DArray(const RHI::TextureDesc& desc)
{
	Init(desc, nullptr);
}


// --- Texture3D ---

Texture3D::Texture3D()
{
}

Texture3D::Texture3D(const RHI::TextureDesc& desc, const void* data)
{
	Init(desc, data);
}


// --- TextureCube ---

TextureCube::TextureCube(const RHI::TextureDesc& desc)
{
	Init(desc, nullptr);
}

void TextureCube::SetData(const void* data, uint32_t size)
{
	if (m_HWTexture)
		m_HWTexture->SetData(data, size);
}

void TextureCube::SetFaceData(uint32_t srcName, uint32_t srcTarget, int32_t srcLevel,
	int32_t srcX, int32_t srcY, int32_t srcZ, CubeFace face)
{
	if (m_HWTexture)
		m_HWTexture->SetFaceData(srcName, srcTarget, srcLevel, srcX, srcY, srcZ, static_cast<uint32_t>(face));
}
