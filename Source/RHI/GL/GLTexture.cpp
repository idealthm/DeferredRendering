#include "GLTexture.h"

#include <glad/glad.h>

GLTexture::GLTexture(const RHI::TextureDesc& desc, const void* data)
	: m_Desc(desc)
{
	glGenTextures(1, &m_RendererID);

	uint32_t target = GetGLTarget();
	uint32_t internalFmt = RHI_Internal::GetGLInternalFormat(desc.Format, false);
	uint32_t format = RHI_Internal::GetGLFormat(desc.Format);
	uint32_t type = RHI_Internal::GetGLType(desc.Format);

	glBindTexture(target, m_RendererID);

	// Mip level range is a texture property, not sampler state.
	// Without this, MAX_LEVEL defaults to 1000 making single-level textures
	// mipmap-incomplete when no sampler object overrides the min filter.
	glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, desc.MipLevels - 1);

	if (desc.Target == RHI::Sampler::Dim2D)
	{
		glTexImage2D(target, 0, internalFmt, desc.Width, desc.Height, 0, format, type, data);
	}
	else
	{
		glTexStorage2D(target, desc.MipLevels, internalFmt, desc.Width, desc.Height);
	}
}

GLTexture::~GLTexture()
{
	glDeleteTextures(1, &m_RendererID);
}

uint32_t GLTexture::GetGLTarget() const
{
	return RHI_Internal::GetGLTextureTarget(m_Desc.Target);
}

void GLTexture::Bind(uint32_t slot)
{
	glBindTextureUnit(slot, m_RendererID);
}

void GLTexture::Unbind()
{
}

void GLTexture::GenerateMipmap()
{
	Bind();
	glGenerateMipmap(GetGLTarget());
}

uint32_t GLTexture::GetRendererID() const
{
	return m_RendererID;
}

uint32_t GLTexture::GetSizeX() const
{
	return m_Desc.Width;
}

uint32_t GLTexture::GetSizeY() const
{
	return m_Desc.Height;
}

void GLTexture::SetData(const void* data, uint32_t size)
{
}

void GLTexture::SetFaceData(uint32_t srcName, uint32_t srcTarget, int32_t srcLevel,
	int32_t srcX, int32_t srcY, int32_t srcZ, uint32_t face)
{
	Bind();
	glCopyImageSubData(srcName, srcTarget, srcLevel, srcX, srcY, srcZ, m_RendererID,
		GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, 0, 0, 0, m_Desc.Width, m_Desc.Height, 1);
}
