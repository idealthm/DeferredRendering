#pragma once

#include "Common/Core.h"
#include "RHI/RHITypes.h"

class HWSampler
{
public:
	static Ref<HWSampler> Create(const RHI::SamplerParams& params);

	virtual ~HWSampler() = default;

	virtual void Bind(uint32_t slot) = 0;
	virtual void Unbind(uint32_t slot) = 0;

	virtual const RHI::SamplerParams& GetParams() const = 0;
};
