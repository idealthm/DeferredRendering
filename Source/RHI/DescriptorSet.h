#pragma once
#include <array>
#include <bitset>
#include <cstdint>

#include "DescriptorSetLayout.h"
#include "Common/Handle.h"
#include "Common/Material/MaterialTypes.h"
#include "Common/Utils/Bitset.h"
#include "RHI/DriverEnums.h"

class DescriptorSetLayout;
enum class DescriptorSetBindingPoints : uint8_t;

namespace RHI
{
	class RHIDriver;
}

namespace RHI
{
	struct HwTexture;
	struct HwBufferObject;
}

struct DescriptorDesc
{
	DescriptorDesc() noexcept {}

	union
	{
		struct
		{
			Handle<RHI::HwBufferObject> handle;
			uint32_t         offset;
			uint32_t         size;
		} buffer{};
		struct
		{
			Handle<RHI::HwTexture>   handle;
			RHI::SamplerParams  params;
			uint32_t            padding;
		} texture;
	};
};

struct DescriptorSet
{
	descriptor_set_t                set = 0;
	Util::bitset64					dirty;
	Util::bitset64                  valid;
	std::vector<DescriptorDesc>		descriptors;
	Handle<RHI::HwDescriptorSet>	handle;

	DescriptorSet() = default;

	DescriptorSet(const DescriptorSetLayout& layout)
		: dirty(std::numeric_limits<uint64_t>::max())
	{
		descriptors.resize(layout.getMaxDescriptorBinding() + 1);
	}

	void commit(RHI::RHIDriver& driver, const DescriptorSetLayout& layout);

	void SetBuffer(descriptor_binding_t binding, Handle<RHI::HwBufferObject> h, uint32_t offset = 0, uint32_t size = 0);

	void SetTexture(descriptor_binding_t binding, Handle<RHI::HwTexture> h, const RHI::SamplerParams& params);

	Handle<RHI::HwTexture> GetTextureHandle(descriptor_binding_t binding) const;

	const RHI::SamplerParams& GetSamplerParams(descriptor_binding_t binding) const;

	void bind(RHI::RHIDriver& driver, DescriptorSetBindingPoints set);

};
