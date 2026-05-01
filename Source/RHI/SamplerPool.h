#pragma once

#include <unordered_map>

#include "Common/Core.h"
#include "RHI/HWSampler.h"
#include "RHI/RHITypes.h"

class SamplerPool
{
public:
	static SamplerPool& Get();

	Ref<HWSampler> GetOrCreate(const RHI::SamplerParams& params);
	void Clear();

private:
	std::unordered_map<RHI::SamplerParams, Ref<HWSampler>, RHI::SamplerParams::Hasher> m_Cache;
};
