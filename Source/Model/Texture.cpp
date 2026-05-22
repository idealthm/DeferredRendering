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
	m_HWTexture = gEngine->GetDriver().CreateTexture(desc.Target, desc.MipLevels, desc.Format, 1, desc.Width, desc.Height, desc.DepthOrLayers, desc.Usage);
}

Texture::~Texture()
{
	gEngine->GetDriver().DestroyTexture(m_HWTexture);
}

