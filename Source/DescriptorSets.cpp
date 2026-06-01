#include "DescriptorSets.h"

#include "EngineEnum.h"

using namespace RHI;
namespace DescriptorSets
{
	static RHI::DescriptorSetLayout perViewDescriptorSetLayout = {{
		{ DescriptorType::UNIFORM_BUFFER, ShaderStageFlags::VERTEX | ShaderStageFlags::FRAGMENT,  +PerViewBindingPoints::FRAME_UNIFORM, DescriptorFlags::DYNAMIC_OFFSET },
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
}


