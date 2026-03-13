#pragma once
#include <map>
#include <string>
#include <vector>

#include "common/Core.h"
#include "Shader/Shader.h"


enum class RenderPassType;
enum class EShaderType;
class Texture2D;

template<typename T, typename enable = void> struct Length;

template<>
struct Length<float>
{
	static constexpr uint32 value = 1;
};

template<typename T>
struct Length<T, decltype(std::declval<T>().length())>
{
	static constexpr uint32 value = T().length();
};

enum class EMaterialDomain
{
	Surface,
	PostProcess,
	Compute,
	UI,
};

enum class EBlendMode
{
	Opaque,
	Mask,
	Transparency
};

class Material
{
public:
	Material(EMaterialDomain domain, EBlendMode blendMode)
		: m_Domain(domain), m_BlendMode(blendMode)
	{
	};

	struct floatInfo
	{
		uint32 offset;
		uint32 size;
	};

	void ApplyMaterial(Ref<Shader> shader);

	template<typename T>
	void SetFloat(const std::string& name, const T& value)
	{
		auto pos = this->m_FloatProperty.find(name);
		if (pos == this->m_FloatProperty.end())
		{
			constexpr uint32 length = Length<T>::value;
			m_FloatProperty.emplace(name, {m_Floats.size(), value.length()});
			if constexpr (!std::is_same_v<T, float>)
				for (int i = 0; i < length; i++) m_Floats.emplace_back(value[i]);
			else
				m_Floats.emplace_back(value);
		}
		else
		{
			if (pos->second.size != Length<T>::value) {ASSERT(false);}
			*(T *)&m_Floats[pos->second.offset] = value;
		}
	}

	template<typename T>
	T GetFloat(const std::string& name)
	{
		auto pos = this->m_FloatProperty.find(name);
		if (pos == this->m_FloatProperty.end())
		{
			return T();
		}
		return *(T *)&m_Floats[pos->second.offset];
	}

	template<typename T>
	T GetFloat(uint32 offset)
	{
		return *(T *)&m_Floats[offset];
	}

	Ref<Texture2D> GetTexture2D(const std::string& name) const;
	void SetTexture2D(const std::string& name, const Ref<Texture2D>& texture);

	EMaterialDomain	GetDomain() const { return m_Domain; }
	EBlendMode	GetBlendMode() const { return m_BlendMode; }
	Ref<Shader> GetShader(RenderPassType PassType) const;

	static Ref<Material> CreateDefault();

private:
	EMaterialDomain m_Domain = EMaterialDomain::Surface;
	EBlendMode m_BlendMode = EBlendMode::Opaque;

	std::vector<float> m_Floats;
	std::map<std::string, floatInfo> m_FloatProperty;
	std::map<std::string, Ref<Texture2D>> m_TextureProperty;
};
