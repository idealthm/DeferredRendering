#include "GLSampler.h"

#include <glad/glad.h>

#include "RHI/RHITypes.h"

GLSampler::GLSampler(const RHI::SamplerParams& params)
	: m_Params(params)
{
	glCreateSamplers(1, &m_RendererID);

	glSamplerParameteri(m_RendererID, GL_TEXTURE_WRAP_S, RHI_Internal::GetGLWrapMode(params.wrapS));
	glSamplerParameteri(m_RendererID, GL_TEXTURE_WRAP_T, RHI_Internal::GetGLWrapMode(params.wrapT));
	glSamplerParameteri(m_RendererID, GL_TEXTURE_WRAP_R, RHI_Internal::GetGLWrapMode(params.wrapR));

	glSamplerParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, RHI_Internal::GetGLFilter(params.filterMin));
	glSamplerParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, RHI_Internal::GetGLFilter(params.filterMag));

	if (params.anisotropyLog2 > 0)
	{
		float maxAniso = 1.0f;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAniso);
		float aniso = static_cast<float>(1 << params.anisotropyLog2);
		if (aniso > maxAniso) aniso = maxAniso;
		glSamplerParameterf(m_RendererID, GL_TEXTURE_MAX_ANISOTROPY, aniso);
	}

	if (params.compareMode != RHI::SamplerCompareMode::NONE)
	{
		glSamplerParameteri(m_RendererID, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glSamplerParameteri(m_RendererID, GL_TEXTURE_COMPARE_FUNC,
			RHI_Internal::GetGLCompareFunc(params.compareFunc));
	}
}

GLSampler::~GLSampler()
{
	glDeleteSamplers(1, &m_RendererID);
}

void GLSampler::Bind(uint32_t slot)
{
	glBindSampler(slot, m_RendererID);
}

void GLSampler::Unbind(uint32_t slot)
{
	glBindSampler(slot, 0);
}
