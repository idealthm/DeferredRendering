#pragma once

namespace RHI
{
	struct DescriptorSetLayout;
}

namespace DescriptorSets
{
	RHI::DescriptorSetLayout const& GetPerRenderableSetLayout() noexcept;
	RHI::DescriptorSetLayout const& GetPerViewSetLayout() noexcept;
}


