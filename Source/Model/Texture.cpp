#include "Texture.h"

#include <iostream>
#include <string>

#include <glad/glad.h>

#include "assimp/TinyFormatter.h"
#include "stb_images/stb_image.h"

namespace
{
	class FScopedFileLoader
	{
	public:
		FScopedFileLoader(const char* path, bool bSRGB);

		~FScopedFileLoader();

		operator bool() const{return !!data;}
		void* data;
		TextureDescription desc;
	};

	FScopedFileLoader::FScopedFileLoader(const char* path, bool bSRGB)
	{
		int32 width, height, nrComponents;
		if (stbi_is_hdr(path))
		{
			data = stbi_loadf(path, &width, &height, &nrComponents, 0);
		}
		else
		{
			data = stbi_load(path, &width, &height, &nrComponents, 0);
		}

		desc.width = width;
		desc.height = height;
		desc.bGenerateMipmap = true;
		desc.minFilter = ETextureFilter::LinearMipmapLinear;
		desc.magFilter = ETextureFilter::Linear;

		switch (nrComponents) {
		case 1: desc.format = ETextureFormat::R8; break;
		case 2: /* 可以映射为 RG8 */
			desc.format = ETextureFormat::RG8;
			break;
		case 3: 
			// 如果是颜色贴图（Albedo），推导为 sRGB 格式以获得正确的 Gamma 矫正
			desc.format = bSRGB ? ETextureFormat::SRGB8 : ETextureFormat::RGB8; 
			break;
		case 4: 
			desc.format = bSRGB ? ETextureFormat::SRGBA8 : ETextureFormat::RGBA8; 
			break;
		}
	}

	FScopedFileLoader::~FScopedFileLoader()
	{
		stbi_image_free(data);
	}
}

static GLint GetGLFilter(ETextureFilter filter)
{
	switch (filter) {
	case ETextureFilter::Nearest:            return GL_NEAREST;
	case ETextureFilter::Linear:             return GL_LINEAR;
	case ETextureFilter::LinearMipmapLinear: return GL_LINEAR_MIPMAP_LINEAR;
	}
	return GL_LINEAR;
}

uint32 GetGLWrapMode(ETextureWrapMode mode)
{
	switch (mode)
	{
		case ETextureWrapMode::ECLampToBorder:		return GL_CLAMP_TO_BORDER;
		case ETextureWrapMode::EClampToEdge:		return GL_CLAMP_TO_EDGE;
		case ETextureWrapMode::EMirrorClampToEdge:	return GL_MIRROR_CLAMP_TO_EDGE;
		case ETextureWrapMode::EMirroredRepeat:		return GL_MIRRORED_REPEAT;
		case ETextureWrapMode::ERepeat:				return GL_REPEAT;
	}
	return GL_REPEAT;
}

Texture::~Texture()
{
	GLCall(glDeleteTextures(1, &m_RendererID));
}

void Texture::Bind(uint32 slot)
{
	GLCall(glBindTextureUnit(slot, m_RendererID));
}

void Texture::Unbind()
{
}

DefaultTextures GDefaultTextures;

GLFormatInfo GetGLInfo(ETextureFormat format)
{
	switch (format) {
	case ETextureFormat::R8:        		return { GL_R8,					GL_RED,				GL_UNSIGNED_BYTE };
	case ETextureFormat::RG8:   			return { GL_RG8,				GL_RG,				GL_UNSIGNED_BYTE };
	case ETextureFormat::RG16F: 			return { GL_RG16F,				GL_RG,				GL_HALF_FLOAT };
	case ETextureFormat::RGB8:      		return { GL_RGB8,				GL_RGB,				GL_UNSIGNED_BYTE };
	case ETextureFormat::RGBA8:     		return { GL_RGBA8,				GL_RGBA,			GL_UNSIGNED_BYTE };
	case ETextureFormat::SRGB8:    			return { GL_SRGB8,				GL_RGB,				GL_UNSIGNED_BYTE };
	case ETextureFormat::SRGBA8:    		return { GL_SRGB8_ALPHA8,		GL_RGBA,			GL_UNSIGNED_BYTE };
	case ETextureFormat::RGBA16F:   		return { GL_RGBA16F,			GL_RGBA,			GL_HALF_FLOAT };
	case ETextureFormat::R11G11B10F:		return { GL_R11F_G11F_B10F,		GL_RGB,				GL_UNSIGNED_INT_10F_11F_11F_REV };
	case ETextureFormat::Depth24:   		return { GL_DEPTH_COMPONENT24,	GL_DEPTH_COMPONENT, GL_UNSIGNED_INT };
	case ETextureFormat::Depth24Stencil8:	return { GL_DEPTH24_STENCIL8,	GL_DEPTH_STENCIL,	GL_UNSIGNED_INT_24_8 };
	}
	return { 0, 0, 0 };
}

Texture2D::Texture2D(const std::string& path, bool bSRGB)
{
	FScopedFileLoader fileLoader(path.c_str(), bSRGB);
	if (fileLoader)
	{
		Init(fileLoader.desc, fileLoader.data);
	}
}

void Texture2D::Init(const TextureDescription& desc, const void* data)
{
	GLCall(glGenTextures(1, &m_RendererID));

	m_Desc = desc;

	GLFormatInfo info = GetGLInfo(desc.format);

	glBindTexture(GL_TEXTURE_2D, m_RendererID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, info.internalFormat, m_Desc.width, m_Desc.height, 0, info.format, info.type, data);

	if (desc.bGenerateMipmap) glGenerateMipmap(GL_TEXTURE_2D);

	SetTextureParameter(m_Desc);
}

void Texture2D::SetTextureParameter(const TextureDescription& desc) const
{
	glBindTexture(GL_TEXTURE_2D, m_RendererID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GetGLWrapMode(desc.FilterS));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GetGLWrapMode(desc.FilterR));

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GetGLFilter(desc.minFilter));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GetGLFilter(desc.magFilter));

	if (desc.FilterR == ETextureWrapMode::ECLampToBorder || desc.FilterS == ETextureWrapMode::ECLampToBorder)
	{
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &desc.BorderColor[0]);
	}
}

Texture2D::Texture2D(const TextureDescription& desc, const void* data)
{
	Init(desc, data);
}

uint32 Texture2D::GetSizeX()
{
	return m_Desc.width;
}

uint32 Texture2D::GetSizeY()
{
	return m_Desc.height;
}

uint32 Texture2D::GetSizeZ()
{
	return 0;
}

const TextureDescription& Texture2D::GetDesc()
{
	return m_Desc;
}

Ref<Texture2D> Texture2D::Create(const std::string& path, bool bSRGB)
{
	FScopedFileLoader fileLoader(path.c_str(), bSRGB);
	if (fileLoader)
	{
		return CreateRef<Texture2D>(fileLoader.desc, fileLoader.data);
	}

	std::cout << "Texture failed to load at path: " << path << std::endl;
	return {};
}

Ref<Texture2D> Texture2D::Create(const uint32& rgba)
{
	TextureDescription desc;
	desc.width = 1;
	desc.height = 1;
	desc.SRGB = true;
	desc.slice = 1;
	desc.format = ETextureFormat::RGBA8;
	return CreateRef<Texture2D>(desc, &rgba);
}

Texture3D::Texture3D(const TextureDescription& desc, const void* data)
	: m_Desc(desc)
{
	GLCall(glGenTextures(1, &m_RendererID));

	GLFormatInfo info = GetGLInfo(desc.format);

	glBindTexture(GL_TEXTURE_3D, m_RendererID);
	GLCall(glTexImage2D(GL_TEXTURE_3D, 0, info.internalFormat, m_Desc.width, m_Desc.height, 0, info.format, info.type, data));

	if (m_Desc.bGenerateMipmap)
		glGenerateMipmap(GL_TEXTURE_3D);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (desc.bGenerateMipmap) glGenerateMipmap(GL_TEXTURE_3D);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GetGLWrapMode(desc.FilterS));
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GetGLWrapMode(desc.FilterT));
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GetGLWrapMode(desc.FilterR));

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GetGLFilter(desc.minFilter));
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GetGLFilter(desc.magFilter));

	if (desc.FilterR == ETextureWrapMode::ECLampToBorder || desc.FilterR == ETextureWrapMode::ECLampToBorder)
	{
		glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, &desc.BorderColor[0]);
	}
}

void Texture3D::Init(const TextureDescription& desc, const void* data)
{
	
}

uint32 Texture3D::GetSizeX()
{
	return m_Desc.width;
}

uint32 Texture3D::GetSizeY()
{
	return m_Desc.height;
}

uint32 Texture3D::GetSizeZ()
{
	return m_Desc.slice;
}

const TextureDescription& Texture3D::GetDesc() const
{
	return m_Desc;
}

TextureCube::TextureCube(const TextureDescription& desc)
{
	GLCall(glGenTextures(1, &m_RendererID));

	glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
	glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGBA16F, desc.width, desc.height);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GetGLWrapMode(desc.FilterS));
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GetGLWrapMode(desc.FilterT));
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GetGLWrapMode(desc.FilterR));
}

uint32 TextureCube::GetSizeX()
{
	return m_Desc.width;
}

uint32 TextureCube::GetSizeY()
{
	return m_Desc.height;
}

uint32 TextureCube::GetSizeZ()
{
	return 0;
}

void TextureCube::SetData(const void* data, uint32 size)
{
	ASSERT(false);
}

void TextureCube::SetFaceData(uint32 srcName, uint32 srcTarget, int32 srcLevel, int32 srcX, int32 srcY, int32 srcZ,
	CubeFace face)
{
	Bind();
	glCopyImageSubData(srcName, srcTarget, srcLevel, srcX, srcY, srcZ, m_RendererID,
		GL_TEXTURE_CUBE_MAP_POSITIVE_X + static_cast<int32>(face), 0, 0, 0, 0, m_Desc.width, m_Desc.height, 1);
}

const TextureDescription& TextureCube::GetDesc()
{
	return m_Desc;
}

