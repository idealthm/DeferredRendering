#include "DescriptorSet.h"

#include "DescriptorSetLayout.h"
#include "Engine.h"
#include "RHIDriver.h"
#include "EngineEnum.h"

DescriptorSet::~DescriptorSet()
{
	if (handle)
	{
		gEngine->GetDriver().DestroyDescriptorSet(handle);
	}
}

void DescriptorSet::commit(RHI::RHIDriver& driver, const DescriptorSetLayout& layout)
{
	if (dirty.any())
	{
		commitSlow(driver, layout);
	}
}

void DescriptorSet::commitSlow(RHI::RHIDriver& driver, const DescriptorSetLayout& layout)
{
	dirty.clear();

	if (handle)
	{
		driver.DestroyDescriptorSet(handle);
	}

	handle = driver.CreateDescriptorSet(layout.getHandle());
	valid.forEachSetBit([&layout, &driver, dsh = handle, descriptors = descriptors.data()]
		(descriptor_binding_t const binding) {
		if (layout.isSampler(binding)) {
			driver.updateDescriptorSetTexture(dsh, binding,
					descriptors[binding].texture.handle,
					descriptors[binding].texture.params);
		} else {
			driver.updateDescriptorSetBuffer(dsh, binding,
					descriptors[binding].buffer.handle,
					descriptors[binding].buffer.offset,
					descriptors[binding].buffer.size);
		}
	});
}

void DescriptorSet::SetBuffer(descriptor_binding_t binding, Handle<RHI::HwBufferObject> h, uint32_t offset, uint32_t size)
{
	if (descriptors[binding].buffer.handle != h || descriptors[binding].buffer.size != size || descriptors[binding].buffer.offset != offset) {
		dirty.set(binding);
	}

	descriptors[binding].buffer = {h, offset, size};
	valid.set(binding, bool(h));
}

void DescriptorSet::SetTexture(descriptor_binding_t binding, Handle<RHI::HwTexture> h, const RHI::SamplerParams& params)
{
	if (descriptors[binding].texture.handle != h || descriptors[binding].texture.params != params) {
		dirty.set(binding);
	}

	descriptors[binding].texture = {h, params, 0};
	valid.set(binding, bool(h));
}

Handle<RHI::HwTexture> DescriptorSet::GetTextureHandle(descriptor_binding_t binding) const
{
	return descriptors[binding].texture.handle;
}

const RHI::SamplerParams& DescriptorSet::GetSamplerParams(descriptor_binding_t binding) const
{
	return descriptors[binding].texture.params;
}

void DescriptorSet::bind(RHI::RHIDriver& driver, DescriptorSetBindingPoints set)
{
	driver.bindDescriptorSet(handle, +set);
}
