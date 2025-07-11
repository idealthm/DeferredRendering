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
		FScopedFileLoader(const char* path);

		~FScopedFileLoader();
		uint8* data;
		int32 width, height, nrComponents;
	};

	FScopedFileLoader::FScopedFileLoader(const char* path)
	{
		data = stbi_load(path, &width, &height, &nrComponents, 0);
	}

	FScopedFileLoader::~FScopedFileLoader()
	{
		stbi_image_free(data);
	}
}

Texture2D::Texture2D(const TextureDescription& desc, const void* data)
	: m_Desc(desc)
{
	GLCall(glGenTextures(1, &m_RendererID));

	glBindTexture(GL_TEXTURE_2D, m_RendererID);
	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, m_Desc.format, m_Desc.width, m_Desc.height, 0, m_Desc.format, GL_UNSIGNED_BYTE, data));
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

Texture2D::~Texture2D()
{
	GLCall(glDeleteTextures(1, &m_RendererID));
}

void Texture2D::Bind(uint32 slot)
{
	GLCall(glBindTextureUnit(slot, m_RendererID));
}

void Texture2D::Unbind()
{
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));
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

uint32 Texture2D::GetRendererID()
{
	return m_RendererID;
}

void Texture2D::SetData(const void* data, uint32 size)
{
	Bind();
	// TODO: format && internal format.
	// TODO: SRGB.
	ASSERT(size == m_Desc.bpp * m_Desc.width * m_Desc.height)
	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, m_Desc.format, m_Desc.width, m_Desc.height, 0, m_Desc.format, GL_UNSIGNED_BYTE, data));
}

const TextureDescription& Texture2D::GetDesc()
{
	return m_Desc;
}

Ref<Texture2D> Texture2D::Create(const TextureDescription& desc, const void* data)
{
	return CreateRef<Texture2D>(desc, data);
}

Ref<Texture2D> Texture2D::Create(const std::string& path)
{
	FScopedFileLoader fileLoader(path.c_str());
	if (fileLoader.data)
	{
		TextureDescription desc;
		desc.importPath = path;
		desc.width = fileLoader.width;
		desc.height = fileLoader.height;
		desc.bpp = fileLoader.nrComponents;

		if (fileLoader.nrComponents == 1)
			desc.format = GL_RED;
		else if (fileLoader.nrComponents == 3)
			desc.format = GL_RGB;
		else if (fileLoader.nrComponents == 4)
			desc.format = GL_RGBA;

		return CreateRef<Texture2D>(desc, fileLoader.data);
	}

	std::cout << "Texture failed to load at path: " << path << std::endl;
	return {};
}
