#pragma once

class TextureSampler
{
public:
	using SamplerMinFilter = RHI::SamplerMinFilter;
	using SamplerMagFilter = RHI::SamplerMagFilter;
	using SamplerWrapMode = RHI::SamplerWrapMode;
	using SamplerCompareMode = RHI::SamplerCompareMode;
	using SamplerCompareFunc = RHI::SamplerCompareFunc;
	using SamplerParams = RHI::SamplerParams;

	TextureSampler() = default;

	TextureSampler(SamplerMagFilter magFilter, SamplerWrapMode  wrap)
	{
		m_Params.filterMag = magFilter;
		m_Params.filterMin = (SamplerMinFilter)magFilter;
		m_Params.wrapR = m_Params.wrapS = m_Params.wrapT = wrap;
	}

	TextureSampler(SamplerMagFilter magFilter, SamplerMinFilter minFilter, SamplerWrapMode  wrap)
	{
		m_Params.filterMag = magFilter;
		m_Params.filterMin = minFilter;
		m_Params.wrapR = m_Params.wrapS = m_Params.wrapT = wrap;
	}

	TextureSampler(SamplerMinFilter minFilter, SamplerMagFilter magFilter,
		SamplerWrapMode wrapS, SamplerWrapMode wrapT, SamplerWrapMode wrapR)
	{
		m_Params.filterMin = minFilter;
		m_Params.filterMag = magFilter;
		m_Params.wrapS = wrapS;
		m_Params.wrapT = wrapT;
		m_Params.wrapR = wrapR;
	}

	// --- named presets --------------------------------------------------------

	static TextureSampler LinearRepeat()
	{
		return {SamplerMagFilter::Linear, SamplerWrapMode::Repeat};
	}

	static TextureSampler LinearClamp()
	{
		return {SamplerMagFilter::Linear, SamplerWrapMode::ClampToEdge};
	}

	static TextureSampler LinearMipmapRepeat()
	{
		TextureSampler s = LinearRepeat();
		s.m_Params.filterMin = SamplerMinFilter::LinearMipmapLinear;
		return s;
	}

	static TextureSampler NearestClamp()
	{
		return {SamplerMagFilter::Nearest, SamplerWrapMode::ClampToEdge};
	}

	static TextureSampler Skybox()
	{
		TextureSampler s = LinearClamp();
		s.m_Params.wrapR = SamplerWrapMode::ClampToEdge;
		return s;
	}

	static TextureSampler Shadow()
	{
		TextureSampler s = LinearClamp();
		s.m_Params.compareMode = SamplerCompareMode::COMPARE_TO_TEXTURE;
		s.m_Params.compareFunc = SamplerCompareFunc::LessEqual;
		return s;
	}

	// --- accessors ------------------------------------------------------------

	const SamplerParams& GetParams() const { return m_Params; }

	// --- chainable setters ----------------------------------------------------

	TextureSampler& SetWrap(SamplerWrapMode s, SamplerWrapMode t)
	{
		m_Params.wrapS = s;
		m_Params.wrapT = t;
		return *this;
	}

	TextureSampler& SetWrapR(SamplerWrapMode r)
	{
		m_Params.wrapR = r;
		return *this;
	}

	TextureSampler& SetAnisotropy(uint8_t log2)
	{
		m_Params.anisotropyLog2 = log2;
		return *this;
	}

private:
	SamplerParams m_Params{};
};
