#include "Material.h"

#include "Engine.h"
#include "Shader/Program.h"

Material::Material(const MaterialInfo& info)
	: m_Info(info)
{
	BuildFromReflection(info);
}

Handle<RHI::HwProgram> Material::GetProgram() const
{
	if (m_CachedProgram)
	{
		return m_CachedProgram;
	}

	m_CachedProgram = gEngine->GetDriver().CreateProgram(Program{m_Info.shaderData, m_Info.descriptorSets});
	return m_CachedProgram;
}

void Material::BuildFromReflection(const MaterialInfo& info)
{
	m_SamplerBlock = info.sib;
	m_UniformBlock = info.uib;
	for (size_t i = 0; i < m_UniformBlock.fields.size(); i++)
		m_FieldIndex[m_UniformBlock.fields[i].name] = i;
	for (size_t i = 0; i < m_SamplerBlock.mSamplersInfoList.size(); i++)
		m_SamplerIndex[m_SamplerBlock.mSamplersInfoList[i].name] = i;
}

const SamplerInfo* Material::FindSampler(const std::string& name) const
{
	const auto it = m_SamplerIndex.find(name);
	if (it == m_SamplerIndex.end())
		return nullptr;
	return &m_SamplerBlock.mSamplersInfoList[it->second];
}

const FieldInfo* Material::FindField(const std::string& name) const
{
	const auto it = m_FieldIndex.find(name);
	if (it == m_FieldIndex.end())
		return nullptr;
	return &m_UniformBlock.fields[it->second];
}
