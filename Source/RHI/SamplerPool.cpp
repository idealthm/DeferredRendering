#include "SamplerPool.h"

SamplerPool& SamplerPool::Get()
{
	static SamplerPool instance;
	return instance;
}

Ref<HWSampler> SamplerPool::GetOrCreate(const RHI::SamplerParams& params)
{
	auto it = m_Cache.find(params);
	if (it != m_Cache.end())
		return it->second;

	auto sampler = HWSampler::Create(params);
	if (sampler)
		m_Cache.emplace(params, sampler);
	return sampler;
}

void SamplerPool::Clear()
{
	m_Cache.clear();
}
