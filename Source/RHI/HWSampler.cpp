#include "HWSampler.h"

#include "GL/GLSampler.h"
#include "RendererAPI.h"

Ref<HWSampler> HWSampler::Create(const RHI::SamplerParams& params)
{
	switch (RendererAPI::Get())
	{
	case RendererAPI::Type::OpenGL:
		return CreateRef<GLSampler>(params);
	case RendererAPI::Type::Vulkan:
		return nullptr;
	}
	return nullptr;
}
