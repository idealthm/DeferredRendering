#include "Material.h"

#include "Engine.h"
#include "EngineEnum.h"
#include "MaterialParser.h"
#include "Shader/Program.h"

Material::Material(MaterialParser& parser)
{
	parser.Get<ChunkUib>(m_UniformBlock);
	parser.Get<ChunkSib>(m_SamplerBlock);
	parser.Get<ChunkSpirv>(m_ShaderData);
	parser.Get<ChunkDescriptorSetBindings>(m_DescriptorSetLayouts);
	parser.Get<ChunkRequiredAttrs>(m_RequiredAttributes);

	std::array<RHI::DescriptorSetLayout, MAX_DESCRIPTOR_SET_COUNT> descLayouts;
	if (parser.Get<ChunkDescriptorSetLayout>(descLayouts))
	{
		auto& driver = gEngine->GetDriver();
		m_DescriptorSetLayout = { driver, std::move(descLayouts[+DescriptorSetBindingPoints::PER_MATERIAL]) };
	}

	for (size_t i = 0; i < m_UniformBlock.getFieldInfoList().size(); i++)
		m_FieldIndex[m_UniformBlock.getFieldInfoList()[i].name] = i;
	for (size_t i = 0; i < m_SamplerBlock.getSamplerInfoList().size(); i++)
		m_SamplerIndex[m_SamplerBlock.getSamplerInfoList()[i].name] = i;
}

const SpirvEntry* Material::FindPass(MaterialPass pass) const
{
	for (auto& entry : m_ShaderData)
	{
		if (entry.pass == pass)
			return &entry;
	}
	return m_ShaderData.empty() ? nullptr : &m_ShaderData[0];
}

Handle<RHI::HwProgram> Material::GetProgram(MaterialPass pass) const
{
	auto it = m_CachedProgram.find(pass);
	if (it != m_CachedProgram.end())
		return it->second;

	const SpirvEntry* entry = FindPass(pass);
	if (!entry || entry->vertexSpirv.empty())
		return {};

	Program::ShaderSource source = { entry->vertexSpirv, entry->fragmentSpirv };
	Handle<RHI::HwProgram> program = gEngine->GetDriver().CreateProgram(
		Program{ source, m_DescriptorSetLayouts });

	m_CachedProgram[pass] = program;
	return program;
}


const Material::SamplerInfo* Material::FindSampler(const std::string& name) const
{
	const auto it = m_SamplerIndex.find(name);
	if (it == m_SamplerIndex.end())
		return nullptr;
	return &m_SamplerBlock.getSamplerInfoList()[it->second];
}

const Material::FieldInfo* Material::FindField(const std::string& name) const
{
	const auto it = m_FieldIndex.find(name);
	if (it == m_FieldIndex.end())
		return nullptr;
	return &m_UniformBlock.getFieldInfoList()[it->second];
}
