#include "Material.h"

#include "Engine.h"
#include "MaterialParser.h"
#include "Shader/Program.h"

Material::Material(MaterialParser& parser)
{
	ChunkSpirv::Container spirv;
	parser.Get<ChunkUib>(m_UniformBlock);
	parser.Get<ChunkSib>(m_SamplerBlock);
	parser.Get<ChunkSpirv>(spirv);
	m_ShaderData[0] = std::move(spirv.vertexSpirv);
	m_ShaderData[1] = std::move(spirv.fragmentSpirv);
	parser.Get<ChunkDescriptorSetLayout>(m_DescriptorSets);

	for (size_t i = 0; i < m_UniformBlock.fields.size(); i++)
		m_FieldIndex[m_UniformBlock.fields[i].name] = i;
	for (size_t i = 0; i < m_SamplerBlock.mSamplersInfoList.size(); i++)
		m_SamplerIndex[m_SamplerBlock.mSamplersInfoList[i].name] = i;
}

Handle<RHI::HwProgram> Material::GetProgram() const
{
	if (m_CachedProgram)
	{
		return m_CachedProgram;
	}

	m_CachedProgram = gEngine->GetDriver().CreateProgram(Program{m_ShaderData, m_DescriptorSets});
	return m_CachedProgram;
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
