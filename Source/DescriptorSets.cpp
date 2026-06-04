#include "DescriptorSets.h"

#include "EngineEnum.h"

using namespace RHI;
namespace DescriptorSets
{
	static RHI::DescriptorSetLayout perViewDescriptorSetLayout = {{
		{ DescriptorType::UNIFORM_BUFFER, ShaderStageFlags::VERTEX | ShaderStageFlags::FRAGMENT,  +PerViewBindingPoints::FRAME_UNIFORM, DescriptorFlags::DYNAMIC_OFFSET },
		{ DescriptorType::UNIFORM_BUFFER, ShaderStageFlags::FRAGMENT,                            +PerViewBindingPoints::LIGHT_DATA },
	}};

	static RHI::DescriptorSetLayout perRenderableDescriptorSetLayout = {{
		{ DescriptorType::UNIFORM_BUFFER, ShaderStageFlags::VERTEX | ShaderStageFlags::FRAGMENT,  +PerRenderableBindingPoints::OBJECT_UNIFORM},
	}};

	RHI::DescriptorSetLayout const& GetPerRenderableSetLayout() noexcept
	{
		return perRenderableDescriptorSetLayout;
	}

	RHI::DescriptorSetLayout const& GetPerViewSetLayout() noexcept
	{
		return perViewDescriptorSetLayout;
	}

	static RHI::DescriptorSetLayout gbufferSetLayout = {{
		{ DescriptorType::SAMPLER, ShaderStageFlags::FRAGMENT, +GBufferBindingPoint::G_BUFFER_DEPTH },
		{ DescriptorType::SAMPLER, ShaderStageFlags::FRAGMENT, +GBufferBindingPoint::G_BUFFER_NORMAL },
		{ DescriptorType::SAMPLER, ShaderStageFlags::FRAGMENT, +GBufferBindingPoint::G_BUFFER_ALBEDO },
		{ DescriptorType::SAMPLER, ShaderStageFlags::FRAGMENT, +GBufferBindingPoint::G_BUFFER_MATERIAL },
	}};

	RHI::DescriptorSetLayout const& GetGBufferSetLayout() noexcept
	{
		return gbufferSetLayout;
	}
}


