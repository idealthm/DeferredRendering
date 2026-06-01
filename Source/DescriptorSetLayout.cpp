#include "DescriptorSetLayout.h"

#include "Engine.h"
#include "RHI/RHIDriver.h"

DescriptorSetLayout::DescriptorSetLayout(RHI::RHIDriver& driver, RHI::DescriptorSetLayout descriptorSetLayout) noexcept
{
	for (auto&& desc : descriptorSetLayout.bindings) {
		mMaxDescriptorBinding = std::max(mMaxDescriptorBinding, desc.binding);
		mSamplers.set(desc.binding,
				desc.type == RHI::DescriptorType::SAMPLER);
		mUniformBuffers.set(desc.binding,
				desc.type == RHI::DescriptorType::UNIFORM_BUFFER);
	}

	mDescriptorSetLayoutHandle = driver.CreateDescriptorSetLayout(std::move(descriptorSetLayout));
}

DescriptorSetLayout::~DescriptorSetLayout() noexcept
{
	terminate(gEngine->GetDriver());
}

void DescriptorSetLayout::terminate(RHI::RHIDriver& driver) noexcept
{
	driver.DestroyDescriptorSetLayout(mDescriptorSetLayoutHandle);
}
