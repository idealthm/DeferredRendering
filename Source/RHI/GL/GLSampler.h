#pragma once

#include "RHI/HWSampler.h"

class GLSampler : public HWSampler
{
public:
	GLSampler(const RHI::SamplerParams& params);
	~GLSampler() override;

	void Bind(uint32_t slot) override;
	void Unbind(uint32_t slot) override;

	const RHI::SamplerParams& GetParams() const override { return m_Params; }

private:
	RHI::SamplerParams m_Params;
	uint32_t m_RendererID = 0;
};
