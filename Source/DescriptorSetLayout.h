#pragma once
#include "Common/Handle.h"
#include "Common/Utils/Bitset.h"
#include "RHI/DriverEnums.h"

namespace RHI
{
	class RHIDriver;
}

namespace RHI
{
	struct HwDescriptorSetLayout;
}

class DescriptorSetLayout
{
public:
	DescriptorSetLayout() noexcept = default;
	DescriptorSetLayout(
			RHI::RHIDriver& driver,
			RHI::DescriptorSetLayout&& descriptorSetLayout) noexcept;
	~DescriptorSetLayout() noexcept;

	DescriptorSetLayout(DescriptorSetLayout const&) = delete;
	DescriptorSetLayout(DescriptorSetLayout&& rhs) noexcept = default;
	DescriptorSetLayout& operator=(DescriptorSetLayout const&) = delete;
	DescriptorSetLayout& operator=(DescriptorSetLayout&& rhs) noexcept = default;

	void terminate(RHI::RHIDriver& driver) noexcept;

	Handle<RHI::HwDescriptorSetLayout> getHandle() const noexcept {
		return mDescriptorSetLayoutHandle;
	}

	size_t getMaxDescriptorBinding() const noexcept {
		return mMaxDescriptorBinding;
	}

	bool isSampler(descriptor_binding_t const binding) const noexcept {
		return mSamplers[binding];
	}

	Util::bitset64 getSamplerDescriptors() const noexcept {
		return mSamplers;
	}

	Util::bitset64 getUniformBufferDescriptors() const noexcept {
		return mUniformBuffers;
	}

private:
	Handle<RHI::HwDescriptorSetLayout> mDescriptorSetLayoutHandle;
	Util::bitset64 mSamplers;
	Util::bitset64 mUniformBuffers;
	uint8_t mMaxDescriptorBinding = 0;
	
};
