#include "HWTexture.h"

#include "GL/GLTexture.h"
#include "RendererAPI.h"

Ref<HWTexture> HWTexture::Create(const RHI::TextureDesc& desc, const void* data)
{
	switch (RendererAPI::Get())
	{
	case RendererAPI::Type::OpenGL:
		return CreateRef<GLTexture>(desc, data);
	case RendererAPI::Type::Vulkan:
		// TODO: VKTexture
		return nullptr;
	}
	return nullptr;
}
